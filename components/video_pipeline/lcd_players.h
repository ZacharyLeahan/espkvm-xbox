/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "capture_lcd_control.h"
#include "lcd_controller.h"

static inline int lcd_players_hit(unsigned x,unsigned y)
{
    if(x<8 || y<24 || y>=64)return -1;
    unsigned p=(x-8)/116;
    return p<4 && (x-8)%116<110?(int)p:-1;
}
static inline void lcd_players_color(uint8_t *fb,unsigned x,unsigned y,unsigned w,unsigned h,
                                     uint8_t red,uint8_t green,uint8_t blue)
{
    for(unsigned yy=y;yy<y+h && yy<480;yy++)for(unsigned xx=x;xx<x+w && xx<640;xx++) {
        size_t at=((size_t)yy*640+xx)*3;fb[at]=blue;fb[at+1]=green;fb[at+2]=red;
    }
}
static inline void lcd_players_draw(uint8_t *fb,const capture_lcd_players_t *s,int selected)
{
    if(!s->enabled)return;
    for(unsigned p=0;p<4;p++) {
        unsigned x=8+116*p;
        lcd_pad_rect(fb,x,24,110,40,selected==(int)p?48:22);
        unsigned state=s->connection[p];
        lcd_players_color(fb,x,24,4,40,state==2?40:state?225:110,state==2?190:state?160:110,state==2?100:state?45:110);
        if(s->active[p])lcd_players_color(fb,x,62,110,2,70,170,255);
        char text[12];snprintf(text,sizeof(text),"P%u %s",p+1,
            (s->direct_usb && p==0)?"DIRECT":"WIFI");
        lcd_pad_text(fb,x+((s->direct_usb && p==0)?4:10),27,text);
        snprintf(text,sizeof(text),"USB %s",state==2?"ON":state==1?"WAIT":state==3?"OLD":(s->direct_usb && p==0)?"OFF":"?");
        lcd_pad_text(fb,x+10,46,text);
    }
    if(selected<0 || selected>3)return;
    unsigned p=(unsigned)selected;
    bool direct=s->direct_usb && p==0;
    char line[48];
    lcd_pad_rect(fb,32,94,576,146,18);
    snprintf(line,sizeof(line),direct?"P%u DIRECT USB - TAP TO CLOSE":"P%u WI-FI - TAP TO CLOSE",p+1);lcd_pad_text(fb,44,104,line);
    snprintf(line,sizeof(line),"%s  %s",direct?"DEVICE USB":s->ip[p][0]?s->ip[p]:"NOT DISCOVERED",
        s->connection[p]==2?"USB CONNECTED":s->connection[p]==1?"USB WAITING":s->connection[p]==3?"STATUS STALE":"NOT SEEN");
    lcd_pad_text(fb,44,124,line);
    char beacon[16],sample[16];
    if(s->beacon_age[p]<0)strcpy(beacon,"UNKNOWN");else if(s->beacon_age[p]>999999999)strcpy(beacon,"OLD");else snprintf(beacon,sizeof(beacon),"%ums",(unsigned)s->beacon_age[p]);
    if(s->diagnostic_age[p]<0)strcpy(sample,"UNKNOWN");else if(s->diagnostic_age[p]>999999999)strcpy(sample,"OLD");else snprintf(sample,sizeof(sample),"%ums",(unsigned)s->diagnostic_age[p]);
    if(direct)snprintf(line,sizeof(line),"XBOX > ADAPTER > USB-C > ESP");
    else snprintf(line,sizeof(line),"WIFI AGE %s USB AGE %s",beacon,sample);
    lcd_pad_text(fb,44,144,line);
    snprintf(line,sizeof(line),"HELD %.40s",s->held[p]);lcd_pad_text(fb,44,164,line);
    if(s->diagnostics[p])snprintf(line,sizeof(line),"SENT %lu  XBOX %lu",(unsigned long)s->submitted[p],(unsigned long)s->consumed[p]);
    else snprintf(line,sizeof(line),"USB COUNTERS NOT OBSERVED");
    lcd_pad_text(fb,44,184,line);
    lcd_pad_text(fb,44,204,s->consuming[p]?"XBOX REPORT COUNT ADVANCING":"NO RECENT USB COMPLETIONS");
    lcd_pad_text(fb,44,224,direct?"P1 FIXED DEVICE USB CONNECTION":"PAIRED WI-FI PLAYER ROUTE");
}
