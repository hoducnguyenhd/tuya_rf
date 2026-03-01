#include "radio.h"
#include "cmt2300a.h"



static void rf_prepare_rx()
{
    CMT2300A_GoStby();

    delay(4);

    CMT2300A_ClearInterruptFlags();
    CMT2300A_ClearRxFifo();

    delay(4);

    CMT2300A_GoRx();

    delay(15);
}



void Radio_Init(void)
{
    CMT2300A_Init();

    delay(20);

    CMT2300A_GoSleep();

    delay(10);

    rf_prepare_rx();
}



/* ===== REQUIRED ===== */

int StartRx(void)
{
    rf_prepare_rx();
    return 1;
}



int StartTx(void)
{
    CMT2300A_GoStby();

    delay(4);

    CMT2300A_ClearInterruptFlags();

    delay(4);

    CMT2300A_GoTx();

    delay(4);

    return 1;
}



int RestartRx(void)
{
    rf_prepare_rx();
    return 1;
}
