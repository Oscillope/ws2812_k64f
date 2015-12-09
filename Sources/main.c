#include "fsl_device_registers.h"
#include "board.h"
#include "pin_mux.h"
#include "fsl_clock_manager.h"
#include "fsl_debug_console.h"
#include "fsl_gpio_driver.h"
#include "fsl_lptmr_driver.h"
#include "fsl_pit_driver.h"
#include "ledctl.h"
#include "bpm.h"
#include <stdio.h>

#define BUFFER_LENGTH NUM_LEDS

GPIO_Type *portc_base;
uint32_t data_pin;
rgb buffer[BUFFER_LENGTH] = {{ 0 }};
void (*callback)(int) = NULL;
int arg = 0;

void output1(void) {
	int time = 0;
	//GPIO_DRV_WritePinOutput(kGpioData, 1);
	GPIO_WR_PSOR(portc_base, 1U << data_pin);
	for (time = 8; time > 0; time--);
	//GPIO_DRV_WritePinOutput(kGpioData, 0);
	GPIO_WR_PCOR(portc_base, 1U << data_pin);
	for (time = 2; time > 0; time--);
}

void output0(void) {
	int time = 0;
	//GPIO_DRV_WritePinOutput(kGpioData, 1);
	GPIO_WR_PSOR(portc_base, 1U << data_pin);
	for (time = 3; time > 0; time--);
	//GPIO_DRV_WritePinOutput(kGpioData, 0);
	GPIO_WR_PCOR(portc_base, 1U << data_pin);
	for (time = 4; time > 0; time--);
}

void LPTMR0_IRQHandler(void)
{
	int len = BUFFER_LENGTH;
	while(len) {
		int mask = 0x800000;
		while (mask) {
			int grb = buffer[len-1].b | (buffer[len-1].r << 8) | (buffer[len-1].g << 16);
			//if (grb)
			//	printf("\ngrb value: %08x\r\n", grb);
			(grb & mask) ? output1() : output0();
			mask >>= 1;
		}
		len--;
	}
	GPIO_DRV_TogglePinOutput(kGpioLED3);
	LPTMR_DRV_IRQHandler(LPTMR0_IDX);	// Reset timer
}

// Debounce timer
void PIT2_IRQHandler(void)
{
	static int col;
	static int state;
	if (PIT_DRV_IsIntPending(0, 2)) {
		PIT_DRV_ClearIntFlag(0, 2);
		PIT_DRV_StopTimer(0, 2);
		if (!GPIO_DRV_ReadPinInput(kGpioSigL)) {
			callback = ledctl_make_flasher;
			arg = -1;
		} else if (!GPIO_DRV_ReadPinInput(kGpioSigR)) {
			callback = ledctl_make_flasher;
			arg = 1;
		} else if (!GPIO_DRV_ReadPinInput(kGpioSig)) {
			callback = ledctl_make_flasher;
			arg = 0;
		}
		if (!GPIO_DRV_ReadPinInput(kGpioBTN1)) {
			if (state < NUM_RAINBOW_STATES) state++;
			else state = 0;
			callback = ledctl_make_swoosh;
			arg = state;
		}
		if (!GPIO_DRV_ReadPinInput(kGpioBTN2)) {
			if (col < NUM_COLORS) col++;
			else col = 0;
			callback = ledctl_make_cylon;
			arg = col;
		}
	}
}

void PORTA_IRQHandler(void)
{
	if (GPIO_DRV_IsPinIntPending(kGpioSigL)) {
		GPIO_SW_DELAY;
		GPIO_DRV_ClearPinIntFlag(kGpioSigL);
	} else if (GPIO_DRV_IsPinIntPending(kGpioSigR)) {
		GPIO_SW_DELAY;
		GPIO_DRV_ClearPinIntFlag(kGpioSigR);
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

void PORTD_IRQHandler(void)
{
	GPIO_DRV_TogglePinOutput(kGpioLED2);
	if (GPIO_DRV_IsPinIntPending(kGpioBTN1)) {
		GPIO_SW_DELAY;
		GPIO_DRV_ClearPinIntFlag(kGpioBTN1);
	} else if (GPIO_DRV_IsPinIntPending(kGpioBTN2)) {
		GPIO_SW_DELAY;
		GPIO_DRV_ClearPinIntFlag(kGpioBTN2);
	}
	PIT_DRV_StartTimer(0, 2);
}

// Define device configuration.
const pit_user_config_t pit2Init = {
    .isInterruptEnabled = true, // Enable timer interrupt.
    .periodUs = 100000U             // Set timer period to 100000us (100ms).
};

int main(void)
{
	portc_base = g_gpioBase[GPIO_EXTRACT_PORT(kGpioData)];
	data_pin = GPIO_EXTRACT_PIN(kGpioData);
	/* enable clock for PORTs */
	CLOCK_SYS_EnablePortClock(PORTA_IDX);
	CLOCK_SYS_EnablePortClock(PORTB_IDX);
	CLOCK_SYS_EnablePortClock(PORTC_IDX);
	CLOCK_SYS_EnablePortClock(PORTE_IDX);

	// LPTMR configurations
	lptmr_user_config_t lptmrConfig =
	{
		.timerMode = kLptmrTimerModeTimeCounter,
		.freeRunningEnable = false,
		.prescalerEnable = true,
		.prescalerClockSource = kClockLptmrSrcLpoClk,
		.prescalerValue = kLptmrPrescalerDivide2,
		.isInterruptEnabled = true,
	};
	// LPTMR driver state information
	lptmr_state_t lptmr0State;

	/* Init board clock */
	BOARD_ClockInit();
	dbg_uart_init();

	// Initializes GPIO driver
	GPIO_DRV_Init(switchPins, outPins);

    // Initialize LPTMR
    LPTMR_DRV_Init(LPTMR0_IDX, &lptmr0State, &lptmrConfig);
    // Set timer period for TMR_PERIOD microseconds
    LPTMR_DRV_SetTimerPeriodUs(LPTMR0_IDX, 1000);
    // Start LPTMR
    LPTMR_DRV_Start(LPTMR0_IDX);

	printf("\nHello World! \r\n");

	PIT_DRV_Init(0, false);
	PIT_DRV_InitChannel(0, 2, &pit2Init);

	ledctl_init(buffer);
	bpm_init(ledctl_update);
	ledctl_make_swoosh(0);

	while (1) {
		if (callback) {
			callback(arg);
			callback = NULL;
		}
	}
	return 0;
}


/*EOF*/
