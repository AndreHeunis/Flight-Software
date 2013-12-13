/***************************************************************************//**
 * @file
 * @brief   CubeComputer Test Program Main.
 * @author  Pieter Botma
 * @version 2.0
 ******************************************************************************/
#include "includes.h"
#include "z_HILcomm.h"		// For HIL testing

#define VERBOSE 1
uint8_t debugTLM, wdgEnable;
volatile uint32_t msTicks; /* Counts 1ms timeTicks */

xSemaphoreHandle printingMutex;		///< Mutex for the printing function. A more useful mutex would be assigned to the UART itself

void print_heap_space( void *pvParameters );

/***************************************************************************//**
 * @brief
 *   This function is required by the FAT file system in order to provide
 *   timestamps for created files. Since this example does not include a
 *   reliable clock we hardcode a value here.
 *
 *   Refer to drivers/fatfs/doc/en/fattime.html for the format of this DWORD.
 * @return
 *    A DWORD containing the current time and date as a packed data structure.
 ******************************************************************************/
DWORD get_fattime(void)
{
	return (28 << 25) | (2 << 21) | (1 << 16);
}


/***************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 ******************************************************************************/
void Delay(uint32_t dlyTicks)
{
	uint32_t curTicks;

	curTicks = msec;
	while ((msec - curTicks) < dlyTicks);
}


/***************************************************************************//**
 * @brief  Main function
 * Main is called from _program_start, see assembly startup file
 ******************************************************************************/
int main(void)
{
	//SCB->VTOR = 0x8000;

	// Initialize chip
	CHIP_Init();

	// set up general clocks
	CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
	CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);

	CMU_ClockSelectSet(cmuClock_HF,  cmuSelect_HFXO);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);

	CMU_ClockEnable(cmuClock_CORELE, true);

	BSP_DMA_Init();

	BSP_WDG_Init (false, false);
	BSP_RTC_Init();
	//BSP_EBI_Init();
	BSP_ADC_Init();

	// Initializes UART and I2C communications
	COMMS_init();

	// Flight Software************************************************************************************************************************************
	// Primary initialization
	// First initialize the satellite mode so other modules know how to behave after initializing
	FSW_MODES_Init();
	FSW_CDH_Init();
	FSW_POWER_Init();
	FSW_ADCS_Init();
	FSW_HandH_Init();

	// Initialize when satellite enters safe mode (i.e. exits detumbling. This function will only get called from detumbling mode)
	// TODO: Don't initialize here. Initialize at startup but only turn the subsystem ON at this point
	FSW_COMM_Init();
	FSW_PAYLOAD_Init();
	FSW_FS_Init();

#ifndef HIL_sim
	printingMutex = xSemaphoreCreateMutex();
	xTaskCreate( HIL_TransceiverRX, "TaskTest", 240, NULL, 1, NULL );				// Prints the menu for test tasks and accepts user input
#endif

	//xTaskCreate( print_heap_space, "heap", 170, NULL, 1, NULL );

	vTaskStartScheduler();

	//If this is reached, then there was insufficient heap space for the idle task to be created.
	for( ;; );

}

/**********************************************************************************
 * FreeRTOS functions
 *********************************************************************************/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	/* This function will be called if a task overflows its stack, if
	configCHECK_FOR_STACK_OVERFLOW != 0.  It might be that the function
	parameters have been corrupted, depending on the severity of the stack
	overflow.  When this is the case pxCurrentTCB can be inspected in the
	debugger to find the offending task. */
	for( ;; );
}

void vApplicationIdleHook( void )
{
	/* Use the idle task to place the CPU into a low power mode.  Greater power
	saving could be achieved by not including any demo tasks that never block. */
	//prvLowPowerMode1();
}

void print_heap_space( void *pvParameters )
{
	//unsigned portBASE_TYPE uxHighWaterMark;
	uint8_t Str[32], StrLen;

	size_t var = 0;
	while( 1 )
	{
		vTaskDelay(2000/portTICK_RATE_MS);
		/*
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		StrLen = sprintf((char*)Str,"Loop HWM = %d\n", (int)uxHighWaterMark);
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)Str,StrLen,true);
		 */
		var = xPortGetFreeHeapSize();
		StrLen = sprintf((char*)Str,"Remaining heap space = %d\n", (int)var);
		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)Str,StrLen,true);
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}


