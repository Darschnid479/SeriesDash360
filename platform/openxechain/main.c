#include <stdint.h>
#include <stdio.h>
#include <xecore/xam.h>
#include <xecore/xboxkrnl.h>

static void get_temps(uint8_t *cpu,uint8_t *gpu,uint8_t *edram,uint8_t *board) {
    uint8_t in[16]={0x07}, out[16]={0};
    HalSendSMCMessage(in,out);
    *cpu=out[1]; *gpu=out[2]; *edram=out[3]; *board=out[4];
}

int main(void) {
    uint8_t cpu=0,gpu=0,edram=0,board=0;
    get_temps(&cpu,&gpu,&edram,&board);
    printf("SeriesDash360 OpenXeChain runtime 1.0\n");
    printf("CPU %u C GPU %u C EDRAM %u C BOARD %u C\n",
           (unsigned)cpu,(unsigned)gpu,(unsigned)edram,(unsigned)board);
    printf("OpenXeChain bootstrap target is running.\n");
    for (;;) { }
    return 0;
}
