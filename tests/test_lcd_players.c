/* SPDX-License-Identifier: Apache-2.0 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcd_players.h"
int main(int argc,char **argv)
{
    assert(lcd_players_hit(8,24)==0);assert(lcd_players_hit(117,45)==0);
    assert(lcd_players_hit(118,24)==-1);assert(lcd_players_hit(124,24)==1);
    assert(lcd_players_hit(466,30)==-1);assert(lcd_players_hit(10,64)==-1);
    size_t bytes=640*480*3;
    uint8_t *memory=malloc(bytes+32);assert(memory);memset(memory,0xa5,bytes+32);
    capture_lcd_players_t s={.enabled=true,.wifi_enabled=true,.paired={true,true,true,true},.connection={2,1,3,0},.active={true},.stale_beacon={false,false,true},.consuming={true}};
    strcpy(s.ip[0],"192.168.1.242");strcpy(s.held[0],"A+LS-R");
    s.diagnostics[0]=true;s.submitted[0]=1234;s.consumed[0]=1233;
    uint8_t *wifi=calloc(1,bytes);assert(wifi);lcd_players_draw(wifi,&s,-1);
    s.direct_usb=true;memset(memory+16,0,bytes);lcd_players_draw(memory+16,&s,-1);
    bool changed=false;
    for(unsigned y=24;y<64;y++) {
        if(memcmp(wifi+(y*640+8)*3,memory+16+(y*640+8)*3,110*3))changed=true;
        assert(!memcmp(wifi+(y*640+124)*3,memory+16+(y*640+124)*3,(640-124)*3));
    }
    assert(changed); /* Source label switches; P2-P4 stay intact. */
    s.direct_usb=false;memset(memory+16,0,bytes);lcd_players_draw(memory+16,&s,-1);
    assert(!memcmp(wifi,memory+16,bytes));free(wifi);
    memset(memory+16,0,bytes);lcd_players_draw(memory+16,&s,0);
    for(unsigned i=0;i<16;i++){assert(memory[i]==0xa5);assert(memory[16+bytes+i]==0xa5);}
    s.direct_usb=true;
    memset(memory+16,0,bytes);lcd_players_draw(memory+16,&s,0);
    assert(memory[16+(30*640+124)*3]!=0); /* Wi-Fi slots remain visible beside direct P1. */
    strcpy(s.ip[1],"192.168.1.243");s.connection[1]=2;s.diagnostics[1]=true;
    s.consuming[1]=true;s.submitted[1]=UINT32_MAX;s.consumed[1]=UINT32_MAX;
    for(int selected=0;selected<4;selected++)lcd_players_draw(memory+16,&s,selected);
    memset(memory+16,0,bytes);lcd_players_draw(memory+16,&s,-1);
    /* The default view leaves video below the compact header untouched. */
    for(size_t i=16+94*640*3;i<16+bytes;i++)assert(memory[i]==0);
    lcd_pad_text(memory+16,4,69,"ETHERNET 192.168.1.218  WI-FI 192.168.1.169");
    uint8_t network[20*640*3];memcpy(network,memory+16+66*640*3,sizeof(network));
    if(argc>2)lcd_players_draw(memory+16,&s,atoi(argv[2]));
    assert(!memcmp(network,memory+16+66*640*3,sizeof(network)));
    for(unsigned i=0;i<16;i++){assert(memory[i]==0xa5);assert(memory[16+bytes+i]==0xa5);}
    if(argc>1){FILE *f=fopen(argv[1],"wb");assert(f);fprintf(f,"P6\n640 480\n255\n");for(size_t p=16;p<16+bytes;p+=3){fputc(memory[p+2],f);fputc(memory[p+1],f);fputc(memory[p],f);}fclose(f);}
    free(memory);puts("LCD player strip bounds and hit targets: PASS");
}
