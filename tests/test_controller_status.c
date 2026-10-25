/* SPDX-License-Identifier: Apache-2.0 */
#include <assert.h>
#include <stdio.h>
#include "xbox_controller_status.h"
int main(void)
{
    const int64_t now=10000000;
    xbox_controller_status_t s={0};
    assert(xbox_controller_connection(&s,now)==0);
    s.configured=true;assert(xbox_controller_connection(&s,now)==0);
    s.seen=true;s.beacon_at=now-100000;s.usb=false;
    assert(xbox_controller_connection(&s,now)==1);
    s.usb=true;assert(xbox_controller_connection(&s,now)==2);
    s.beacon_at=now-4000000;assert(xbox_controller_connection(&s,now)==3);
    s.diagnostic_at=now-500000;s.diagnostic_usb=true;
    assert(xbox_controller_connection(&s,now)==2); /* Lost beacon is not lost control. */
    s.diagnostic_usb=false;assert(xbox_controller_connection(&s,now)==1);
    s.diagnostic_at=now-4000000;assert(xbox_controller_connection(&s,now)==3);
    assert(xbox_controller_age(0,now)==-1);
    assert(xbox_controller_age(now-1234000,now)==1234);
    s.beacon_at=now;s.usb=false;s.diagnostic_at=now;s.diagnostic_usb=true;
    assert(xbox_controller_connection(&s,now)==1); /* Signed USB state wins while fresh. */
    uint32_t submitted,consumed;bool usb;
    const char *body="USB host: configured | suspended: no\nXID open: yes | USB submitted: 123 | host consumed: 122 | held: -";
    assert(xbox_controller_sample(body,&submitted,&consumed,&usb));
    assert(submitted==123 && consumed==122 && usb);
    assert(!xbox_controller_sample("USB submitted: 4294967296 | host consumed: 0 | held: -",&submitted,&consumed,&usb));
    assert(!xbox_controller_sample("USB submitted: -1 | host consumed: 0 | held: -",&submitted,&consumed,&usb));
    assert(!xbox_controller_sample("USB submitted: 1 | host consumed: 2x | held: -",&submitted,&consumed,&usb));
    assert(xbox_controller_sample("USB host: configured | suspended: yes\nXID open: yes | USB submitted: 1 | host consumed: 1 | held: -",&submitted,&consumed,&usb) && !usb);
    puts("Controller status freshness and receiver diagnostics: PASS");
}
