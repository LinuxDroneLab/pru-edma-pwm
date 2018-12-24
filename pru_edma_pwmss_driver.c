/*
 * pru_edma_pwmss_driver.c
 *
 *  Created on: 12 ago 2018
 *      Author: Andrea Lambruschini
 */
#include <pru_edma_pwmss_driver.h>
#include <sys_pwmss.h>

/*
 * from http://theduchy.ualr.edu/?p=299
 *
 * Do NOT use CLKCONFIG (ePWMCLK_EN) to try to turn the PWM on/off.
 * Once that clock is turned off, it cannot be turned back on from within the PRU.
 * The CM_PER register (0x44E0 0000) contains three important registers:
 *      epwmss0clkctrl (0xCC),
 *      epwmss1clkctrl (0xD4),
 *      epwmss2clkctrl (0xD8)
 * which have to be set up in order for the pwm to work at all.
 * These registers can be twiddled to turn the pwm off and on.
 *
 * In turning the PWM off, it’s nice to also force the module to go low.
 * This can be done through the ACQTLA/B register.
 * I need to figure this out again.  Not going to happen tonight given how tired I am.
 * Another register that has an interesting module is CONTROL_MODULE (0x44E1 0000).
 * This register has pwmss_ctrl (0x664), which has the bits timebase_clock_enable for pwm 0, 1, 2.
 * I have not checked it out yet, but it could be a necessary register.
 *
 *
 * Set bit 8 of the CLKCONFIG register (ePWMCLK_EN = 1), which enables the clock.
 * Set bits 15, 14 of TBCTL (FREE_SOFT) to 2h to allow free running of the PWM.
 * Set bits 5, 4 of TBCTL (SYNCOSEL) to 3h to disable EPWMxSYNCO signal.
 * Set bits 12-10 of TBCTL (CLKDIV) to 2h to make the prescaler = 4:1.
 * Set bits 9-7 (HSPCLKDIV) to 0 to set the high speed prescaler to 1.
 * This makes TBCLK = SYSCLKOUT/CLKDIV/HSPCLKDIV.
 * If SYSCLKOUT = 100MHz, TBCLK = 25 MHz.
 * Set the other bits to zero (PHSDIR =0, SWFSYNC=0h, PRDLD=0h, PHSEN=0h, CTRMODE=0h).
 * Among other things, these choices set the counter-mode to ‘up-count.’
 * Set TBPRD = 0xnn.  This is the frequency of the PWM.
 * It interacts with TBCLK, such that PWMPERIOD = (TBPRD+1)/TBCLK.
 * If nn = 99, then PWMPERIOD = 100/25 MHz = 4 usec. (confirmed)
 * The CMPA register is compared against TBCNT register and actions taken based on AQCTLA register.
 * Setting AQCTLA (CAU=3h,PRD=0h, ZRO=2h) in count-up mode will give a traditional PWM signal.
 * Duty cycle varies between 0 and TBPRD+1 (100%). (confirmed)
 *
 */

volatile sysPwmss* PRU_PWMSS[3] = { &PWMSS0, &PWMSS1, &PWMSS2 };
uint32_t* CM_PER_PWMCSS_CLKCTRL[3] = { (uint32_t*) 0x44E000D4,
                                       (uint32_t*) 0x44E000CC,
                                       (uint32_t*) 0x44E000D8 };
uint32_t* PWMSS_CTRL_REG = (uint32_t*) 0x44E10664;

uint8_t pru_edma_pwmss_lib_initialized[3] = { 0, 0, 0 };
PruDShotLibConfig* pru_edma_pwmss_lib_config = 0;
unsigned char pru_edma_pwmss_lib_cmd_rsp[4] = { 0 };

//#define NUM_FRAME_BLOCK 17
//uint8_t FRAME_TO_TRANSFER[NUM_FRAME_BLOCK] = { 125, 63, 63, 125, 125, 63, 125,
//                                               125, 125, 63, 63, 63, 125, 63,
//                                               125, 63, 1 };
//uint8_t indexFrame[3] = { 0, 0, 0 };

/***********************************************************************
 * C O N F I G U R A T I O N   O F   P W M S S   A N D   C L O C K S   *
 **********************************************************************/

