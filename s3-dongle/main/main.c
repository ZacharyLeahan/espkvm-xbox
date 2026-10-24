/* SPDX-License-Identifier: Apache-2.0 */
#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "usb_xbox.h"
#include "tusb.h"
#include "network.h"

/* Keep the tested access point as a recovery and network-status page. */
static const char page[] =
"<!doctype html><meta name='viewport' content='width=device-width,initial-scale=1'><title>Xbox S3 test</title>"
"<style>body{font:18px system-ui;max-width:650px;margin:24px auto;padding:12px;background:#152019;color:white}button{font:inherit;padding:18px;margin:5px;touch-action:none}pre{white-space:pre-wrap}</style>"
"<h1>Xbox S3 controller test</h1><p>Hold a control to press it. Releasing or losing this page releases inputs within 500 ms.</p><pre id='status'></pre><div id='buttons'></div><p>No rumble. This board controls the Xbox port it is plugged into.</p>"
"<script>const r=new Uint8Array(20);r[1]=20;let busy=false;async function send(){if(busy)return;busy=true;try{await fetch('/report',{method:'POST',headers:{'Content-Type':'application/octet-stream'},body:r.slice()})}finally{busy=false}}"
"const controls=[['Up',2,1],['Down',2,2],['Left',2,4],['Right',2,8],['Start',2,16],['Back',2,32],['L click',2,64],['R click',2,128],['A',4,255],['B',5,255],['X',6,255],['Y',7,255],['Black',8,255],['White',9,255],['LT',10,255],['RT',11,255]];"
"for(const [name,offset,value]of controls){const b=document.createElement('button');b.textContent=name;buttons.append(b);b.onpointerdown=e=>{e.preventDefault();b.setPointerCapture(e.pointerId);r[offset]|=value;send()};b.onpointerup=b.onpointercancel=b.onlostpointercapture=()=>{r[offset]&=~value;send()}}"
"for(let axis=0;axis<4;axis++)for(const direction of [-1,1]){const b=document.createElement('button');b.textContent=['LX','LY','RX','RY'][axis]+(direction<0?' -':' +');buttons.append(b);b.onpointerdown=e=>{e.preventDefault();b.setPointerCapture(e.pointerId);new DataView(r.buffer).setInt16(12+axis*2,direction*32767,true);send()};b.onpointerup=b.onpointercancel=b.onlostpointercapture=()=>{r[12+axis*2]=r[13+axis*2]=0;send()}}"
"function release(){r.fill(0);r[1]=20;send()}window.onblur=release;document.onvisibilitychange=()=>{if(document.hidden)release()};setInterval(()=>{if(!document.hidden)send()},100);"
"setInterval(async()=>{try{document.getElementById('status').textContent=await(await fetch('/status')).text()}catch(e){document.getElementById('status').textContent='Disconnected'}},1000);</script>";

static esp_err_t home(httpd_req_t *r)
{ httpd_resp_set_type(r,"text/html"); return httpd_resp_send(r,page,HTTPD_RESP_USE_STRLEN); }
static esp_err_t status_get(httpd_req_t *r)
{
    char s[512], network[160], held[48], lines[3][48], timing[48];
    uint32_t submitted,consumed; bool opened;
    usb_xbox_diagnostics(&submitted,&consumed,&opened);
    usb_xbox_history(held,lines,timing);
    s3_network_status(network,sizeof(network));
    snprintf(s,sizeof(s),"Firmware 0.2.1 | USB host: %s | suspended: %s\n%s\nXID open: %s | USB submitted: %lu | host consumed: %lu | held: %s",tud_mounted()?"configured":"not configured",tud_suspended()?"yes":"no",network,opened?"yes":"no",(unsigned long)submitted,(unsigned long)consumed,held);
    return httpd_resp_send(r,s,HTTPD_RESP_USE_STRLEN);
}
static esp_err_t report_post(httpd_req_t *r)
{
    char type[64]; uint8_t data[20];
    if (httpd_req_get_hdr_value_str(r,"Content-Type",type,sizeof(type)) != ESP_OK || strcmp(type,"application/octet-stream") || r->content_len != 20)
        return httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Expected 20-byte XID report");
    int received=0;
    while (received<20) {
        int n=httpd_req_recv(r,(char *)data+received,20-received);
        if(n<=0) return ESP_FAIL;
        received+=n;
    }
    if(data[0]!=0 || data[1]!=20 || data[3]!=0)
        return httpd_resp_send_err(r,HTTPD_400_BAD_REQUEST,"Invalid XID header");
    if(s3_network_owns_input()) {
        httpd_resp_set_status(r,"409 Conflict");
        return httpd_resp_send(r,"Player 1 is controlled by the main ESP",HTTPD_RESP_USE_STRLEN);
    }
    usb_xbox_bluetooth(data);
    return httpd_resp_send(r,"ok",2);
}
void app_main(void)
{
    /* Never erase provisioning or factory data as an automatic recovery. */
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t init=WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    uint8_t mac[6]; ESP_ERROR_CHECK(esp_read_mac(mac,ESP_MAC_WIFI_SOFTAP));
    wifi_config_t wifi={0};
    snprintf((char *)wifi.ap.ssid,sizeof(wifi.ap.ssid),"Xbox-S3-%02X%02X%02X",mac[3],mac[4],mac[5]);
    strcpy((char *)wifi.ap.password,"XboxTest2026");
    wifi.ap.ssid_len=strlen((char *)wifi.ap.ssid);
    wifi.ap.channel=6; wifi.ap.authmode=WIFI_AUTH_WPA2_PSK; wifi.ap.max_connection=2;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP,&wifi));
    ESP_ERROR_CHECK(usb_xbox_init());
    s3_network_start();
    httpd_config_t cfg=HTTPD_DEFAULT_CONFIG(); cfg.stack_size=6144; cfg.recv_wait_timeout=2;
    httpd_handle_t server; ESP_ERROR_CHECK(httpd_start(&server,&cfg));
    httpd_uri_t routes[]={
        {.uri="/",.method=HTTP_GET,.handler=home},
        {.uri="/status",.method=HTTP_GET,.handler=status_get},
        {.uri="/report",.method=HTTP_POST,.handler=report_post},
    };
    for(unsigned i=0;i<sizeof(routes)/sizeof(routes[0]);i++) ESP_ERROR_CHECK(httpd_register_uri_handler(server,&routes[i]));
    printf("READY: %s http://192.168.4.1 firmware 0.2.1\n",wifi.ap.ssid);
}
