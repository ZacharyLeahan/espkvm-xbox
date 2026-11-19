/* SPDX-License-Identifier: Apache-2.0 */
#include "sdkconfig.h"
#if CONFIG_KVM_XBOX_S3_PLAYER1
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "xbox_s3_route.h"
#include "xbox_controller_status.h"
#if CONFIG_KVM_XBOX_DIRECT_PLAYER1
#include "usb_xbox.h"
#endif
#include "xbox_input_history.h"
#include "esp_http_client.h"
#include "freertos/task.h"
#include "xbox_s3_protocol.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_netif.h"
#include "lwip/sockets.h"
#include "fcntl.h"

static int s_socket=-1;
static int s_tx_socket=-1;
static char s_tx_interface[IFNAMSIZ];
static bool route_output_interface(void)
{
    esp_netif_t *netif=esp_netif_get_default_netif();
    struct ifreq interface={0};
    if(!netif || esp_netif_get_netif_impl_name(netif,interface.ifr_name)!=ESP_OK)return false;
    if(!strcmp(s_tx_interface,interface.ifr_name))return true;
    /* lwIP's first same-subnet match can select Wi-Fi even when Ethernet has
     * higher route priority. Keep discovery unbound, but send on the preferred
     * live interface, following failover changes without restarting the rig. */
    if(setsockopt(s_tx_socket,SOL_SOCKET,SO_BINDTODEVICE,&interface,sizeof(interface))<0)return false;
    memcpy(s_tx_interface,interface.ifr_name,sizeof(s_tx_interface));
    ESP_LOGI("xbox_s3","Controller output interface: %s",s_tx_interface);
    return true;
}
static psa_key_id_t s_key;
static uint32_t s_session;
static struct {
    uint8_t mac[6],last_report[20];
    uint32_t session,hello_seq,sequence;
    int64_t at,sent_at;
    int64_t error_logged_at;
    bool configured,usb;
    unsigned logged_connection;
    struct sockaddr_in address;
} s_peers[4];
static portMUX_TYPE s_lock=portMUX_INITIALIZER_UNLOCKED;