uint8_t pru_edma_pwmss_lib_IsConfigured(uint8_t pwmssDevice)
{
    return (pru_edma_pwmss_lib_config != 0);
}
uint8_t pru_edma_pwmss_lib_IsInitialized(uint8_t pwmssDevice)
{
    return pru_edma_pwmss_lib_initialized[pwmssDevice];
}
uint8_t pru_edma_pwmss_lib_IsRunning(uint8_t pwmssDevice)
{
    return (PRU_PWMSS[pwmssDevice]->CLKCONFIG_bit.EPWMCLK_EN);
}
uint8_t pru_edma_pwmss_lib_IsCmdSupported(unsigned char* cmd, uint8_t numBytes)
{
    return (numBytes > 1) && (cmd[0] == PRU_EDMA_PWMSS_LIB_CMD_ID)
            && (cmd[1] == PRU_EDMA_PWMSS_LIB_CMD_START)
            || (cmd[1] == PRU_EDMA_PWMSS_LIB_CMD_STOP)
            || (cmd[1] == PRU_EDMA_PWMSS_LIB_CMD_SET_DATA)
            || (cmd[1] == PRU_EDMA_PWMSS_LIB_CMD_SET_DUTY)
            || (cmd[1] == PRU_EDMA_PWMSS_LIB_CMD_SET_PERIOD);
}

uint8_t pru_edma_pwmss_lib_Init(uint8_t pwmssDevice)
{
    /*
     * A T T E N Z I O N E!
     * https://beagleboard.org/discuss?place=msg%2Fbeagleboard%2FLZhL4S9taic%2FK4HCC6d_AgAJ
     * Per utilizzare pwmss da PRU con kernel 4 linux è necessario modificare
     * la configurazione dei ehrpwmX_tbclk aggiungendo ti,set-bit-to-disable
     *
     *     ehrpwm0_tbclk: ehrpwm0_tbclk@44e10664 {
     #clock-cells = <0>;
     compatible = "ti,gate-clock";
     clocks = <&l4ls_gclk>;
     ti,bit-shift = <0>;
     ti,set-bit-to-disable;
     reg = <0x0664>;
     };

     ehrpwm1_tbclk: ehrpwm1_tbclk@44e10664 {
     #clock-cells = <0>;
     compatible = "ti,gate-clock";
     clocks = <&l4ls_gclk>;
     ti,bit-shift = <1>;
     ti,set-bit-to-disable;
     reg = <0x0664>;
     };

     ehrpwm2_tbclk: ehrpwm2_tbclk@44e10664 {
     #clock-cells = <0>;
     compatible = "ti,gate-clock";
     clocks = <&l4ls_gclk>;
     ti,bit-shift = <2>;
     ti,set-bit-to-disable;
     reg = <0x0664>;
     };

     La soluzione è riferita da questa discussione:
     https://groups.google.com/forum/#!msg/beagleboard/eVgyVduT288/XgsiUiNiBwAJ

     La modifica l'ho stata fatta su am335x-boneblack-uboot.dts e funziona correttamente.

     */
    /* PinMux configured with device tree */
    if (pru_edma_pwmss_lib_initialized[pwmssDevice] == 0)
    {
        PRU_PWMSS[pwmssDevice]->CLKCONFIG_bit.EPWMCLK_EN = 0b1; /* enable clock */
        PRU_PWMSS[pwmssDevice]->EPWM_TBCTL = 0xFFFF & (0x8000 | 0x0030); /* FREE RUN on, SUNC0SEL off, CLKDIV = 0 (100MHz) */

        /* Frequenza PWM */
        /* SYSCLKOUT=100MHz, HSPCLKDIV=0
         * CLKDIV = 0 => TBCLK = 100MHz/1 = 100MHz
         * PWMPERIOD = (TBPRD + 1)/TBCLK; a 600KHz deve valere 1670ns
         * per avere un PWMPERIOD di 1670ns serve TBPRD = 1670ns * TBCLK - 1 = 1670ns * 100MHz/1000 - 1 = 166;
         * */
        PRU_PWMSS[pwmssDevice]->EPWM_TBPRD = 0x00A6; // 166
        PRU_PWMSS[pwmssDevice]->EPWM_AQCTLA = 0xFFFF & (0x0003 | 0x0010); // (Toggle quando CNT == 0) ZRO = 2, (Forza Low qando CNT = A)CAU = 1
        PRU_PWMSS[pwmssDevice]->EPWM_AQCTLB = 0xFFFF & (0x0003 | 0x0100); // (Toggle quando CNT == 0) ZRO = 2, (Forza Low qando CNT = B)CBU = 1
        PRU_PWMSS[pwmssDevice]->EPWM_CMPA = 30; //
        PRU_PWMSS[pwmssDevice]->EPWM_CMPB = 0; //

        PRU_PWMSS[pwmssDevice]->EPWM_CMPCTL = 0b0000; // LOAD(A/B)MOD when CNT=0
        PRU_PWMSS[pwmssDevice]->EPWM_ETSEL = 0b1010; // INT abilitata e scatta quando CNT == PRD
        PRU_PWMSS[pwmssDevice]->EPWM_ETPS = 1; // genera interrupt al primo evento
        *PWMSS_CTRL_REG = 7; // enable  pwmss_ctrl Register: bits pwmss2_tbclk, pwmss1_tbclke, pwmss0_tbclke
        pru_edma_pwmss_lib_initialized[pwmssDevice] = 1;
    }
    return 1;
}

