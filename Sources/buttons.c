#include "fsl_pit_driver.h"
#include "fsl_gpio_driver.h"
#include "board.h"
#include "buttons.h"

struct button_cb_descr {
	void (*cb)(void);
	int button;
};
struct button_cb_descr callbacks[NUM_BTN_CB] = {{ 0 }};
struct button_cb_descr *debounce;
void (*defer_callback)(void) = NULL;

// Debounce timer
void PIT2_IRQHandler(void)
{
	int i;
	if (PIT_DRV_IsIntPending(0, 2)) {
		PIT_DRV_StopTimer(0, 2);
		PIT_DRV_ClearIntFlag(0, 2);
		if (!GPIO_DRV_ReadPinInput(debounce->button)) {
			defer_callback = debounce->cb;
		}
		for (i = 0; i < NUM_BTN_CB; i++) {
			GPIO_DRV_ClearPinIntFlag(debounce->button);
		}
	}
}

void buttons_reg_callback(void (*callback)(void), int button, enum button_cb_type type)
{
	if (type >= NUM_BTN_CB) {
		return;
	}
	callbacks[type] = (struct button_cb_descr){
			.cb = callback,
			.button = button,
	};
}

void buttons_do_deferred(void)
{
	if (defer_callback) {
		defer_callback();
		defer_callback = NULL;
	}
}

void PORTA_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending(kGpioSigL)) {
		debounce = &callbacks[BUTTON_CB_SIGL];
	} else if (GPIO_DRV_IsPinIntPending(kGpioSigR)) {
		debounce = &callbacks[BUTTON_CB_SIGR];
	}
	PIT_DRV_StartTimer(0, 2);
}

void PORTB_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending(kGpioSig)) {
		debounce = &callbacks[BUTTON_CB_SIG];
	}
	PIT_DRV_StartTimer(0, 2);
}

void PORTC_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending(kGpioBPMBTN)) {
		debounce = &callbacks[BUTTON_CB_BPM];
	}
	PIT_DRV_StartTimer(0, 2);
}

void PORTD_IRQHandler(void)
{
	GPIO_DRV_TogglePinOutput(kGpioLED2);
	if (GPIO_DRV_IsPinIntPending(kGpioBTN1)) {
		debounce = &callbacks[BUTTON_CB_BTN1];
	} else if (GPIO_DRV_IsPinIntPending(kGpioBTN2)) {
		debounce = &callbacks[BUTTON_CB_BTN2];
	}
	PIT_DRV_StartTimer(0, 2);
}

// Define device configuration.
const pit_user_config_t pit2Init = {
    .isInterruptEnabled = true, // Enable timer interrupt.
    .periodUs = 150000U             // Set timer period to 100000us (100ms).
};

void buttons_init(void) {
	PIT_DRV_InitChannel(0, 2, &pit2Init);
}
