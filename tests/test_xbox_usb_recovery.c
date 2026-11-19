/* SPDX-License-Identifier: Apache-2.0 */
#include <assert.h>
#include <stdio.h>
#include "xbox_usb_recovery.h"
int main(void)
{
    xbox_usb_recovery_t s={0};
    assert(xbox_usb_recovery_tick(&s,1,false,false)==XBOX_USB_KEEP);
    assert(xbox_usb_recovery_tick(&s,10000000,false,false)==XBOX_USB_KEEP);
    assert(xbox_usb_recovery_tick(&s,10000001,false,false)==XBOX_USB_DETACH);
    assert(xbox_usb_recovery_tick(&s,10999999,false,false)==XBOX_USB_KEEP);
    assert(xbox_usb_recovery_tick(&s,11000001,false,false)==XBOX_USB_ATTACH);
    assert(xbox_usb_recovery_tick(&s,31000000,false,false)==XBOX_USB_KEEP);
    assert(xbox_usb_recovery_tick(&s,31000001,false,false)==XBOX_USB_DETACH);
    assert(xbox_usb_recovery_tick(&s,32000001,false,false)==XBOX_USB_ATTACH);
    /* Enumeration cancels the pending retry, including a long-lived idle host. */
    for(int64_t now=33000001;now<100000000;now+=1000000)
        assert(xbox_usb_recovery_tick(&s,now,true,false)==XBOX_USB_KEEP);
    assert(s.attempts==0);
    /* Suspend is not an enumeration failure and must never force a detach. */
    for(int64_t now=100000001;now<200000000;now+=1000000)
        assert(xbox_usb_recovery_tick(&s,now,false,true)==XBOX_USB_KEEP);
    /* A later unplug starts recovery again without a firmware restart. */
    assert(xbox_usb_recovery_tick(&s,210000001,false,false)==XBOX_USB_DETACH);
    assert(xbox_usb_recovery_tick(&s,211000001,false,false)==XBOX_USB_ATTACH);
    puts("Xbox USB enumeration recovery timing and suspend protection: PASS");
}
