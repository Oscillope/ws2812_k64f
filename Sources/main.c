#include "fsl_device_registers.h"
#include "board.h"
#include "pin_mux.h"
#include "fsl_clock_manager.h"
#include "fsl_debug_console.h"
#include "fsl_gpio_driver.h"
#include "fsl_lptmr_driver.h"
#include "ledctl.h"
#include "bpm.h"
#include <stdio.h>

#define BUFFER_LENGTH NUM_LEDS

GPIO_Type *portc_base;
uint32_t data_pin;
rgb buffer[BUFFER_LENGTH] = {{ 0 }};

void output1(void) {
	int time = 0;
	//GPIO_DRV_WritePinOutput(kGpioData, 1);
	GPIO_WR_PSOR(portc_base, 1U << data_pin);
	for(time = 8; time > 0; time--);
	//GPIO_DRV_WritePinOutput(kGpioData, 0);
	GPIO_WR_PCOR(portc_base, 1U << data_pin);
	for(time = 2; time > 0; time--);
}

void output0(void) {
	int time = 0;
	//GPIO_DRV_WritePinOutput(kGpioData, 1);
	GPIO_WR_PSOR(portc_base, 1U << data_pin);
	for(time = 3; time > 0; time--);
	//GPIO_DRV_WritePinOutput(kGpioData, 0);
	GPIO_WR_PCOR(portc_base, 1U << data_pin);
	for(time = 4; time > 0; time--);
}

void LPTMR0_IRQHandler(void)
{
	int len = BUFFER_LENGTH;
	while(len) {
		int mask = 0x800000;
		while(mask) {
			int grb = buffer[len-1].g | (buffer[len-1].r << 8) | (buffer[len-1].b << 16);
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
    LPTMR_DRV_SetTimerPeriodUs(LPTMR0_IDX, 100);
    // Start LPTMR
    LPTMR_DRV_Start(LPTMR0_IDX);

	printf("\nHello World! \r\n");

	ledctl_init(buffer);
	bpm_init(ledctl_update);

	while(1) {
		// Everything is using timers now.
	}
	return 0;
}


/*EOF*/
