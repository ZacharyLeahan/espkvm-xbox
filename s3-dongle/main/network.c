/* SPDX-License-Identifier: Apache-2.0 */
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "network.h"
#include "xbox_s3_protocol.h"
#include "usb_xbox.h"
#include "tusb.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "fcntl.h"

static esp_netif_t *s_station;
static psa_key_id_t s_key;
static uint8_t s_mac[6];
static uint32_t s_session;
static portMUX_TYPE s_lock=portMUX_INITIALIZER_UNLOCKED;
static int64_t s_last_input;
static uint32_t s_reports;
static void network_event(void *arg,esp_event_base_t base,int32_t id,void *data)
{
    (void)arg;(void)data;
    if(base==WIFI_EVENT && id==WIFI_EVENT_STA_START) esp_wifi_connect();
    if(base==WIFI_EVENT && id==WIFI_EVENT_STA_DISCONNECTED) {
        usb_xbox_bluetooth(NULL);
        esp_wifi_connect();
    }
}
bool s3_network_owns_input(void)
{
    portENTER_CRITICAL(&s_lock);
    int64_t last=s_last_input;
    portEXIT_CRITICAL(&s_lock);
    return last && esp_timer_get_time()-last<500000;
}
void s3_network_status(char *out,unsigned size)
{
    esp_netif_ip_info_t ip={0};esp_netif_get_ip_info(s_station,&ip);
    portENTER_CRITICAL(&s_lock);
    uint32_t reports=s_reports;
    portEXIT_CRITICAL(&s_lock);
    snprintf(out,size,"Home Wi-Fi: " IPSTR " | P4 reports: %lu | routing: %s",IP2STR(&ip.ip),(unsigned long)reports,s3_network_owns_input()?"active":"waiting");
}
static void receiver(void *unused)
{
    (void)unused;
    int fd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(fd<0) { ESP_LOGE("s3_net","socket failed"); vTaskDelete(NULL);return; }
    int enable=1;setsockopt(fd,SOL_SOCKET,SO_BROADCAST,&enable,sizeof(enable));
    struct sockaddr_in local={.sin_family=AF_INET,.sin_port=htons(XBOX_S3_PORT),.sin_addr.s_addr=htonl(INADDR_ANY)};
    if(bind(fd,(void *)&local,sizeof(local))<0 || fcntl(fd,F_SETFL,O_NONBLOCK)<0) {
        close(fd);ESP_LOGE("s3_net","bind failed");vTaskDelete(NULL);return;
    }
    uint32_t hello_seq=0,peer_session=0,last_seq=0;
    int64_t hello_at=0,last_at=0;
    bool have_peer=false;
    for(;;) {
        int64_t now=esp_timer_get_time();
        esp_netif_ip_info_t ip={0};esp_netif_get_ip_info(s_station,&ip);
        if(ip.ip.addr && now-hello_at>=1000000) {
            xbox_s3_packet_t hello={.kind=XBOX_S3_HELLO,.flags=tud_mounted()&&!tud_suspended(),.sequence=++hello_seq,.session=s_session};
            memcpy(hello.mac,s_mac,6);
            struct sockaddr_in dest={.sin_family=AF_INET,.sin_port=htons(XBOX_P4_PORT),.sin_addr.s_addr=ip.ip.addr|~ip.netmask.addr};
            if(xbox_s3_sign(s_key,&hello))sendto(fd,&hello,sizeof(hello),MSG_DONTWAIT,(void *)&dest,sizeof(dest));
            hello_at=now;
        }
        for(unsigned i=0;i<8;i++) {
            uint8_t buffer[sizeof(xbox_s3_packet_t)+1];
            int n=recv(fd,buffer,sizeof(buffer),0);
            if(n<0)break;
            if(n!=sizeof(xbox_s3_packet_t))continue;
            xbox_s3_packet_t p;memcpy(&p,buffer,sizeof(p));
            if(p.kind!=XBOX_S3_STATE || p.receiver!=s_session || memcmp(p.mac,s_mac,6) ||
               p.report[0]!=0 || p.report[1]!=20 || p.report[3]!=0 || !xbox_s3_verify(s_key,&p))continue;
            if(have_peer && p.session==peer_session && !xbox_s3_newer(p.sequence,last_seq))continue;
            /* A restarted sender can claim control only after the old sender's
             * lease expires. Within a session duplicates never extend a hold. */
            if(have_peer && p.session!=peer_session && now-last_at<500000)continue;
            have_peer=true;peer_session=p.session;last_seq=p.sequence;last_at=now;
            usb_xbox_bluetooth(p.report);
            portENTER_CRITICAL(&s_lock);
            s_last_input=now; s_reports++;
            portEXIT_CRITICAL(&s_lock);
        }
        /* A 5 ms delay rounds to zero at the default 100 Hz tick rate.
         * Always block at least one tick so the USB service task can run. */
        vTaskDelay(pdMS_TO_TICKS(5)+1);
    }
}
void s3_network_start(void)
{
    s_station=esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,ESP_EVENT_ANY_ID,network_event,NULL));
    ESP_ERROR_CHECK(esp_read_mac(s_mac,ESP_MAC_WIFI_STA));
    s_session=esp_random();
    ESP_ERROR_CHECK(psa_crypto_init()==PSA_SUCCESS?ESP_OK:ESP_FAIL);
    ESP_ERROR_CHECK(xbox_s3_key(CONFIG_KVM_XBOX_S3_KEY,&s_key)==PSA_SUCCESS?ESP_OK:ESP_ERR_INVALID_ARG);
    wifi_config_t station={0};
    strlcpy((char *)station.sta.ssid,CONFIG_KVM_WIFI_DEFAULT_SSID,sizeof(station.sta.ssid));
    strlcpy((char *)station.sta.password,CONFIG_KVM_WIFI_DEFAULT_PASSWORD,sizeof(station.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&station));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(xTaskCreate(receiver,"p4_receiver",6144,NULL,4,NULL)==pdPASS?ESP_OK:ESP_ERR_NO_MEM);
}
