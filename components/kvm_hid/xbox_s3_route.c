/* SPDX-License-Identifier: Apache-2.0 */
#include "sdkconfig.h"
#if CONFIG_KVM_XBOX_S3_PLAYER1
#include <errno.h>
#include "freertos/FreeRTOS.h"
#include <stdbool.h>
#include <stdio.h>
#include "xbox_s3_route.h"
#include "xbox_s3_protocol.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "lwip/sockets.h"
#include "fcntl.h"

static int s_socket=-1;
static psa_key_id_t s_key;
static uint8_t s_mac[6];
static uint32_t s_session,s_peer_session,s_hello_seq,s_sequence;
static int64_t s_peer_at,s_sent_at;
static bool s_usb;
static uint8_t s_last_report[20]={0,20};
static portMUX_TYPE s_lock=portMUX_INITIALIZER_UNLOCKED;
static struct sockaddr_in s_peer;

bool xbox_s3_route_alive(void)
{
    portENTER_CRITICAL(&s_lock);
    int64_t at=s_peer_at;
    portEXIT_CRITICAL(&s_lock);
    return at && esp_timer_get_time()-at<4000000;
}
bool xbox_s3_route_ready(void)
{
    portENTER_CRITICAL(&s_lock);
    bool usb=s_usb;
    portEXIT_CRITICAL(&s_lock);
    return usb && xbox_s3_route_alive();
}
esp_err_t xbox_s3_route_init(void)
{
    unsigned mac[6];
    if(sscanf(CONFIG_KVM_XBOX_S3_MAC,"%x:%x:%x:%x:%x:%x",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5])!=6)
        return ESP_ERR_INVALID_ARG;
    for(unsigned i=0;i<6;i++) { if(mac[i]>255)return ESP_ERR_INVALID_ARG; s_mac[i]=mac[i]; }
    if(psa_crypto_init()!=PSA_SUCCESS || xbox_s3_key(CONFIG_KVM_XBOX_S3_KEY,&s_key)!=PSA_SUCCESS)
        return ESP_ERR_INVALID_ARG;
    s_session=esp_random();
    s_socket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(s_socket<0)return ESP_FAIL;
    struct sockaddr_in local={.sin_family=AF_INET,.sin_port=htons(XBOX_P4_PORT),.sin_addr.s_addr=htonl(INADDR_ANY)};
    if(bind(s_socket,(void *)&local,sizeof(local))<0 || fcntl(s_socket,F_SETFL,O_NONBLOCK)<0) {
        close(s_socket); s_socket=-1; return ESP_FAIL;
    }
    ESP_LOGI("xbox_s3","Player 1 routes to paired S3; waiting for home-network discovery");
    return ESP_OK;
}
void xbox_s3_route_tick(const uint8_t report[20])
{
    /* Never block the input worker on a missing dongle or slow network. */
    for(unsigned i=0;i<4;i++) {
        uint8_t buffer[sizeof(xbox_s3_packet_t)+1];
        xbox_s3_packet_t packet; struct sockaddr_in from; socklen_t size=sizeof(from);
        int n=recvfrom(s_socket,buffer,sizeof(buffer),0,(void *)&from,&size);
        if(n<0)break;
        if(n!=sizeof(packet))continue;
        memcpy(&packet,buffer,sizeof(packet));
        if(packet.kind!=XBOX_S3_HELLO ||
           memcmp(packet.mac,s_mac,6) || !xbox_s3_verify(s_key,&packet))continue;
        if(packet.session==s_peer_session && !xbox_s3_newer(packet.sequence,s_hello_seq))continue;
        bool changed=!xbox_s3_route_alive() || packet.session!=s_peer_session || from.sin_addr.s_addr!=s_peer.sin_addr.s_addr;
        s_peer=from; s_peer.sin_port=htons(XBOX_S3_PORT);
        s_peer_session=packet.session; s_hello_seq=packet.sequence;
        portENTER_CRITICAL(&s_lock);
        s_peer_at=esp_timer_get_time(); s_usb=(packet.flags&1)!=0;
        portEXIT_CRITICAL(&s_lock);
        if(changed)ESP_LOGI("xbox_s3","Player 1 S3 discovered at %s; USB %s",inet_ntoa(from.sin_addr),s_usb?"configured":"not configured");
    }
    int64_t now=esp_timer_get_time();
    if(!xbox_s3_route_alive())return;
    /* Send edges immediately: an adjacent press/release must not disappear
     * merely because a periodic refresh was sent less than 20 ms ago. */
    if(now-s_sent_at<20000 && !memcmp(s_last_report,report,20))return;
    xbox_s3_packet_t out={.kind=XBOX_S3_STATE,.sequence=++s_sequence,.session=s_session,.receiver=s_peer_session};
    memcpy(out.mac,s_mac,6); memcpy(out.report,report,20);
    if(xbox_s3_sign(s_key,&out) && sendto(s_socket,&out,sizeof(out),MSG_DONTWAIT,(void *)&s_peer,sizeof(s_peer))==sizeof(out))
        memcpy(s_last_report,report,20);
    s_sent_at=now;
}
#endif
