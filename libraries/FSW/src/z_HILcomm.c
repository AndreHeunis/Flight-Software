/*
 * HILL_comm.c
 * This is a temporary file included in the project to test the flight software using the HIL program
 *  Created on: 18 Sep 2013
 *      Author: 15731979
 */
#include "z_HILcomm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "bsp_uart.h"
#include "comms.h"

uint8_t hillReady = 0;

void HIL_TransceiverRX( void *pvParameters )
{
	printString("Flight Software Alpha Terminal Test Application\n");
	printString("MENU:\n't': Print out RTC value\n'm': MicroSD test\n");
	printString("c/w/e: Test log entries\n");
	printString("'r': Reset MCU\n'a': Print test string\n");

	while(1)
	{
		COMMS_processTCMD();

		vTaskDelay( 500/portTICK_RATE_MS );
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}
