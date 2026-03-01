#include "radio.h"
#include "cmt2300a.h"



void Radio_Init(void)
{
    CMT2300A_Init();

    delay(30);

    CMT2300A_GoStby();

    delay(20);

    CMT2300A_ClearInterruptFlags();
    CMT2300A_ClearRxFifo();

    delay(10);

    CMT2300A_GoRx();

    delay(50);

    CMT2300A_ClearInterruptFlags();
    CMT2300A_ClearRxFifo();
}



/* ========================= */
/* REQUIRED BY tuya_rf_libretiny */
/* ========================= */

int StartRx(void)
{

    CMT2300A_GoStby();

    delay(10);

    CMT2300A_ClearInterruptFlags();
    CMT2300A_ClearRxFifo();

    delay(10);

    CMT2300A_GoRx();

    delay(40);

    CMT2300A_ClearInterruptFlags();
    CMT2300A_ClearRxFifo();

    return 1;
}



int StartTx(void)
{

    CMT2300A_GoStby();

    delay(10);

    CMT2300A_ClearInterruptFlags();

    delay(5);

    CMT2300A_GoTx();

    delay(10);

    return 1;
}



int RestartRx(void)
{

    CMT2300A_GoSleep();

    delay(10);

    CMT2300A_GoStby();

    delay(10);

    CMT2300A_ClearInterruptFlags();
    CMT2300A_ClearRxFifo();

    delay(10);

    CMT2300A_GoRx();

    delay(40);

    CMT2300A_ClearInterruptFlags();
    CMT2300A_ClearRxFifo();

    return 1;
}
