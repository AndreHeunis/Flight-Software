/***************************************************************************//**
 * @file	fsw_comm.c
 * @brief	FSW Telecommunications source file.
 *
 * This file acts as the module for command and data handling. It contains task handler
 * definitions, initialization functions, macros, etc to do with C&DH processes.
 * @author	Andre Heunis
 * @date	09/05/2013
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

/// Definitions for FSW_COMM_MSV masks.
#define ERROR_INIT		0x01		///< Module initialization error.
#define ERROR_CMDINV	0x02		///< Invalid command received.

/***************************************************************************//**
 * @addtogroup FSW_Library
 * @brief Flight Sofware (<b>FSW</b>) Module Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup C&DH
 * @brief API for the Telecommunications interface module.
 * @{
 ******************************************************************************/

enum commsState{
	waitForId,
	waitForData
} uartState, i2cState;

static uint8_t FSW_COMM_MSV = 0;							///< Health status byte for COMM module.
static uint8_t FSW_COMM_mode = 0;

static xQueueHandle FSW_transceiver_queue;

//ADDED FROM HIL CODE FOR SIMULATION PURPOSES ONLY*************************************************************************************

// Handle required to suspend the task while processing TCMD/TLM
xTaskHandle Poll_UART_Handle;

portTickType xLastWakeTime;		///< Used to delay the poll_UART task

//TODO: Ideally use the processed flag in the CMD structure instead
uint8_t CMD_processed = 1;		// Used to flag when no commands are waiting to be processed

uint8_t TCMD_dataIndex = 0;

uint8_t adcs_H = 0xFF, cdh_H = 0xFF, comm_H = 0xFF, handh_H = 0xFF, payload_H = 0xFF, power_H = 0xFF;

uint8_t orbitMode;
uint8_t controlMode = 0;
uint8_t powerState;
uint8_t adcsError1;
uint8_t adcsError2;

//*************************************************************************************************************************************

static void FSW_COMM_reportHealthStatus( void );			///< Reports the subsystem's mode and MSV
static void FSW_COMM_modeChange( uint8_t newMode );			///< Changes the module's mode and runs any associated procedures

static void FSW_COMM_manager( void *pvParameters );			///< Subsystem manager

#ifdef HIL_sim
static void Poll_UART( void *pvParameters );				///< Continuously polls the UART for TCMDs and TLM requests
static void Process_TLM_TCM( void *pvParameters );			///< Processes any queued TLM requests and TCMDs
static uint8_t process_TLM(uint8_t id, uint8_t *txBuffer);
static void process_TCMD( CDH_CMD_TypeDef ReceivedCMD );
static uint8_t identify_TCMD_len( uint8_t tcmd_id );
#endif

// FUNCTIONS *************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function initializes the FSW's Telecommunications interface module.
 * Runs any initialization of hardware etc.
 ******************************************************************************/