uint8_t pru_edma_pwmss_lib_Start(uint8_t pwmssDevice)
{
    if (!pru_edma_pwmss_lib_IsRunning(pwmssDevice))
    {
        (*CM_PER_PWMCSS_CLKCTRL[pwmssDevice]) = 2; // enable module
    }
    return 1;
}
uint8_t pru_edma_pwmss_lib_Stop(uint8_t pwmssDevice)
{
    if (pru_edma_pwmss_lib_IsRunning(pwmssDevice))
    {
        (*CM_PER_PWMCSS_CLKCTRL[pwmssDevice]) = 0; // disable module
    }
    return 1;
}

uint8_t pru_edma_pwmss_lib_ExecCmd(unsigned char* cmd, uint8_t numBytes)
{
    if (cmd[0] == PRU_EDMA_PWMSS_LIB_CMD_ID)
    {
        switch (cmd[1])
        {
        case (PRU_EDMA_PWMSS_LIB_CMD_START):
        {
            pru_edma_pwmss_lib_cmd_rsp[0] = PRU_EDMA_PWMSS_LIB_CMD_ID;
            pru_edma_pwmss_lib_cmd_rsp[1] = PRU_EDMA_PWMSS_LIB_CMD_START_RSP;
            if (numBytes >= 3)
            {
                uint8_t i = 0;
                pru_edma_pwmss_lib_cmd_rsp[2] = cmd[2]; // pwmssDevices to start
                pru_edma_pwmss_lib_cmd_rsp[3] = 1;
                for (i = 0; i < 3; i++)
                {
                    if (cmd[2] & (1 << i))
                    {
                        pru_edma_pwmss_lib_cmd_rsp[3] &= pru_edma_pwmss_lib_Start(i); // result
                    }
                }
                return (pru_edma_pwmss_lib_config->onStart)(pru_edma_pwmss_lib_cmd_rsp, 4)
                        && pru_edma_pwmss_lib_cmd_rsp[3];
            }
            else
            {
                pru_edma_pwmss_lib_cmd_rsp[2] = 0xFF; // error
                pru_edma_pwmss_lib_cmd_rsp[3] = 0; // result KO
                return (pru_edma_pwmss_lib_config->onStart)(pru_edma_pwmss_lib_cmd_rsp, 4)
                        && pru_edma_pwmss_lib_cmd_rsp[3];
            }
        }
        case (PRU_EDMA_PWMSS_LIB_CMD_STOP):
        {
            pru_edma_pwmss_lib_cmd_rsp[0] = PRU_EDMA_PWMSS_LIB_CMD_ID;
            pru_edma_pwmss_lib_cmd_rsp[1] = PRU_EDMA_PWMSS_LIB_CMD_STOP_RSP;
            if (numBytes >= 3)
            {
                uint8_t i = 0;
                pru_edma_pwmss_lib_cmd_rsp[2] = cmd[2]; // pwmssDevices to stop
                pru_edma_pwmss_lib_cmd_rsp[3] = 1;
                for (i = 0; i < 3; i++)
                {
                    if (cmd[2] & (1 << i))
                    {
                        pru_edma_pwmss_lib_cmd_rsp[3] &= pru_edma_pwmss_lib_Stop(i); // result
                    }
                }
                return (pru_edma_pwmss_lib_config->onStop)(pru_edma_pwmss_lib_cmd_rsp, 4)
                        && pru_edma_pwmss_lib_cmd_rsp[3];
            }
            else
            {
                pru_edma_pwmss_lib_cmd_rsp[2] = 0xFF; // error
                pru_edma_pwmss_lib_cmd_rsp[3] = 0; // KO
                return (pru_edma_pwmss_lib_config->onStop)(pru_edma_pwmss_lib_cmd_rsp, 4)
                        && pru_edma_pwmss_lib_cmd_rsp[3];
            }
        }
        case (PRU_EDMA_PWMSS_LIB_CMD_SET_DATA):
        {
            pru_edma_pwmss_lib_cmd_rsp[0] = PRU_EDMA_PWMSS_LIB_CMD_ID;
            pru_edma_pwmss_lib_cmd_rsp[1] = PRU_EDMA_PWMSS_LIB_CMD_SET_DATA_RSP;
            pru_edma_pwmss_lib_cmd_rsp[2] = 0;
            pru_edma_pwmss_lib_cmd_rsp[3] = 1;
            if (numBytes >= 9)
            {
                /*
                 * 1 byte: deviceId
                 * 2 bytes: period
                 * 2 bytes: cmpA
                 * 2 bytes: cmpB
                 * da uno a tre volte
                 */
                uint8_t groups = (numBytes - 2) / 7;
                uint8_t i = 0;
                for (i = 0; i < groups; i++)
                {
                    uint16_t* data = (uint16_t*) (cmd + (i * 7) + 3);
                    uint8_t numDev = (uint8_t) (*(cmd + (i * 7) + 2));
                    pru_edma_pwmss_lib_cmd_rsp[2] |= (1 << numDev);
                    pru_edma_pwmss_lib_cmd_rsp[3] &= pru_edma_pwmss_lib_SetData(numDev,
                                                                      data[0],
                                                                      data[1],
                                                                      data[2]); // result
                }
                return (pru_edma_pwmss_lib_config->onSetData)(pru_edma_pwmss_lib_cmd_rsp,
                                                         4)
                        && pru_edma_pwmss_lib_cmd_rsp[3];
            }
            else
            {
                pru_edma_pwmss_lib_cmd_rsp[2] = 0xFF; // error
                pru_edma_pwmss_lib_cmd_rsp[3] = 0; // KO
                return (pru_edma_pwmss_lib_config->onSetData)(pru_edma_pwmss_lib_cmd_rsp,
                                                         4)
                        && pru_edma_pwmss_lib_cmd_rsp[3];
            }
        }
        case (PRU_EDMA_PWMSS_LIB_CMD_SET_DUTY):
        {
            pru_edma_pwmss_lib_cmd_rsp[0] = PRU_EDMA_PWMSS_LIB_CMD_ID;
            pru_edma_pwmss_lib_cmd_rsp[1] = PRU_EDMA_PWMSS_LIB_CMD_SET_DUTY_RSP;
            pru_edma_pwmss_lib_cmd_rsp[2] = 0; //
            pru_edma_pwmss_lib_cmd_rsp[3] = 1;
            if (numBytes >= 7)
            {
                /*
                 * 1 byte: deviceId
                 * 2 bytes: cmpA
                 * 2 bytes: cmpB
                 * da uno a tre volte
                 */
                uint8_t groups = (numBytes - 2) / 5;
                uint8_t i = 0;
                for (i = 0; i < groups; i++)
                {
                    uint16_t* data = (uint16_t*) (cmd + (i * 5) + 3);
                    uint8_t numDev = (uint8_t) (*(cmd + (i * 5) + 2));
                    pru_edma_pwmss_lib_cmd_rsp[2] |= (1 << numDev);
                    pru_edma_pwmss_lib_cmd_rsp[3] &= pru_edma_pwmss_lib_SetDuty(numDev,
                                                                      data[1],
                                                                      data[2]); // result
                }
                return (pru_edma_pwmss_lib_config->onSetDuty)(pru_edma_pwmss_lib_cmd_rsp,
                                                         4)
                        && pru_edma_pwmss_lib_cmd_rsp[3];
            }
            else
            {
                pru_edma_pwmss_lib_cmd_rsp[2] = 0xFF; // error
                pru_edma_pwmss_lib_cmd_rsp[3] = 0; // KO
                return (pru_edma_pwmss_lib_config->onSetDuty)(pru_edma_pwmss_lib_cmd_rsp,
                                                         4)
                        && pru_edma_pwmss_lib_cmd_rsp[3];
            }
        }
        case (PRU_EDMA_PWMSS_LIB_CMD_SET_PERIOD):
        {
            if (numBytes >= 5)
            {
                /*
                 * 1 byte: deviceId
                 * 2 bytes: period
                 * da uno a tre volte
                 */
                pru_edma_pwmss_lib_cmd_rsp[0] = PRU_EDMA_PWMSS_LIB_CMD_ID;
                pru_edma_pwmss_lib_cmd_rsp[1] = PRU_EDMA_PWMSS_LIB_CMD_SET_PERIOD_RSP;
                pru_edma_pwmss_lib_cmd_rsp[2] = 0; //
                pru_edma_pwmss_lib_cmd_rsp[3] = 1;
                uint8_t groups = (numBytes - 2) / 3;
                uint8_t i = 0;
                for (i = 0; i < groups; i++)
                {
                    uint16_t* data = (uint16_t*) (cmd + (i * 3) + 3);
                    uint8_t numDev = (uint8_t) (*(cmd + (i * 3) + 2));
                    pru_edma_pwmss_lib_cmd_rsp[2] |= (1 << numDev);
                    pru_edma_pwmss_lib_cmd_rsp[3] &= pru_edma_pwmss_lib_SetPeriod(
                            cmd[2], data[1]); // result
                }
                return (pru_edma_pwmss_lib_config->onSetPeriod)(
                        pru_edma_pwmss_lib_cmd_rsp, 4) && pru_edma_pwmss_lib_cmd_rsp[3];
            }
            else
            {
                pru_edma_pwmss_lib_cmd_rsp[2] = 0xFF; // error
                pru_edma_pwmss_lib_cmd_rsp[3] = 0; // KO
                return (pru_edma_pwmss_lib_config->onSetPeriod)(
                        pru_edma_pwmss_lib_cmd_rsp, 4) && pru_edma_pwmss_lib_cmd_rsp[3];
            }
        }
        }
    }
    return 0;
}

