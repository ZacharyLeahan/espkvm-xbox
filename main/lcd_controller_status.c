/* SPDX-License-Identifier: Apache-2.0 */
#include "sdkconfig.h"
#include "capture_lcd_control.h"
#if CONFIG_KVM_XBOX_S3_PLAYER1
#include "xbox_controller_status.h"
#include "esp_timer.h"
#include <string.h>
static void get_players(capture_lcd_players_t *out)
{
    xbox_controller_status_t states[4];xbox_s3_route_status(states);
    int64_t now=esp_timer_get_time();out->enabled=true;
    for(unsigned p=0;p<4;p++) {
        xbox_controller_status_t *s=&states[p];
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
}
#endif
void lcd_controller_status_init(void)
{
#if CONFIG_KVM_XBOX_S3_PLAYER1
    capture_lcd_players_register(get_players);
#endif
}
