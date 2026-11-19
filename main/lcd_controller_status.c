/* SPDX-License-Identifier: Apache-2.0 */
#include "sdkconfig.h"
#include "capture_lcd_control.h"
#include "usb_xbox.h"
#if CONFIG_KVM_USB_XBOX
#include "xbox_controller_status.h"
#include "esp_timer.h"
#include <string.h>
static void get_players(capture_lcd_players_t *out)
{
#if CONFIG_KVM_XBOX_S3_PLAYER1
    xbox_controller_status_t states[4];xbox_s3_route_status(states);
    int64_t now=esp_timer_get_time();out->enabled=true;out->wifi_enabled=true;
    for(unsigned p=0;p<4;p++) {
        xbox_controller_status_t *s=&states[p];
        if(p==0)out->direct_usb=s->direct_usb;
        out->paired[p]=s->configured;
        out->connection[p]=xbox_controller_connection(s,now);
        out->active[p]=s->activity_at && now-s->activity_at<1000000;
        out->stale_beacon[p]=s->seen && now-s->beacon_at>=4000000;
        out->consuming[p]=s->completion_at && now-s->completion_at<4000000;
        out->beacon_age[p]=xbox_controller_age(s->beacon_at,now);
        out->diagnostic_age[p]=xbox_controller_age(s->diagnostic_at,now);
        out->diagnostics[p]=s->diagnostics;
        out->submitted[p]=s->submitted;out->consumed[p]=s->consumed;
        memcpy(out->ip[p],s->ip,16);memcpy(out->held[p],s->held,48);
    }
#endif
#if !CONFIG_KVM_XBOX_S3_PLAYER1
    usb_xbox_status_t status;usb_xbox_status(&status);
    int64_t sampled_at=esp_timer_get_time();
    out->enabled=true;out->direct_usb=true;out->paired[0]=true;
    out->connection[0]=status.connected?2:status.bus_alive?1:0;
    out->consuming[0]=status.connected && status.completion_at && sampled_at-status.completion_at<1000000;
    out->diagnostics[0]=true;
    out->submitted[0]=status.submitted;out->consumed[0]=status.consumed;
    usb_xbox_history(out->held[0],(char[3][48]){{0}},(char[48]){0});
#endif
}
#endif
void lcd_controller_status_init(void)
{
#if CONFIG_KVM_USB_XBOX
    capture_lcd_players_register(get_players);
#endif
}
