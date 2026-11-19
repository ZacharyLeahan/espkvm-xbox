/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct { int64_t retry_at,attach_at;unsigned attempts; } xbox_usb_recovery_t;
enum { XBOX_USB_KEEP, XBOX_USB_DETACH, XBOX_USB_ATTACH };
/* Called only on the USB task. Never disturb configured or suspended hosts.
 * A full second off-bus exceeds hub debounce; retries back off to one minute. */
static inline unsigned xbox_usb_recovery_tick(xbox_usb_recovery_t *s,int64_t now,
                                               bool configured,bool suspended)
{
    if(s->attach_at) {
        if(now<s->attach_at)return XBOX_USB_KEEP;
        s->attach_at=0;
        unsigned seconds=s->attempts==1?20:s->attempts==2?40:60;
        s->retry_at=now+(int64_t)seconds*1000000;
        return XBOX_USB_ATTACH;
    }
    if(configured || suspended) {
        s->retry_at=now+10000000;s->attempts=0;return XBOX_USB_KEEP;
    }
    if(!s->retry_at) { s->retry_at=now+10000000;return XBOX_USB_KEEP; }
    if(now<s->retry_at)return XBOX_USB_KEEP;
    if(s->attempts<3)s->attempts++;
    s->attach_at=now+1000000;
    return XBOX_USB_DETACH;
}