uint8_t pru_edma_pwmss_lib_Pulse(uint8_t pwmssDevice)
{
//            if (((*CM_PER_PWMCSS_CLKCTRL[pwmssDevice]) & 0x0002)) // se è attivo ...
//            {
//                if (PRU_PWMSS[pwmssDevice]->EPWM_ETFLG & 0x0001) // se ricevuto interrupt
//                { // finito il periodo carico
//                    PRU_PWMSS[pwmssDevice]->EPWM_ETCLR = 1;
//                    // preparo A/B per il prossimo periodo
//                    PRU_PWMSS[pwmssDevice]->EPWM_CMPA =
//                            FRAME_TO_TRANSFER[indexFrame[pwmssDevice]];
//                    indexFrame[pwmssDevice]++;
//                    indexFrame[pwmssDevice] = (indexFrame[pwmssDevice] == NUM_FRAME_BLOCK) ? 0 : indexFrame[pwmssDevice];
//                }
//            }
    return 1;
}
uint8_t pru_edma_pwmss_lib_Conf(PruDShotLibConfig* config)
{
    pru_edma_pwmss_lib_config = config;
    return 1;
}
uint8_t pru_edma_pwmss_lib_SetData(uint8_t pwmssDevice, uint16_t period,
                              uint16_t duA, uint16_t duB)
{
    PRU_PWMSS[pwmssDevice]->EPWM_TBPRD = period;
    PRU_PWMSS[pwmssDevice]->EPWM_CMPA = duA;
    PRU_PWMSS[pwmssDevice]->EPWM_CMPB = duB;
    return 1;
}
uint8_t pru_edma_pwmss_lib_SetDuty(uint8_t pwmssDevice, uint16_t duA, uint16_t duB)
{
    PRU_PWMSS[pwmssDevice]->EPWM_CMPA = duA;
    PRU_PWMSS[pwmssDevice]->EPWM_CMPB = duB;
    return 1;
}
uint8_t pru_edma_pwmss_lib_SetPeriod(uint8_t pwmssDevice, uint16_t period)
{
    PRU_PWMSS[pwmssDevice]->EPWM_TBPRD = period;
    return 1;
}
