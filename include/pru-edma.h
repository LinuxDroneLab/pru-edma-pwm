/*
 * edma.h
 *
 *  Created on: 22 dic 2018
 *      Author: andrea
 */

#ifndef PRU_EDMA_H_
#define PRU_EDMA_H_

#include <stdint.h>

typedef struct {

    /* PRU_TCC_PID register bit field */
    volatile uint16_t TCC_PID;         // 0x0 - 0x01
    uint16_t rsvd01;                    // 0x02 - 0x03

    /* PRU_TCC_CCCFG register bit field */
    union {
        volatile uint32_t TCC_CCCFG;

        volatile struct {
            unsigned TCC_CCCFG_NUM_DMACH    : 3;       //00:02
            unsigned TCC_CCCFG_RSVD6        : 1;       //03:03
            unsigned TCC_CCCFG_NUM_QDMACH   : 3;       //06:04
            unsigned TCC_CCCFG_RSVD5        : 1;       //07:07
            unsigned TCC_CCCFG_NUM_INTCH    : 3;       //10:08
            unsigned TCC_CCCFG_RSVD4        : 1;       //11:11
            unsigned TCC_CCCFG_NUM_PAENTRY  : 3;       //14:12
            unsigned TCC_CCCFG_RSVD3        : 1;       //15:15
            unsigned TCC_CCCFG_NUM_EVQUE    : 3;       //18:16
            unsigned TCC_CCCFG_RSVD2        : 1;       //19:19
            unsigned TCC_CCCFG_NUM_REGN     : 2;       //21:20
            unsigned TCC_CCCFG_RSVD1        : 2;       //23:22
            unsigned TCC_CCCFG_CHMAP_EXIST  : 1;       //24:24
            unsigned TCC_CCCFG_MP_EXIST     : 1;       //25:25
            unsigned TCC_CCCFG_RSVD0        : 6;       //31:26
        } TCC_CCCFG_bit;
    };  // 0x04 - 0x07

    uint32_t rsvd02[2];                 // 0x08 - 0x0F

    /* PRU_TCC_SYSCONFIG register bit field */
    union {
        volatile uint32_t TCC_SYSCONFIG;

        volatile struct {
            unsigned TCC_SYSCONFIG_RSVD2        : 2;       //01:00
            unsigned TCC_SYSCONFIG_IDLEMODE     : 2;       //03:02
            unsigned TCC_SYSCONFIG_RSVD1        : 28;      //31:04
        } TCC_SYSCONFIG_bit;
    };  // 0x10 - 0x13

    uint32_t rsvd03[59];                     // 0x14 - 0xFF

    /* PRU_TCC_DCHMAP register bit field array*/
    union {
        volatile uint32_t TCC_DCHMAP[64];

        volatile struct {
            unsigned TCC_DCHMAP_RSVD2        : 5;       //04:00
            unsigned TCC_DCHMAP_PAENTRY      : 9;       //13:05
            unsigned TCC_DCHMAP_RSVD1        : 18;      //31:14
        } TCC_DCHMAP_bit[64];
    };  // 0x100 - 0x1FF

    uint32_t TCC_QCHMAP[8];                     // 0x200 - 0x21F
    uint32_t rsvd04[8];                          // 0x220 - 0x23F

    /* PRU_TCC_DMAQNUM register bit field array*/
    union {
        volatile uint32_t TCC_DMAQNUM[8];

        volatile struct {
            unsigned TCC_DMAQNUM_E0           : 3;       //02:00
            unsigned TCC_DMAQNUM_RSVD0        : 1;       //03:03
            unsigned TCC_DMAQNUM_E1           : 3;       //06:04
            unsigned TCC_DMAQNUM_RSVD1        : 1;       //07:07
            unsigned TCC_DMAQNUM_E2           : 3;       //10:08
            unsigned TCC_DMAQNUM_RSVD2        : 1;       //11:11
            unsigned TCC_DMAQNUM_E3           : 3;       //14:12
            unsigned TCC_DMAQNUM_RSVD3        : 1;       //15:15
            unsigned TCC_DMAQNUM_E4           : 3;       //18:16
            unsigned TCC_DMAQNUM_RSVD4        : 1;       //19:19
            unsigned TCC_DMAQNUM_E5           : 3;       //22:20
            unsigned TCC_DMAQNUM_RSVD5        : 1;       //23:23
            unsigned TCC_DMAQNUM_E6           : 3;       //26:26
            unsigned TCC_DMAQNUM_RSVD6        : 1;       //27:27
            unsigned TCC_DMAQNUM_E7           : 3;       //30:28
            unsigned TCC_DMAQNUM_RSVD7        : 1;       //31:31
        } TCC_DMAQNUM_bit[8];
    };  // 0x240 - 0x25F

    uint32_t TCC_QDMAQNUM;                     // 0x260 - 0x263
    uint32_t rsvd05[8];                         // 0x264 - 0x283

    /* PRU_TCC_QUEPRI register bit field */
    union {
        volatile uint32_t TCC_QUEPRI;

        volatile struct {
            unsigned TCC_QUEPRI_PRIQ0        : 3;       //00:02
            unsigned TCC_QUEPRI_RSVD0        : 1;       //03:03
            unsigned TCC_QUEPRI_PRIQ1        : 3;       //04:06
            unsigned TCC_QUEPRI_RSVD1        : 1;       //07:07
            unsigned TCC_QUEPRI_PRIQ2        : 3;       //10:08
            unsigned TCC_QUEPRI_RSVD2        : 21;      //31:11
        } TCC_QUEPRI_bit;
    };  // 0x284 - 0x287

    uint32_t rsvd06[6];                         // 0x288 - 0x29F
    uint32_t TCC_EMR;                           // 0x300 - 0x303
    uint32_t TCC_EMRH;                          // 0x304 - 0x307
    uint32_t TCC_EMCR;                          // 0x308 - 0x30B
    uint32_t TCC_EMCRH;                         // 0x30C - 0x30F
    uint32_t TCC_QEMR;                          // 0x310 - 0x313
    uint32_t TCC_QEMCR;                         // 0x314 - 0x317
    uint32_t TCC_CCERR;                         // 0x318 - 0x31B
    uint32_t TCC_CCERRCLR;                      // 0x31C - 0x31F
    uint32_t TCC_EEVAL;                         // 0x320 - 0x323
    uint32_t rsvd07[16];                        // 0x324 - 0x33F
    uint64_t TCC_DRAE[8];                       // 0x340 - 0x37F
    uint64_t TCC_QRAE[8];                       // 0x380 - 0x3BF
    uint32_t rsvd08[16];                        // 0x3C0 - 0x3FF
    uint32_t TCC_Q0E[16];                       // 0x400 - 0x43F
    uint32_t TCC_Q1E[16];                       // 0x440 - 0x47F
    uint32_t TCC_Q2E[16];                       // 0x480 - 0x4BF
    uint32_t rsvd09[80];                        // 0x4C0 - 0x5FF
    uint32_t TCC_QSTAT[3];                      // 0x600 - 0x60B
    uint32_t rsvd10[5];                         // 0x60C - 0x61F
    uint32_t TCC_QWMTHRA;                       // 0x620 - 0x623
    uint32_t rsvd11[7];                         // 0x624 - 0x63F
    uint32_t TCC_CCSTAT;                        // 0x640 - 0x643
    uint32_t rsvd12[111];                       // 0x644 - 0x7FF
    uint32_t TCC_MPFAR;                         // 0x800 - 0x803
    uint32_t TCC_MPFSR;                         // 0x804 - 0x807
    uint32_t TCC_MPFCR;                         // 0x808 - 0x80B
    uint32_t TCC_MPPAG;                         // 0x80C - 0x80F
    uint32_t TCC_MPPA[8];                       // 0x810 - 0x82F
    uint32_t rsvd13[500];                       // 0x830 - 0xFFF
    uint32_t TCC_ER;                            // 0x1000 - 0x1003
    uint32_t TCC_ERCH;                          // 0x1004 - 0x1007
    uint32_t TCC_ECR;                           // 0x1008 - 0x100B
    uint32_t TCC_ECRH;                          // 0x100C - 0x100F
    uint32_t TCC_ESR;                           // 0x1010 - 0x1013
    uint32_t TCC_ESRH;                          // 0x1014 - 0x1017
    uint32_t TCC_CER;                           // 0x1018 - 0x101B
    uint32_t TCC_CERH;                          // 0x101C - 0x101F
    uint32_t TCC_EER;                           // 0x1020 - 0x1023
    uint32_t TCC_EERH;                          // 0x1024 - 0x1027
    uint32_t TCC_EECR;                          // 0x1028 - 0x102B
    uint32_t TCC_EECRH;                         // 0x102C - 0x102F
    uint32_t TCC_EESR;                          // 0x1030 - 0x1033
    uint32_t TCC_EESRH;                         // 0x1034 - 0x1037
    uint32_t TCC_SER;                           // 0x1038 - 0x103B
    uint32_t TCC_SERH;                          // 0x103C - 0x103F
    uint32_t TCC_SECR;                          // 0x1040 - 0x1043
    uint32_t TCC_SECRH;                         // 0x1044 - 0x1047
    uint32_t rsvd14[2];                         // 0x1048 - 0x104F
    uint32_t TCC_IER;                           // 0x1050 - 0x1053
    uint32_t TCC_IERH;                          // 0x1054 - 0x1057
    uint32_t TCC_IECR;                          // 0x1058 - 0x105B
    uint32_t TCC_IECRH;                         // 0x105C - 0x105F
    uint32_t TCC_IESR;                          // 0x1060 - 0x1063
    uint32_t TCC_IESRH;                         // 0x1064 - 0x1067
    uint32_t TCC_IPR;                           // 0x1068 - 0x106B
    uint32_t TCC_IPRH;                          // 0x106C - 0x106F
    uint32_t TCC_ICR;                           // 0x1070 - 0x1073
    uint32_t TCC_ICRH;                          // 0x1074 - 0x1077
    uint32_t TCC_IEVAL;                         // 0x1078 - 0x107B
    uint32_t rsvd15;                            // 0x107C - 0x107F
    uint32_t TCC_QER;                           // 0x1080 - 0x1083
    uint32_t TCC_QEER;                          // 0x1084 - 0x1087
    uint32_t TCC_QEECR;                         // 0x1088 - 0x108B
    uint32_t TCC_QEESR;                         // 0x108C - 0x108F
    uint32_t TCC_QSER;                          // 0x1090 - 0x1093
    uint32_t TCC_QSECR;                         // 0x1094 - 0x1097

} pruTcc;

