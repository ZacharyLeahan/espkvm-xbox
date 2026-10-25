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
    assert(lcd_players_hit(466,30)==-1);assert(lcd_players_hit(10,46)==-1);
    size_t bytes=640*480*3;
    uint8_t *memory=malloc(bytes+32);assert(memory);memset(memory,0xa5,bytes+32);
    capture_lcd_players_t s={.enabled=true,.connection={2,1,3,0},.active={true},.stale_beacon={false,false,true},.consuming={true}};
    strcpy(s.ip[0],"192.168.1.242");strcpy(s.held[0],"A+LS-R");
    s.diagnostics[0]=true;s.submitted[0]=1234;s.consumed[0]=1233;
    memset(memory+16,0,bytes);lcd_players_draw(memory+16,&s,0);
    for(unsigned i=0;i<16;i++){assert(memory[i]==0xa5);assert(memory[16+bytes+i]==0xa5);}
    if(argc>1){FILE *f=fopen(argv[1],"wb");assert(f);fprintf(f,"P6\n640 480\n255\n");for(size_t p=16;p<16+bytes;p+=3){fputc(memory[p+2],f);fputc(memory[p+1],f);fputc(memory[p],f);}fclose(f);}
    free(memory);puts("LCD player strip bounds and hit targets: PASS");
}
