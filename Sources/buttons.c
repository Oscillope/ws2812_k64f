#include "fsl_pit_driver.h"
#include "fsl_gpio_driver.h"
#include "board.h"
#include "buttons.h"


#define NUM_BTN_CB 20

struct button_cb_descr {
	void (*cb)(void);
	int button;
};
struct button_cb_descr callbacks[NUM_BTN_CB] = {{ 0 }};
void (*defer_callback)(int) = NULL;
int arg = 0;

void *button_find_callback(int button)
{
	int i;
	for(i = 0; i < NUM_BTN_CB; i++) {
		if (callbacks[i].button == button) {
			return callbacks[i].cb;
		}
	}
	return NULL;
}

// Debounce timer
void PIT2_IRQHandler(void)
{
	if (PIT_DRV_IsIntPending(0, 2)) {
		PIT_DRV_ClearIntFlag(0, 2);
		PIT_DRV_StopTimer(0, 2);
		if (!GPIO_DRV_ReadPinInput(kGpioSigL)) {
			defer_callback = button_find_callback(kGpioSigL);
			arg = -1;
		} else if (!GPIO_DRV_ReadPinInput(kGpioSigR)) {
			defer_callback = button_find_callback(kGpioSigR);
			arg = 1;
		}
//		This code disabled until I get another button
//		} else if (!GPIO_DRV_ReadPinInput(kGpioSig)) {
//			callback = ledctl_make_flasher;
//			arg = 0;
//		}
		if (!GPIO_DRV_ReadPinInput(kGpioBTN1)) {
			defer_callback = button_find_callback(kGpioBTN1);
		}
		if (!GPIO_DRV_ReadPinInput(kGpioBTN2)) {
			defer_callback = button_find_callback(kGpioBTN2);
		}
		if (!GPIO_DRV_ReadPinInput(kGpioBPMBTN)) {
			defer_callback = button_find_callback(kGpioBPMBTN);
		}
		GPIO_DRV_ClearPinIntFlag(kGpioSigL);
		GPIO_DRV_ClearPinIntFlag(kGpioSigR);
		GPIO_DRV_ClearPinIntFlag(kGpioBTN1);
		GPIO_DRV_ClearPinIntFlag(kGpioBTN2);
		GPIO_DRV_ClearPinIntFlag(kGpioBPMBTN);
	}
}

void buttons_reg_callback(void (*callback)(void), int button)
{
	static unsigned idx;
	if (idx >= NUM_BTN_CB) {
		return;
	}
	callbacks[idx++] = (struct button_cb_descr){
			.cb = callback,
			.button = button,
	};
}

void buttons_do_deferred(void)
{
	if (defer_callback) {
		defer_callback(arg);
		defer_callback = NULL;
	}
}

void PORTA_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending(kGpioSigL)) {
		GPIO_SW_DELAY;
	} else if (GPIO_DRV_IsPinIntPending(kGpioSigR)) {
		GPIO_SW_DELAY;
	}
	PIT_DRV_StartTimer(0, 2);
}

void PORTB_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending(kGpioSig)) {
		GPIO_SW_DELAY;
		GPIO_DRV_ClearPinIntFlag(kGpioSig);
	}
	PIT_DRV_StartTimer(0, 2);
}

void PORTC_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending(kGpioBPMBTN)) {
		GPIO_SW_DELAY;
		GPIO_DRV_ClearPinIntFlag(kGpioBPMBTN);
	}
	PIT_DRV_StartTimer(0, 2);
}

void PORTD_IRQHandler(void)
{
	GPIO_DRV_TogglePinOutput(kGpioLED2);
	if (GPIO_DRV_IsPinIntPending(kGpioBTN1)) {
		GPIO_SW_DELAY;
	} else if (GPIO_DRV_IsPinIntPending(kGpioBTN2)) {
		GPIO_SW_DELAY;
	}
	PIT_DRV_StartTimer(0, 2);
}

// Define device configuration.
const pit_user_config_t pit2Init = {
    .isInterruptEnabled = true, // Enable timer interrupt.
    .periodUs = 100000U             // Set timer period to 100000us (100ms).
};

void buttons_init(void) {
	PIT_DRV_InitChannel(0, 2, &pit2Init);
}
