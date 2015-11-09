/* Copyright 2015 Jason Rosenman
 * BPM detection for the K64F using hwtimers
 */
#include "fsl_gpio_driver.h"
#include "fsl_pit_driver.h"
#include "board.h"
#include "bpm.h"

#define BPM_DEFAULT_COUNT	0xffffffff

static struct bpm_dev dev;

void PIT0_IRQHandler(void* data)
{
	PIT_DRV_ClearIntFlag(0, 0);
	GPIO_DRV_TogglePinOutput(kGpioLEDW);
	switch (dev.mode) {
	case BPM_MODE_RUN:
		if (dev.callback) {
			dev.callback();
		}
		break;
	case BPM_MODE_LEARN:
		// timeout
		bpm_update_div(dev.div);
		dev.mode = BPM_MODE_RUN;
		break;
	case BPM_MODE_STOP:
		PIT_DRV_StopTimer(0, 0);
		break;
	}
}

void PORTC_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending(kGpioSW2)) {
		static int learn_state;
		static uint32_t times[4];
		GPIO_DRV_ClearPinIntFlag(kGpioSW2);
		// enter "learn" mode
		if (dev.mode != BPM_MODE_LEARN) {
			learn_state = 0;
			dev.mode = BPM_MODE_LEARN;
			PIT_DRV_StopTimer(0, 0);
			PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
			goto out;
		}
		times[learn_state] = BPM_DEFAULT_COUNT - PIT_DRV_ReadTimerCount(0, 0);
		PIT_DRV_StopTimer(0, 0);
		if (learn_state == 3) {
			uint64_t avg = 0;
			avg = (times[0] + times[1] + times[2] + times[3]) >> 2;
			PIT_DRV_SetTimerPeriodByCount(0, 0, avg / dev.div);
			learn_state = 0;
			dev.mode = BPM_MODE_RUN;
			goto out;
		}
		PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
		learn_state++;
out:
		GPIO_SW_DELAY;
		PIT_DRV_StartTimer(0, 0);
	}
}

void bpm_update_div(int div)
{
	dev.div = div;
	PIT_DRV_StopTimer(0, 0);
	PIT_DRV_SetTimerPeriodByCount(0, 0, PIT_DRV_ReadTimerCount(0, 0) / div);
	PIT_DRV_StartTimer(0, 0);
}

// Define device configuration.
const pit_user_config_t pitInit = {
    .isInterruptEnabled = true, // Enable timer interrupt.
    .periodUs = 20U             // Set timer period to 20 us.
};

void bpm_init(void (*callback)(void))
{
	dev.callback = callback;
	dev.mode = BPM_MODE_RUN;
	dev.div = 1;
	PIT_DRV_Init(0, false);
	PIT_DRV_InitChannel(0, 0, &pitInit);
	PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
	PIT_DRV_StartTimer(0, 0);
}
