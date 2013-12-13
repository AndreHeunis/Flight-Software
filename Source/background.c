/*
 * background.c
 *
 *  Created on: 11 Jul 2013
 *      Author: pjbotma
 */

#include "includes.h"

volatile uint32_t  sec = 0; ///< total time in seconds
volatile uint16_t msec = 0; ///< total time in mseconds

volatile uint32_t singleErrors = 0;
volatile uint32_t doubleErrors = 0;
volatile uint32_t multiErrors  = 0;

/**************************************************************************//**
 * @brief RTC_IRQHandler
 * Interrupt Service Routine for real time clock
 *****************************************************************************/
void RTC_IRQHandler(void)
{
	// clear interrupt source
	RTC_IntClear(RTC_IFC_COMP0);

	// update milliseconds timer
	msec++;

	if(msec >= 1024)
	{
		// update seconds timer
		sec++;

		// reset milliseconds timer
		msec = 0;
	}

	// toggle external wdog
	// BSP_WDG_ToggleExt();
}


/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter.
 *****************************************************************************/
/*
void SysTick_Handler(void)
{
	// update milliseconds timer
	// msec++;
}
*/

/**************************************************************************//**
 * @brief RTC_IRQHandler
 * Interrupt Service Routine for analog comparators ACMP0 and ACMP1
 *****************************************************************************/
void ACMP0_IRQHandler(void)
{
	if(ACMP0->IF & ACMP_IF_EDGE)
	{
		BSP_EBI_disableSRAM (bspEbiSram1);

		ACMP0->IFC = ACMP_IFC_EDGE;
	}

	if(ACMP1->IF & ACMP_IF_EDGE)
	{
		BSP_EBI_disableSRAM (bspEbiSram2);

		ACMP1->IFC = ACMP_IFC_EDGE;
	}
}

/**************************************************************************//**
 * @brief RTC_IRQHandler
 * Interrupt Service Routine for general purpose input/output pins
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
	uint8_t errors;

	errors = ( (uint8_t)GPIO_PortInGet(gpioPortB) ) & 0x5;

	switch(errors)
	{
	case 0x0:
		multiErrors++;
		break;

	case 0x1:
		doubleErrors++;
		break;

	case 0x4:
		singleErrors++;
		break;
	}
}
