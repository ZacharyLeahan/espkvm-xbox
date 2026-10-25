/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef struct {
    bool configured,seen,usb,diagnostics,diagnostic_usb;
    char ip[16],held[48];
    uint32_t sent,send_errors,submitted,consumed;
    int64_t beacon_at,diagnostic_at,completion_at,activity_at;
    uint32_t generation;
} xbox_controller_status_t;
/* A consistent cached snapshot; never performs network I/O. */
void xbox_s3_route_status(xbox_controller_status_t out[4]);
/* Zero timestamp means unknown, never a freshly connected controller. */
static inline int64_t xbox_controller_age(int64_t at,int64_t now)
{ return at ? (now-at)/1000 : -1; }
static inline unsigned xbox_controller_connection(const xbox_controller_status_t *s,int64_t now)
{
    if(!s->configured || !s->seen)return 0;
    bool beacon=s->beacon_at && now-s->beacon_at<4000000;
    bool diagnostics=s->diagnostic_at && now-s->diagnostic_at<4000000;
    if(!beacon && !diagnostics)return 3;
    return (beacon?s->usb:s->diagnostic_usb)?2:1;
}

/* These HTTP diagnostics supplement signed discovery; they never authorize input. */
static inline bool xbox_controller_sample(const char *body,uint32_t *submitted,uint32_t *consumed,bool *usb)
{
    const char *p=strstr(body,"USB submitted: ");
    if(!p)return false;
    p+=15;
    if(*p<'0' || *p>'9')return false;
    char *end;errno=0;unsigned long long first=strtoull(p,&end,10);
    if(errno || first>UINT32_MAX || strncmp(end," | host consumed: ",18))return false;
    p=end+18;if(*p<'0' || *p>'9')return false;
    errno=0;unsigned long long second=strtoull(p,&end,10);
    if(errno || second>UINT32_MAX || strncmp(end," | held: ",9))return false;
    *submitted=(uint32_t)first;*consumed=(uint32_t)second;
    *usb=strstr(body,"USB host: configured") && strstr(body,"suspended: no") && strstr(body,"XID open: yes");
    return true;
}
