#include "radio.h"
#include "cmt2300a.h"



static void rf_rx_clean()
{
    CMT2300A_GoStby();

    delay(3);

    CMT2300A_ClearInterruptFlags();
    CMT2300A_ClearRxFifo();

    delay(3);

    CMT2300A_GoRx();

    delay(10);
}



void Radio_Init(void)
{
    CMT2300A_Init();

    delay(20);

    rf_rx_clean();
}



int StartRx(void)
{
    rf_rx_clean();
    return 1;
}



int RestartRx(void)
{
    rf_rx_clean();
    return 1;
}



int StartTx(void)
{
    CMT2300A_GoStby();

    delay(4);

    CMT2300A_GoTx();

    delay(4);

    return 1;
}