static xbox_controller_status_t s_status[4];
static void wifi_status(xbox_controller_status_t out[4])
{
    portENTER_CRITICAL(&s_lock);
    memcpy(out,s_status,sizeof(s_status));
    portEXIT_CRITICAL(&s_lock);
}
void xbox_s3_route_status(xbox_controller_status_t out[4])
{
    wifi_status(out);
#if CONFIG_KVM_XBOX_DIRECT_PLAYER1
    usb_xbox_status_t usb;usb_xbox_status(&usb);
    if(!xbox_controller_use_direct(usb.connected,usb.bus_alive,
        xbox_controller_connection(&out[0],esp_timer_get_time())==2))return;
    xbox_controller_status_t *p1=&out[0];
    memset(p1,0,sizeof(*p1));p1->direct_usb=true;
    p1->configured=true;p1->seen=usb.bus_alive || usb.connected;
    p1->usb=usb.connected;p1->diagnostics=true;p1->diagnostic_usb=usb.connected;
    p1->diagnostic_at=esp_timer_get_time();p1->completion_at=usb.completion_at;
    p1->submitted=usb.submitted;p1->consumed=usb.consumed;p1->sent=usb.submitted;
    usb_xbox_history(p1->held,(char[3][48]){{0}},(char[48]){0});
#endif
}
typedef struct {
    esp_http_client_handle_t client;
    struct ifreq interface;
    char ip[16],body[512];
    size_t used;
    bool overflow;
    uint32_t generation;
    int64_t retry_at;
} diagnostic_peer_t;
static esp_err_t diagnostic_event(esp_http_client_event_t *event)
{
    diagnostic_peer_t *peer=event->user_data;
    if(event->event_id==HTTP_EVENT_ON_DATA) {
        if(event->data_len<0 || peer->used+(size_t)event->data_len>=sizeof(peer->body))peer->overflow=true;
        else { memcpy(peer->body+peer->used,event->data,event->data_len);peer->used+=event->data_len; }
    }
    return ESP_OK;
}
static void diagnostics_worker(void *unused)
{
    (void)unused;
    /* Persistent connections avoid four TCP handshakes and TIME_WAIT entries
     * per sample. All requests remain outside the controller input worker. */
    diagnostic_peer_t *cache=calloc(4,sizeof(*cache));
    if(!cache) { vTaskDelete(NULL);return; }
    for(;;) {
        xbox_controller_status_t peers[4];
        wifi_status(peers);
        for(unsigned p=0;p<4;p++) {
            if(!peers[p].seen)continue;
            diagnostic_peer_t *sample=&cache[p];
            esp_netif_t *netif=esp_netif_get_default_netif();
            struct ifreq interface={0};
            if(!netif || esp_netif_get_netif_impl_name(netif,interface.ifr_name)!=ESP_OK)continue;
            if(sample->client && (strcmp(sample->ip,peers[p].ip) || sample->generation!=peers[p].generation ||
                strcmp(sample->interface.ifr_name,interface.ifr_name))) {
                esp_http_client_cleanup(sample->client);sample->client=NULL;sample->retry_at=0;
            }
            if(esp_timer_get_time()<sample->retry_at)continue;
            if(!sample->client) {
                char url[64];snprintf(url,sizeof(url),"http://%s/status",peers[p].ip);
                sample->interface=interface;
                esp_http_client_config_t config={.url=url,.timeout_ms=500,.buffer_size=512,
                    .disable_auto_redirect=true,.keep_alive_enable=true,
                    .if_name=&sample->interface,
                    .event_handler=diagnostic_event,.user_data=sample};
                sample->client=esp_http_client_init(&config);
                if(!sample->client)continue;
                memcpy(sample->ip,peers[p].ip,16);sample->generation=peers[p].generation;
            }
            sample->used=0;sample->overflow=false;
            esp_err_t result=esp_http_client_perform(sample->client);
            if(result!=ESP_OK) {
                esp_http_client_cleanup(sample->client);sample->client=NULL;
                sample->retry_at=esp_timer_get_time()+3000000;continue;
            }
            if(esp_http_client_get_status_code(sample->client)!=200 || sample->overflow)continue;
            sample->body[sample->used]=0;
            uint32_t submitted,consumed;bool usb;
            if(!xbox_controller_sample(sample->body,&submitted,&consumed,&usb))continue;
            int64_t now=esp_timer_get_time();
            portENTER_CRITICAL(&s_lock);
            xbox_controller_status_t *state=&s_status[p];
            /* A slow response must not revive diagnostics from an older boot. */
            if(state->generation==peers[p].generation && !strcmp(state->ip,peers[p].ip)) {
                if(consumed!=state->consumed)state->completion_at=now;
                state->submitted=submitted;state->consumed=consumed;
                state->diagnostic_usb=usb;
                state->diagnostic_at=now;state->diagnostics=true;
            }
            portEXIT_CRITICAL(&s_lock);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static bool peer_alive(unsigned player)
{
    portENTER_CRITICAL(&s_lock);
    int64_t at=s_peers[player].at;
    portEXIT_CRITICAL(&s_lock);
    return at && esp_timer_get_time()-at<4000000;
}
bool xbox_s3_route_alive(void)
{
#if CONFIG_KVM_XBOX_DIRECT_PLAYER1
    usb_xbox_status_t usb;usb_xbox_status(&usb);
    if(usb.bus_alive || usb.connected)return true;
#endif
    return peer_alive(0);
}
bool xbox_s3_route_ready(void)
{
    xbox_controller_status_t states[4];xbox_s3_route_status(states);
    return xbox_controller_connection(&states[0],esp_timer_get_time())==2;
}
esp_err_t xbox_s3_route_init(void)
{
    const char *identities[4]={CONFIG_KVM_XBOX_S3_MAC,CONFIG_KVM_XBOX_S3_MAC2,
        CONFIG_KVM_XBOX_S3_MAC3,CONFIG_KVM_XBOX_S3_MAC4};
    for(unsigned p=0;p<4;p++) {
        if(!identities[p][0])continue;
        unsigned mac[6]; char tail;
        if(sscanf(identities[p],"%2x:%2x:%2x:%2x:%2x:%2x%c",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5],&tail)!=6)
            return ESP_ERR_INVALID_ARG;
        for(unsigned i=0;i<6;i++)s_peers[p].mac[i]=mac[i];
        for(unsigned other=0;other<p;other++)
            if(s_peers[other].configured && !memcmp(s_peers[other].mac,s_peers[p].mac,6))return ESP_ERR_INVALID_ARG;
        s_peers[p].configured=true;
        s_status[p].configured=true;
        strcpy(s_status[p].held,"-");
        s_peers[p].last_report[1]=20;
    }
    if(psa_crypto_init()!=PSA_SUCCESS || xbox_s3_key(CONFIG_KVM_XBOX_S3_KEY,&s_key)!=PSA_SUCCESS)
        return ESP_ERR_INVALID_ARG;
    s_session=esp_random();
    s_socket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(s_socket<0)return ESP_FAIL;
    struct sockaddr_in local={.sin_family=AF_INET,.sin_port=htons(XBOX_P4_PORT),.sin_addr.s_addr=htonl(INADDR_ANY)};
    if(bind(s_socket,(void *)&local,sizeof(local))<0 || fcntl(s_socket,F_SETFL,O_NONBLOCK)<0) {
        close(s_socket); s_socket=-1; return ESP_FAIL;
    }
    s_tx_socket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(s_tx_socket<0 || fcntl(s_tx_socket,F_SETFL,O_NONBLOCK)<0) {
        if(s_tx_socket>=0)close(s_tx_socket);
        close(s_socket);s_socket=-1;s_tx_socket=-1;return ESP_FAIL;
    }
    if(xTaskCreate(diagnostics_worker,"s3_status",6144,NULL,2,NULL)!=pdPASS)return ESP_ERR_NO_MEM;
    ESP_LOGI("xbox_s3","Xbox players route to paired S3s; waiting for home-network discovery");
    return ESP_OK;
}
void xbox_s3_route_tick(const uint8_t reports[4][20])
{
    /* Never block the input worker on a missing dongle or slow network. */
    for(unsigned i=0;i<16;i++) {
        uint8_t buffer[sizeof(xbox_s3_packet_t)+1];
        xbox_s3_packet_t packet; struct sockaddr_in from; socklen_t size=sizeof(from);
        int n=recvfrom(s_socket,buffer,sizeof(buffer),0,(void *)&from,&size);
        if(n<0)break;
        if(n!=sizeof(packet))continue;
        memcpy(&packet,buffer,sizeof(packet));
        if(packet.kind!=XBOX_S3_HELLO || !xbox_s3_verify(s_key,&packet))continue;
        unsigned p;
        for(p=0;p<4;p++)if(s_peers[p].configured && !memcmp(packet.mac,s_peers[p].mac,6))break;
        if(p==4)continue;
        if(packet.session==s_peers[p].session && !xbox_s3_newer(packet.sequence,s_peers[p].hello_seq))continue;
        bool changed=!peer_alive(p) || packet.session!=s_peers[p].session || from.sin_addr.s_addr!=s_peers[p].address.sin_addr.s_addr;
        bool reboot=packet.session!=s_peers[p].session;
        bool usb_changed=s_peers[p].usb!=((packet.flags&1)!=0);
        s_peers[p].address=from; s_peers[p].address.sin_port=htons(XBOX_S3_PORT);
        s_peers[p].session=packet.session; s_peers[p].hello_seq=packet.sequence;
        portENTER_CRITICAL(&s_lock);
        s_peers[p].at=esp_timer_get_time(); s_peers[p].usb=(packet.flags&1)!=0;
        xbox_controller_status_t *state=&s_status[p];
        if(reboot) { state->generation++;state->diagnostics=false;state->diagnostic_at=0;
            state->completion_at=0;state->consumed=0;state->submitted=0; }
        state->seen=true;state->usb=s_peers[p].usb;state->beacon_at=s_peers[p].at;
        inet_ntoa_r(from.sin_addr,state->ip,sizeof(state->ip));
        portEXIT_CRITICAL(&s_lock);
        if(changed || usb_changed)ESP_LOGI("xbox_s3",
            "Route %u S3 %02x:%02x:%02x:%02x:%02x:%02x at %s; USB %s; physical Xbox port unknown",
            p+1,packet.mac[0],packet.mac[1],packet.mac[2],packet.mac[3],packet.mac[4],packet.mac[5],
            inet_ntoa(from.sin_addr),s_peers[p].usb?"configured":"not configured");
    }
    int64_t now=esp_timer_get_time();
    bool output_ready=route_output_interface();
    xbox_controller_status_t snapshot[4];
    wifi_status(snapshot);
#if CONFIG_KVM_XBOX_DIRECT_PLAYER1
    usb_xbox_status_t direct;usb_xbox_status(&direct);
    const uint8_t neutral[20]={0,20};
#endif
    for(unsigned p=0;p<4;p++) {
        const uint8_t *report=reports[p];
#if CONFIG_KVM_XBOX_DIRECT_PLAYER1
        /* Keep the standby receiver discovered and neutral, never duplicate P1. */
        if(p==0 && direct.connected) {
            report=neutral;
            /* A removed standby is expected during a cable swap. Its next
             * signed announcement resumes neutral refresh without error spam. */
            if(!peer_alive(0))continue;
        }
#endif
        unsigned connection=xbox_controller_connection(&snapshot[p],now);
        if(connection!=s_peers[p].logged_connection) {
            s_peers[p].logged_connection=connection;
            ESP_LOGI("xbox_s3","Route %u: %s; physical Xbox port unknown",p+1,
                connection==2?"USB configured":connection==1?"USB waiting":connection==3?"telemetry stale":"not seen");
        }
        /* Broadcast discovery can be lost while unicast control still works.
         * Keep the authenticated learned route until a new announcement updates
         * it. Input expiry is independent; an offline/rebooted receiver cannot
         * replay these states because its boot nonce must match. */
        if(!s_peers[p].at)continue;
        /* Each peer owns its refresh clock and replay sequence. */
        if(now-s_peers[p].sent_at<20000 && !memcmp(s_peers[p].last_report,report,20))continue;
        xbox_s3_packet_t out={.kind=XBOX_S3_STATE,.sequence=++s_peers[p].sequence,
            .session=s_session,.receiver=s_peers[p].session};
        memcpy(out.mac,s_peers[p].mac,6); memcpy(out.report,report,20);
        bool signed_packet=xbox_s3_sign(s_key,&out);
        int written=signed_packet && output_ready?sendto(s_tx_socket,&out,sizeof(out),MSG_DONTWAIT,
            (void *)&s_peers[p].address,sizeof(s_peers[p].address)):-1;
        int send_errno=signed_packet && output_ready && written<0?errno:0;
        bool sent=written==sizeof(out);
        if(!sent && (!s_peers[p].error_logged_at || now-s_peers[p].error_logged_at>=5000000)) {
            ESP_LOGW("xbox_s3","Route %u failed: signed=%d bytes=%d errno=%d",p+1,signed_packet,written,send_errno);
            s_peers[p].error_logged_at=now;
        }
        if(sent)memcpy(s_peers[p].last_report,report,20);
        char held[48]={0};
        xbox_history_label(xbox_history_mask(report,0),held,sizeof(held));
        bool active=false;
        for(unsigned i=2;i<20;i++)if(report[i])active=true;
        if(active && !strcmp(held,"-"))strcpy(held,"ANALOG");
        portENTER_CRITICAL(&s_lock);
        if(sent) {
            s_status[p].sent++;
            memcpy(s_status[p].held,held,sizeof(held));
            if(active)s_status[p].activity_at=now;
        } else s_status[p].send_errors++;
        portEXIT_CRITICAL(&s_lock);
        s_peers[p].sent_at=now;
    }
}
#endif
