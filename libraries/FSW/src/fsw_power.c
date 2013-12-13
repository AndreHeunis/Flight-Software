/***************************************************************************//**
 * @file	fsw_power.c
 * @brief	FSW Power source file
 *
 * This header file contains the interface to the EPS software/hardware
 * @author	Andre Heunis
 * @date	27/08/2013
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2021 ESL , http://http://www.esl.sun.ac.za/</b>
 *******************************************************************************
 *
 * This source code is the property of the ESL. The source and compiled code may
 * only be used on the CubeComputer.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: ESL has no obligation to
 * support this Software. ESL is providing the Software "AS IS", with no express
 * or implied warranties of any kind, including, but not limited to, any implied
 * warranties of merchantability or fitness for any particular purpose or
 * warranties against infringement of any proprietary rights of a third party.
 *
 * ESL will not be liable for any consequential, incidental, or special damages,
 * or any other relief, or for any claim by any third party, arising from your
 * use of this Software.
 *
 ******************************************************************************/

// for printing
#include "comms.h"

#define CMD_Qlen	6

/// Definitions for FSW_PAYLOAD_MSV masks
#define ERROR_INIT 		0x01		///< Module initialization error.
#define ERROR_CMDINV 	0x02		///< Invalid command received.

/***************************************************************************//**
 * @addtogroup FSW_Library
 * @brief Flight Software (<b>FSW</b>) Module Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup C&DH
 * @brief API for the EPS interface module.
 * @{
 ******************************************************************************/

static uint8_t FSW_POWER_MSV = 0;					///< Health status byte for Power module.
static uint8_t FSW_POWER_mode = 0;

static void FSW_POWER_reportHealthStatus( void );		///< Reports the subsystem's mode and MSV
static void FSW_POWER_modeChange( uint8_t newMode );	///< Changes the module's mode and runs associated procedures
static void FSW_POWER_checkSC( void );					///< Test functions for receiving commands.
static void FSW_POWER_readVI( void );

static void FSW_POWER_manager( void *pvParameters );	///< Subsystem manager for the power module

// FUNCTIONS *************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function initializes the EPS interface's command queue and subsystem
 * manager.
 ******************************************************************************/

void FSW_POWER_Init( void )
{
	FSW_POWER_CMDqueue = xQueueCreate( CMD_Qlen, sizeof( CDH_CMD_TypeDef ) );

	// Periodically sends subsystem commands to the command queue.
	if( FSW_POWER_CMDqueue == NULL )
	{
		FSW_POWER_MSV &= ERROR_INIT;
	}
	else
	{
		xTaskCreate( FSW_POWER_manager, "POWERmanager", 240, NULL, 1, NULL );

		FSW_POWER_MSV = 0;
		FSW_POWER_mode = 1;
	}
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function reports the health status of the EPS interface module to
 * the health and housekeeping module by sending the health status to the
 * housekeeping module's data queue.
 ******************************************************************************/

static void FSW_POWER_reportHealthStatus( void )
{
	addToBuffer_uint8 ( &(uartTxBuffer[0]), FSW_POWER_mode );
	addToBuffer_uint8 ( &(uartTxBuffer[1]), FSW_POWER_MSV );

	// Transmit the 2 values
	if( !BSP_UART_txInProgress() )
	{
		BSP_UART_txBuffer(BSP_UART_DEBUG, uartTxBuffer, 2, true);
	}
	else
	{
		while(1);
	}
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   8/11/2013
 *
 * This function runs any procedures that might be associated with changing
 * the mode
 ******************************************************************************/

static void FSW_POWER_modeChange( uint8_t newMode )
{
	FSW_POWER_mode = newMode;

	// Run required procedures to complete the mode change
	switch( newMode )
	{
	case FSW_MODE_OFF:

	break;

	case FSW_MODE_ON:

	break;

	case FSW_MODE_SAFE:

	break;

	case FSW_MODE_ERP:

	break;
	}
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This functions below are test functions to test receiving a diary of commands
 ******************************************************************************/

void FSW_POWER_checkSC( void )
{
	printString("Power module: Checking for short circuits\n");
}

static void FSW_POWER_readVI( void )
{
	printString("Power module: Reading power levels\n");
}

// TASKS *****************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * Subsystem manager for EPS interface module. Processes and executes commands
 * on the EPS command queue.
 ******************************************************************************/

void FSW_POWER_manager( void *pvParameters )
{
	portBASE_TYPE Status;
	CDH_CMD_TypeDef ReceivedCMD;

	while(1)
	{
		Status = xQueueReceive( FSW_POWER_CMDqueue, &ReceivedCMD, portMAX_DELAY );

		if(Status == pdPASS)
		{
			switch(ReceivedCMD.id)
			{
			case 0x01:
				FSW_POWER_reportHealthStatus();
				break;

			case 0x02:
				FSW_POWER_modeChange( (uint8_t)ReceivedCMD.params[0] );
				break;

			case 0x03:
				FSW_POWER_checkSC();
				break;

			case 0x04:
				FSW_POWER_readVI();
				break;

			default:
				FSW_POWER_MSV &= ERROR_CMDINV;
				break;
			}
		}
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}
