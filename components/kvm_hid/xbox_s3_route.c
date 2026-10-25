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
static uint32_t s_session;
static struct {
    uint8_t mac[6],last_report[20];
    uint32_t session,hello_seq,sequence;
    int64_t at,sent_at;
    bool configured,usb;
    struct sockaddr_in address;
} s_peers[4];
static portMUX_TYPE s_lock=portMUX_INITIALIZER_UNLOCKED;


static bool peer_alive(unsigned player)
{
    portENTER_CRITICAL(&s_lock);
    int64_t at=s_peers[player].at;
    portEXIT_CRITICAL(&s_lock);
    return at && esp_timer_get_time()-at<4000000;
}
bool xbox_s3_route_alive(void) { return peer_alive(0); }
bool xbox_s3_route_ready(void)
{
    portENTER_CRITICAL(&s_lock);
    bool usb=s_peers[0].usb;
    portEXIT_CRITICAL(&s_lock);
    return usb && peer_alive(0);
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
        s_peers[p].address=from; s_peers[p].address.sin_port=htons(XBOX_S3_PORT);
        s_peers[p].session=packet.session; s_peers[p].hello_seq=packet.sequence;
        portENTER_CRITICAL(&s_lock);
        s_peers[p].at=esp_timer_get_time(); s_peers[p].usb=(packet.flags&1)!=0;
        portEXIT_CRITICAL(&s_lock);
        if(changed)ESP_LOGI("xbox_s3","Player %u S3 discovered at %s; USB %s",p+1,inet_ntoa(from.sin_addr),s_peers[p].usb?"configured":"not configured");
    }
    int64_t now=esp_timer_get_time();
    for(unsigned p=0;p<4;p++) {
        /* Broadcast discovery can be lost while unicast control still works.
         * Keep the authenticated learned route until a new announcement updates
         * it. Input expiry is independent; an offline/rebooted receiver cannot
         * replay these states because its boot nonce must match. */
        if(!s_peers[p].at)continue;
        /* Each peer owns its refresh clock and replay sequence. */
        if(now-s_peers[p].sent_at<20000 && !memcmp(s_peers[p].last_report,reports[p],20))continue;
        xbox_s3_packet_t out={.kind=XBOX_S3_STATE,.sequence=++s_peers[p].sequence,
            .session=s_session,.receiver=s_peers[p].session};
        memcpy(out.mac,s_peers[p].mac,6); memcpy(out.report,reports[p],20);
        if(xbox_s3_sign(s_key,&out) && sendto(s_socket,&out,sizeof(out),MSG_DONTWAIT,
            (void *)&s_peers[p].address,sizeof(s_peers[p].address))==sizeof(out))
            memcpy(s_peers[p].last_report,reports[p],20);
        s_peers[p].sent_at=now;
    }
}
#endif