void FSW_COMM_Init( void )
{
	FSW_COMM_CMDqueue = xQueueCreate( CMD_Qlen, sizeof( CDH_CMD_TypeDef ) );
	FSW_transceiver_queue = xQueueCreate( CMD_Qlen, sizeof( CDH_CMD_TypeDef ) );	///< Holds command read from the transceiver ( the UART for now )

	if( FSW_COMM_CMDqueue == NULL )
	{
		// If the queue could not be created, set the reinit flag for the module.
		FSW_COMM_MSV &= ERROR_INIT;
	}
	else
	{
		xTaskCreate( FSW_COMM_manager, "COMMmanager", 240, NULL, 1, NULL );

#ifdef HIL_sim
		xTaskCreate( Poll_UART, "PollUART", 240, NULL, 1, &Poll_UART_Handle );			///< Polls the transceiver for commands received from the GS ( the UART for now )
		xTaskCreate( Process_TLM_TCM, "ProcessTLMTCM", 240, NULL, 1, NULL );				///< Processes commands queued by Poll_UART
#endif

		FSW_COMM_MSV = 0;
		FSW_COMM_mode = 1;
	}
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function reports the health status of the telecommunications interface
 * module to the health and housekeeping module by sending the health status to
 * the housekeeping module's data queue.
 ******************************************************************************/

static void FSW_COMM_reportHealthStatus( void )
{
	addToBuffer_uint8 ( &(uartTxBuffer[0]), FSW_COMM_mode );
	addToBuffer_uint8 ( &(uartTxBuffer[1]), FSW_COMM_MSV );

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

static void FSW_COMM_modeChange( uint8_t newMode )
{
	FSW_COMM_mode = newMode;

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


#ifdef HIL_sim

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * Processes a telecommand
 ******************************************************************************/
static uint8_t identify_TCMD_len( uint8_t tcmd_id )
{
	uint8_t tempLen;

	switch( tcmd_id )
	{
	case 0x01:
		tempLen = 0;
		break;

	case 0x11:													// Set satellite mode
		tempLen = 1;
		break;

	case 0x12:
		tempLen = 0;
		break;

	case 0x13:	// Set OBC date and time
		tempLen = 0;
		break;

	default:

		break;
	}

	return tempLen;
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * Processes a telecommand
 ******************************************************************************/
static void process_TCMD( CDH_CMD_TypeDef ReceivedCMD )
{
	switch( ReceivedCMD.id )
	{
	/*case 0x01:	// Initiate transfer
	case 0x02:	// Print mode transition in MATLAB
	case 0x03:
	case 0x04:
		addToBuffer_uint8 ( &(uartTxBuffer[0]), ReceivedCMD.id );
		BSP_UART_txBuffer( BSP_UART_DEBUG, uartTxBuffer, 1, false );
		break;

	case 0x11:	// Set mode
		ReceivedCMD.id = 0x02;	//id

		xQueueSendToBack( FSW_CDH_CMDqueue, &ReceivedCMD, 0 );

		break;
	 */
	/*
	case 0x12:	// Retrieve health of all subsystems
		xQueueSendToBack( FSW_CDH_CMDqueue, &sysHealthCMD, 0 );
		break;
		*/
/*
	case 0x13:	// Set OBC date and time
		xQueueSendToBack( FSW_CDH_CMDqueue, &setTimeCMD, 0 );
		break;
*/
	default:
		while(1);
		break;
	}
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * Places the requested telemetry on the transmit buffer and sets the length
 * for transmission.
 ******************************************************************************/

static uint8_t process_TLM(uint8_t id, uint8_t *txBuffer)
{
	uint8_t tlmLen;


	// TODO: Want to do away with this function so implement trsnfr rqst ack and tcmd ack for simulation elsewhere
	switch(id)
	{
	case 0x80:	// Transfer request acknowledge. No new data

		break;

	case 0x82: // telecommand acknowledge

		addToBuffer_uint8(&(txBuffer[0]), 1);						// id
		addToBuffer_uint8(&(txBuffer[1]), CMD_processed);			// processed
		addToBuffer_uint8(&(txBuffer[2]), 0);						// error

		// clear error flags
		//tcmdBuffer[tcmdWriteIndex].error = 0;

		tlmLen = 3;

		break;
/*
	case 0x90: // adcs states. ADDED FROM HIL CODE FOR SIMULATION PURPOSES ONLY

		//addToBuffer_uint8(&(txBuffer[0]), estimationMode);
		addToBuffer_uint8(&(txBuffer[0]), current_state);

		tlmLen = 1;

		break;
*/
		/*
	case 0x91:	// health indicators for each subsystem

		addToBuffer_uint8 ( &(txBuffer[0]), adcs_H );
		addToBuffer_uint8 ( &(txBuffer[1]), cdh_H );
		addToBuffer_uint8 ( &(txBuffer[2]), comm_H );
		addToBuffer_uint8 ( &(txBuffer[3]), handh_H );
		addToBuffer_uint8 ( &(txBuffer[4]), payload_H );
		addToBuffer_uint8 ( &(txBuffer[5]), power_H );

		tlmLen = 6;

		break;
		*/
/*
	case 0x92:	// Report OBC time and date
		addToBuffer_uint32 ( uartTxBuffer, (uint32_t)OBC_time );
		//addToBuffer_uint32 ( uartTxBuffer, (uint32_t)getOBC_time );
		//addToBuffer_uint32 ( &(txBuffer[0]), (uint32_t)1383222393 );
		tlmLen = 4;

		break;
*/
	default:
		// error: unknown telemetry id
		while(1);
		break;
	}
	if( BSP_UART_txInProgress() )
	{
		commsErr = COMMS_ERROR_UARTTLM;
		// TODO: sort out the occurence of this error
		while(1);
	}
	else
	{
		if( id != 0x80 )
		{
			BSP_UART_txBuffer(BSP_UART_DEBUG, uartTxBuffer, tlmLen, true);
		}
	}

	return tlmLen;
}

#endif

// TASKS ****************************************************************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * Subsystem manager for telecommunications interface module. Processes and
 * executes commands on the comms command queue.
 ******************************************************************************/

static void FSW_COMM_manager( void *pvParameters )
{
	portBASE_TYPE Status;
	CDH_CMD_TypeDef ReceivedCMD;

	while(1)
	{
		Status = xQueueReceive( FSW_COMM_CMDqueue, &ReceivedCMD, portMAX_DELAY );

		if(Status == pdPASS)
		{
			switch( ReceivedCMD.id )
			{
			case 0x01:							// Report health status
				FSW_COMM_reportHealthStatus();
				break;

			case 0x02:							// Change the modules mode and run any associated procedures
				FSW_COMM_modeChange( (uint8_t)ReceivedCMD.params[0] );
				break;
#ifdef HIL_sim
			case 0x03:							// Initiate transfer command to Matlab
				addToBuffer_uint8 ( &(uartTxBuffer[0]), ReceivedCMD.params[0] );
				BSP_UART_txBuffer( BSP_UART_DEBUG, uartTxBuffer, 1, false );
				break;

			case 0x04:							// Return the requested telemetry to matlab simulation
				process_TLM ( ReceivedCMD.params[0], uartTxBuffer );
				break;
#endif
			default:
				FSW_COMM_MSV &= ERROR_CMDINV;
				break;
			}
		}
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}

#ifdef HIL_sim

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function fulfills the roll done by the UART interrupt handler in the
 * cubecomputer BSP. The UART is polled periodically and when data is detected,
 * the ID of the data is used to place a structure on a queue for processing by
 * the Process_TLM_TCM task.
 ******************************************************************************/

static void Poll_UART( void *pvParameters )
{
	//xLastWakeTime = xTaskGetTickCount();

	while(1)
	{
		if( current_state == SAFE_MODE || current_state == LINK_MODE )
		{
			// Periodically send request for any data that needs to be sent from the simulation
			if( CMD_processed == 1 )
			{
				sim_Data.params[0] = 0x01;
				xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
			}
		}

		//vTaskDelayUntil( &xLastWakeTime, 1000/portTICK_RATE_MS );
		vTaskDelay(  1000/portTICK_RATE_MS  );
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}

static void Process_TLM_TCM( void *pvParameters )
{
	int id = 0;
	CDH_CMD_TypeDef tempCMD;			// Used to store the received data once it is cast to a CMD struct
	unsigned char CMD_rxBuf [64];		// Used to temporarily store incoming data

	// TODO: This loop needs to be removed. Not good to continuously spin through and check two flags
	while(1)
	{
		// If new data appears on the UART bus
		if( ( BSP_UART_DEBUG->STATUS & USART_STATUS_RXDATAV ) )
		{
			id = BSP_UART_DEBUG->RXDATA;

			if( id == 0x82 )												// 0x82 is id for TCMD acknowledge sent to MATLAB
			{
				BSP_UART_txBuffer(BSP_UART_DEBUG, uartTxBuffer, process_TLM ( id, uartTxBuffer ), true);
			}
			else if( id == 0x01 || id == 0x02 )											// 0x01 is id for incoming CMD struct (TCMD or TLM)
			{
				// Prevent polling for further data
				tempCMD.processed = 0;
				CMD_processed = 0;

				while( TCMD_dataIndex < 14)	// Receive the entire length of the TCMD
				{
					if( (BSP_UART_DEBUG->STATUS & USART_STATUS_RXDATAV) )
					{
						CMD_rxBuf [TCMD_dataIndex] = BSP_UART_DEBUG->RXDATA;

						TCMD_dataIndex++;
					}
				}

				tempCMD = *((CDH_CMD_TypeDef*)CMD_rxBuf);	// Cast received bytes to a CMD structure

				// Telemetry request received
				if( id == 0x02 )
				{
					xQueueSendToBack( FSW_CDH_CMDqueue, &tempCMD, 0 );

					tempCMD.processed = 1;
					CMD_processed = 1;

				}
				// Telecommand received
				else if ( id == 0x01 )
				{
					xQueueSendToBack( FSW_CDH_CMDqueue, &tempCMD, 0 );

					tempCMD.processed = 1;
					CMD_processed = 1;
				}
			}

			TCMD_dataIndex = 0;	// reset counter to CMD byte buffer
			id = 0;				// reset id after whole communication completed
		}
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}


#endif

