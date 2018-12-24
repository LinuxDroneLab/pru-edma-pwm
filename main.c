#include <include/resource_table.h>
#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <pru_rpmsg.h>
#include <pru_rpmsg_lib.h>
#include <pru_edma_pwmss_driver.h>
#include <pru-edma.h>
#include <sys_pwmss.h>

#define VIRTIO_CONFIG_S_DRIVER_OK       4

/**
 * main.c
 */
uint32_t* CM_PER_TPCC_CLKCTRL  = (uint32_t*) 0x44E000BC;
uint32_t* CM_PER_TPTC0_CLKCTRL = (uint32_t*) 0x44E00024;

char* DATA_MEMORY_BASE_ADDRESS = (char*)0x4A300000;

// buffer src dei segnali da inviare
#define NUM_FRAME_BLOCK 17

uint16_t FRAME_TO_TRANSFER[NUM_FRAME_BLOCK] = { 125, 63, 63, 125, 125, 63, 125,
                                           125, 125, 63, 63, 63, 125, 63,
                                           125, 63, 1 };

//uint16_t* frameToTransferPtr = FRAME_TO_TRANSFER;

int main(void)
{

    PruRpmsgLibConfig rpmsgConfig;
    PruDShotLibConfig pwmssConfig = {0};

    volatile unsigned char *status;

    /* Allow OCP master port access by the PRU so the PRU can read external memories */
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    /* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
    CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

    /* Make sure the Linux drivers are ready for RPMsg communication */
    /* this is another place where a hang could occur */
    status = &resourceTable.rpmsg_vdev.status;
    while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK))
        ;

    /* Initialize the RPMsg transport structure */
    /* this function is defined in rpmsg_pru.c.  It's sole purpose is to call pru_virtqueue_init twice (once for
     vring0 and once for vring1).  pru_virtqueue_init is defined in pru_virtqueue.c.  It's sole purpose is to
     call vring_init.  Not sure yet where that's defined, but it appears to be part of the pru_rpmsg iface.*/
    /* should probably test for RPMSG_SUCCESS.  If not, then the interface is not working and code should halt */
    pru_rpmsg_init(&rpmsgConfig.transport, &resourceTable.rpmsg_vring0,
                   &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

    /* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
    // In a real-time environment, rather than waiting forever, this can probably be run loop-after-loop
    // until success is achieved.  At that point, set a flag and then enable the send/receive functionality
    while (pru_rpmsg_channel(RPMSG_NS_CREATE, &rpmsgConfig.transport, CHAN_NAME, CHAN_DESC,
    CHAN_PORT) != PRU_RPMSG_SUCCESS)
        ;

    (*CM_PER_TPCC_CLKCTRL) = 0x02; // enable tpcc module
    (*CM_PER_TPTC0_CLKCTRL) = 0x02; // enable tptc0 module

    // Configura PWM
    pru_edma_pwmss_lib_Start(0); // enable clock
    pru_edma_pwmss_lib_Init(0);  // initialize
    pru_edma_pwmss_lib_Stop(0);  // stop clock


    // inizializzare EDMA
    // PaRAM
    (EDMA_PaRAM)->optBits.tcc = 14; // Transfer Complete interrupt enabled; TCC = 14
    (EDMA_PaRAM)->src = (uint32_t)(DATA_MEMORY_BASE_ADDRESS + (uint32_t)FRAME_TO_TRANSFER);
    (EDMA_PaRAM)->acnt = 2;
    (EDMA_PaRAM)->bcnt = 17;
    (EDMA_PaRAM)->dst = 0x48300212; // cmpa
    (EDMA_PaRAM)->srcbidx = 2;
    (EDMA_PaRAM)->dstbidx = 0;
    (EDMA_PaRAM)->link = 0x4020;
    (EDMA_PaRAM)->bcntrld = 0;
    (EDMA_PaRAM)->srccidx = 0;
    (EDMA_PaRAM)->dstcidx = 0;
    (EDMA_PaRAM)->ccnt = 1;

    // PaRAM Link
    (EDMA_PaRAM + 1)->optBits.tcc = 14; // Transfer Complete interrupt enabled; TCC = 14
    (EDMA_PaRAM + 1)->src = (uint32_t)(DATA_MEMORY_BASE_ADDRESS + (uint32_t)FRAME_TO_TRANSFER);
    (EDMA_PaRAM + 1)->acnt = 2;
    (EDMA_PaRAM + 1)->bcnt = 17;
    (EDMA_PaRAM + 1)->dst = 0x48300212;
    (EDMA_PaRAM + 1)->srcbidx = 2;
    (EDMA_PaRAM + 1)->dstbidx = 0;
    (EDMA_PaRAM + 1)->link = 0x4020;
    (EDMA_PaRAM + 1)->bcntrld = 0;
    (EDMA_PaRAM + 1)->srccidx = 0;
    (EDMA_PaRAM + 1)->dstcidx = 0;
    (EDMA_PaRAM + 1)->ccnt = 1;

    CT_TCC.TCC_DCHMAP_bit[EDMA3CC_PWM0_EVT].TCC_DCHMAP_PAENTRY = 0;
    CT_TCC.TCC_DMAQNUM_bit[1].TCC_DMAQNUM_E7 = 0; // coda 0 per channel 14

    CT_TCC.TCC_ECR |= (1 << 14); // clear evento pwm0 (14)
    CT_TCC.TCC_IECR |= (1 << 14); // clear interrupt pwm0 (14)
    CT_TCC.TCC_SECR |= (1 << 14); // clear secondary event pwm0 (14)
    CT_TCC.TCC_ICR |= (1 << 14); // clear interrupt pwm0 (14)
    CT_TCC.TCC_EMCR |= (1 << 14); // clear event missing pwm0 (14)

    CT_TCC.TCC_EESR = (1 << 14); // abilito evento pwm0 (14)
    CT_TCC.TCC_IESR = (1 << 14); // abilito interrupt pwm0 (14)
    CT_TCC.TCC_DRAE[1] |= (1 << 14); // abilito evento pwm0 su region 1

    /* Compose RC lib with RPMSG lib */
    pwmssConfig.onStart = pru_rpmsg_lib_Send;
    pwmssConfig.onStop = pru_rpmsg_lib_Send;
    pwmssConfig.onSetData = pru_rpmsg_lib_Send;
    pwmssConfig.onSetDuty = pru_rpmsg_lib_Send;
    pwmssConfig.onSetPeriod = pru_rpmsg_lib_Send;
    pru_edma_pwmss_lib_Conf(&pwmssConfig);

    rpmsgConfig.onReceived = pru_edma_pwmss_lib_ExecCmd;
    rpmsgConfig.hostInt = HOST_INT;
    rpmsgConfig.fromArmHost = FROM_ARM_HOST;
    pru_rpmsg_lib_Conf(&rpmsgConfig);


    CT_TCC.TCC_ECR |= (1 << 14); // clear evento pwm0 (14)
    CT_TCC.TCC_IECR |= (1 << 14); // clear interrupt pwm0 (14)
    CT_TCC.TCC_SECR |= (1 << 14); // clear secondary event pwm0 (14)
    CT_TCC.TCC_ICR |= (1 << 14); // clear interrupt pwm0 (14)
    CT_TCC.TCC_EMCR |= (1 << 14); // clear event missing pwm0 (14)

    // go!!
    pru_edma_pwmss_lib_Start(0);

    // rimanere in loop per ricevere comandi. Anche uscendo il segnale continua
    // il segnala prodotto da PWM deve corrispondere a quanto definito nel buffer src.
    while (1)
    {
        if(PWMSS0.EPWM_ETFLG & 0x0001) {
            PWMSS0.EPWM_ETCLR = 1;
        }

        pru_rpmsg_lib_Pulse();
    }

}
