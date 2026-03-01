#include "radio.h"
#include "cmt2300a.h"


void Radio_Init(void)
{
    CMT2300A_Init();

    delay(20);

    CMT2300A_GoStby();

    delay(10);

    CMT2300A_GoRx();

    delay(50);

    CMT2300A_ClearInterruptFlags();
}


/* =========================
   REQUIRED BY tuya_rf_libretiny.cpp
   ========================= */

void StartRx(void)
{
    CMT2300A_GoStby();

    delay(5);

    CMT2300A_ClearInterruptFlags();

    delay(5);

    CMT2300A_GoRx();

    delay(50);

    CMT2300A_ClearInterruptFlags();
}


void StartTx(void)
{
    CMT2300A_GoStby();

    delay(5);

    CMT2300A_ClearInterruptFlags();

    delay(5);

    CMT2300A_GoTx();

    delay(10);
}


/* Optional nhưng tốt cho stability */

void RestartRx(void)
{
    CMT2300A_GoSleep();

    delay(5);

    CMT2300A_GoStby();

    delay(5);

    CMT2300A_ClearInterruptFlags();

    delay(5);

    CMT2300A_GoRx();

    delay(50);

    CMT2300A_ClearInterruptFlags();
}
