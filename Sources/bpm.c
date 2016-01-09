/* Copyright 2015 Jason Rosenman
 * BPM detection for the K64F using hwtimers
 */
#include "fsl_gpio_driver.h"
#include "fsl_pit_driver.h"
#include "board.h"
#include "buttons.h"
#include "bpm.h"

#define BPM_DEFAULT_COUNT	0xffffffff

static struct bpm_dev dev;

void PIT0_IRQHandler(void* data)
{
	if (PIT_DRV_IsIntPending(0, 0)) {
		PIT_DRV_ClearIntFlag(0, 0);
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
}

void PIT1_IRQHandler(void* data) {
	if (PIT_DRV_IsIntPending(0, 1)) {
		PIT_DRV_ClearIntFlag(0, 1);
		GPIO_DRV_TogglePinOutput(kGpioBPMLED);
	}
}

void bpm_button_callback(void)
{
	static int learn_state;
	static uint32_t times[4];
	// enter "learn" mode
	if (dev.mode != BPM_MODE_LEARN) {
		learn_state = 0;
		dev.mode = BPM_MODE_LEARN;
		PIT_DRV_StopTimer(0, 0);
		PIT_DRV_StopTimer(0, 1);
		PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
		goto out;
	}
	times[learn_state] = BPM_DEFAULT_COUNT - PIT_DRV_ReadTimerCount(0, 0);
	PIT_DRV_StopTimer(0, 0);
	if (learn_state == 3) {
		uint64_t avg = 0;
		avg = (times[0] + times[1] + times[2] + times[3]) >> 2;
		dev.rate = avg;
		PIT_DRV_SetTimerPeriodByCount(0, 0, dev.rate / dev.div);
		PIT_DRV_SetTimerPeriodByCount(0, 1, dev.rate);
		learn_state = 0;
		dev.mode = BPM_MODE_RUN;
		PIT_DRV_StartTimer(0, 1);
		goto out;
	}
	PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT);
	learn_state++;
out:
	PIT_DRV_StartTimer(0, 0);
}

void bpm_update_div(int div)
{
	PIT_DRV_StopTimer(0, 0);
	PIT_DRV_StopTimer(0, 1);
	dev.div = div;
	PIT_DRV_SetTimerPeriodByCount(0, 0, dev.rate / dev.div);
	PIT_DRV_StartTimer(0, 0);
	PIT_DRV_StartTimer(0, 1);
	if (dev.callback) {
		dev.callback();
	}
}

// Define device configuration.
const pit_user_config_t pitInit = {
    .isInterruptEnabled = true, // Enable timer interrupt.
    .periodUs = 2000U             // Set timer period to 2000 us.
};

void bpm_init(void (*callback)(void))
{
	dev.callback = callback;
	dev.mode = BPM_MODE_RUN;
	dev.rate = BPM_DEFAULT_COUNT;
	dev.div = 1;
	PIT_DRV_InitChannel(0, 0, &pitInit);
	PIT_DRV_SetTimerPeriodByCount(0, 0, BPM_DEFAULT_COUNT >> 4);
	PIT_DRV_StartTimer(0, 0);
	PIT_DRV_InitChannel(0, 1, &pitInit);
	buttons_reg_callback(bpm_button_callback, kGpioBPMBTN);
}