typedef struct {
    /* PRU_TPTC_PID register bit field */
    union {
        volatile uint32_t TPTC_PID;

        volatile struct {
            unsigned TPTC_PID_PID         : 16;       //15:00
            unsigned TPTC_PID_RSVD        : 16;       //31:16
        } TPTC_PID_bit;
    };  // 0x00 - 0x03
    uint32_t TPTC_TCCFG;                         // 0x04 - 0x07
    uint32_t rsvd01[2];                          // 0x08 - 0x0F
    uint32_t TPTC_SYCONFIG;                      // 0x10 - 0x13
    uint32_t rsvd02[59];                         // 0x14 - 0x0FF
    uint32_t TPTC_TCSTAT;                        // 0x100 - 0x103
    uint32_t rsvd03[7];                          // 0x104 - 0x11F
    uint32_t TPTC_ERRSTAT;                       // 0x120 - 0x123
    uint32_t TPTC_ERREN;                         // 0x124 - 0x127
    uint32_t TPTC_ERRCLR;                        // 0x128 - 0x12B
    uint32_t TPTC_ERRDET;                        // 0x12C - 0x12F
    uint32_t TPTC_ERRCMD;                        // 0x130 - 0x133
    uint32_t rsvd04[3];                          // 0x134 - 0x13F
    uint32_t TPTC_RDRATE;                        // 0x140 - 0x143
    uint32_t rsvd05[63];                         // 0x144 - 0x23F
    uint32_t TPTC_SAOPT;                         // 0x240 - 0x243
    uint32_t TPTC_SASRC;                         // 0x244 - 0x247
    uint32_t TPTC_SACNT;                         // 0x248 - 0x24B
    uint32_t TPTC_SADST;                         // 0x24C - 0x24F
    uint32_t TPTC_SABIDX;                        // 0x250 - 0x253
    uint32_t TPTC_SAMPPRXY;                      // 0x254 - 0x257
    uint32_t TPTC_SACNTRLD;                      // 0x258 - 0x25B
    uint32_t TPTC_SASRCBREF;                     // 0x25C - 0x25F
    uint32_t TPTC_SADSTBREF;                     // 0x260 - 0x263
    uint32_t rsvd06[7];                          // 0x264 - 0x27F
    uint32_t TPTC_DFCTRLD;                       // 0x280 - 0x283
    uint32_t TPTC_DFSRCBREF;                     // 0x284 - 0x287
    uint32_t TPTC_DFDSTBREF;                     // 0x288 - 0x28B
    uint32_t rsvd07[29];                         // 0x28C - 0x2FF

    uint32_t TPTC_DFOPT0;                        // 0x300 - 0x303
    uint32_t TPTC_DFSRC0;                        // 0x304 - 0x307
    uint32_t TPTC_DFCNT0;                        // 0x308 - 0x30B
    uint32_t TPTC_DFDST0;                        // 0x30C - 0x30F
    uint32_t TPTC_DFBIDX0;                       // 0x310 - 0x313
    uint32_t TPTC_DFMPPRXY0;                     // 0x314 - 0x317
    uint32_t rsvd08[10];                         // 0x318 - 0x33F
    uint32_t TPTC_DFOPT1;                        // 0x340 - 0x343
    uint32_t TPTC_DFSRC1;                        // 0x344 - 0x347
    uint32_t TPTC_DFCNT1;                        // 0x348 - 0x34B
    uint32_t TPTC_DFDST1;                        // 0x34C - 0x34F
    uint32_t TPTC_DFBIDX1;                       // 0x350 - 0x353
    uint32_t TPTC_DFMPPRXY1;                     // 0x354 - 0x357
    uint32_t rsvd09[10];                         // 0x358 - 0x37F
    uint32_t TPTC_DFOPT2;                        // 0x380 - 0x383
    uint32_t TPTC_DFSRC2;                        // 0x384 - 0x387
    uint32_t TPTC_DFCNT2;                        // 0x388 - 0x38B
    uint32_t TPTC_DFDST2;                        // 0x38C - 0x38F
    uint32_t TPTC_DFBIDX2;                       // 0x390 - 0x393
    uint32_t TPTC_DFMPPRXY2;                     // 0x394 - 0x397
    uint32_t rsvd10[10];                         // 0x398 - 0x3BF
    uint32_t TPTC_DFOPT3;                        // 0x3C0 - 0x3C3
    uint32_t TPTC_DFSRC3;                        // 0x3C4 - 0x3C7
    uint32_t TPTC_DFCNT3;                        // 0x3C8 - 0x3CB
    uint32_t TPTC_DFDST3;                        // 0x3CC - 0x3CF
    uint32_t TPTC_DFBIDX3;                       // 0x3D0 - 0x3D3
    uint32_t TPTC_DFMPPRXY3;                     // 0x3D4 - 0x3D7

} pruTptc;

