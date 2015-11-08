/* Copyright 2015 Jason Rosenman
 * BPM detection for the K64F using hwtimers
 */
#include "fsl_gpio_driver.h"
#include "fsl_pit_driver.h"
#include "board.h"
#include "bpm.h"

#define BPM_DEFAULT_COUNT	100000000

static struct bpm_dev dev;

void PIT0_IRQHandler(void* data)
{
	PIT_DRV_ClearIntFlag(0, 0);
	GPIO_DRV_TogglePinOutput(kGpioLED2);
	switch (dev.mode) {
	case BPM_MODE_RUN:
		if (dev.callback) {
			dev.callback();
		}
		break;
	case BPM_MODE_LEARN:
		// timeout
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
		}
		GPIO_SW_DELAY;
		switch (learn_state) {
		case 0:
			PIT_DRV_StopTimer(0, 0);
			PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
			learn_state++;
			break;
		case 1:
			times[0] = BPM_DEFAULT_COUNT - PIT_DRV_ReadTimerCount(0, 0);
			PIT_DRV_StopTimer(0, 0);
			PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
			learn_state++;
			break;
		case 2:
			times[1] = BPM_DEFAULT_COUNT - PIT_DRV_ReadTimerCount(0, 0);
			PIT_DRV_StopTimer(0, 0);
			PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
			learn_state++;
			break;
		case 3:
			times[2] = BPM_DEFAULT_COUNT - PIT_DRV_ReadTimerCount(0, 0);
			PIT_DRV_StopTimer(0, 0);
			PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
			learn_state++;
			break;
		case 4:
		{
			uint64_t avg = 0;
			times[3] = BPM_DEFAULT_COUNT - PIT_DRV_ReadTimerCount(0, 0);
			avg = (times[0] + times[1] + times[2] + times[3]) >> 2;
			PIT_DRV_StopTimer(0, 0);
			PIT_DRV_SetTimerPeriodByCount(0, 0, avg);
			learn_state = 0;
			dev.mode = BPM_MODE_RUN;
			break;
		}
		}
		PIT_DRV_StartTimer(0, 0);
	}
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
	PIT_DRV_Init(0, false);
	PIT_DRV_InitChannel(0, 0, &pitInit);
	PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
	PIT_DRV_StartTimer(0, 0);
}