volatile __far pruTcc CT_TCC __attribute__((cregister("TPCC", near), peripheral));
volatile __far pruTptc CT_TPTC0 __attribute__((cregister("TPTC0", near), peripheral));
volatile __far pruTptc CT_TPTC1 __attribute__((cregister("TPTC1", near), peripheral));
volatile __far pruTptc CT_TPTC2 __attribute__((cregister("TPTC2", near), peripheral));

#define EDMA_BASE_ADDRESS      0x49000000
#define EDMA_0_PARAM           0x49004000

typedef struct {
    union {
        volatile uint32_t opt;
        volatile struct {
            unsigned sam          : 1;       //00:00
            unsigned dam          : 1;       //01:01
            unsigned syncdim      : 1;       //02:02
            unsigned isStatic     : 1;       //03:03
            unsigned rsv03        : 4;       //07:04
            unsigned fwid         : 3;       //10:08
            unsigned tccmod       : 1;       //11:11
            unsigned tcc          : 6;       //17:12
            unsigned rsv02        : 2;       //19:18
            unsigned tcinten      : 1;       //20:20
            unsigned itcinten     : 1;       //21:21
            unsigned tcchen       : 1;       //22:22
            unsigned itcchen      : 1;       //23:23
            unsigned privId       : 4;       //27:24
            unsigned rsv01        : 3;       //30:28
            unsigned priv         : 1;       //31:31

        } optBits;
    };
    uint32_t src;
    uint32_t acnt  : 16;
    uint32_t bcnt  : 16;
    uint32_t dst;
    uint32_t srcbidx  : 16;
    uint32_t dstbidx  : 16;
    uint32_t link     : 16;
    uint32_t bcntrld  : 16;
    uint32_t srccidx  : 16;
    uint32_t dstcidx  : 16;
    uint32_t ccnt     : 16;
    uint32_t rsvd     : 16;
} EDMA_PaRAM_STRUCT;

EDMA_PaRAM_STRUCT* EDMA_PaRAM = (EDMA_PaRAM_STRUCT*)EDMA_0_PARAM;


#endif /* PRU_EDMA_H_ */
