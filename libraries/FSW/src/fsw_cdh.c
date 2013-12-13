/***************************************************************************//**
 * @file	fsw_cdh.c
 * @brief	FSW Command and Data Handling source file.
 *
 * This file acts as the module for command and data handling. It contains task handler
 * definitions, initialization functions, macros, etc to do with C&DH processes.
 * @author	Andre Heunis
 * @date	26/08/2013
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

#include "fsw_cdh.h"

#define CMD_QLEN		6
#define DIARY_QLEN		6

/// Definitions for FSW_CDH_HEALTH masks.
#define ERROR_INIT		0x01		///< Module initialization error.
#define ERROR_CMDINV	0x02		///< Invalid command received.

/***************************************************************************//**
 * @addtogroup FSW_Library
 * @brief Flight Sofware (<b>FSW</b>) Module Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup C&DH
 * @brief API for the Command and Data Handling module.
 * @{
 ******************************************************************************/

static uint8_t FSW_CDH_MSV = 0;								///< Health status byte for C&DH module
static uint8_t FSW_CDH_mode = 0;

// Data used to schedule commands
typedef struct node{
	CDH_CMD_TypeDef CMD_entry;
	struct node *next;
}NODE;

xSemaphoreHandle LinkedListMutex;							///< Prevents simultaneously adding and removing an element from the CMD schedule linked list
NODE *rootNode;												///< The constant first node in the LL
NODE *currentNode;											///< Temporary NODE used to traverse the LL
NODE *prevNode;												///< Tracks the node behind the current node
NODE *tempNode;												///< Used to add a new list element in the middle of the list

// Software timers for command scheduling
xTimerHandle CMDsched_timer;
static void CMDsched_Callback( xTimerHandle xTimer );		///< Callback function to schedule CMD when timer expires

static void scheduleCMD( CDH_CMD_TypeDef CMD );
static void testCMD_initialize( void );						///< Definitions for commands
static void FSW_CDH_reportHealthStatus( void );				///< Reports the subsystem's mode and MSV
static void FSW_CDH_modeChange( uint8_t newMode );			///< Changes the module's mode and runs any associated procedures

static void FSW_CDH_processCMD( void *pvParameters );		///< Processes commands on the management level command queue.
static void FSW_CDH_processDIARY( void *pvParameters );		///< Processes diaries on the management level diary queue.

// FUNCTIONS *************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function initializes the FSW's C&DH module which serves as one of the
 * two management level modules
 ******************************************************************************/
void FSW_CDH_Init( void )
{
	// Create the first node of the linked list to store scheduled commands
	rootNode = (NODE *) malloc( sizeof(NODE) );
	rootNode->next = 0;																// Initially root points to NULL. (no CMDs queued yet)
	rootNode->CMD_entry.id = 0x99;	// FOR DEBUGGING
	currentNode = rootNode;															// Set the pointer used to traverse the list to the first list entry

	FSW_CDH_CMDqueue = xQueueCreate( CMD_QLEN, sizeof( CDH_CMD_TypeDef ) );			// Receives CMD's
	FSW_CDH_DIARYqueue = xQueueCreate( DIARY_QLEN, sizeof( CDH_Diary_TypeDef ) );	// Receives diarys

	if( ( FSW_CDH_DIARYqueue != NULL ) && ( FSW_CDH_CMDqueue != NULL ) )
	{
		xTaskCreate( FSW_CDH_processCMD, "processCMD", 240, NULL, 1, NULL );		// Continuously processes commands waiting on the command queue
		xTaskCreate( FSW_CDH_processDIARY, "processDIARY", 240, NULL, 1, NULL );	// Continuously processes diaries waiting on the diary queue

		FSW_CDH_MSV = 0;
		FSW_CDH_mode = 1;
	}
	else
	{
		// If the queue could not be created, set the reinit flag for the module
		FSW_CDH_MSV &= ERROR_INIT;
	}

	testCMD_initialize();	// Initialize parameters for test commands and test CMD diary
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   14/10/2013
 *
 * This function schedules a command for execution at a future date by adding it
 * to the queue for scheduled commands and setting a timer to read from that
 * queue at the required time. The command is inserted into the queue in
 * chronological order.
 *
 * @param[in]
 * 		The CMD to be scheduled
 ******************************************************************************/
static void scheduleCMD( CDH_CMD_TypeDef CMD )
{
	xSemaphoreTake( LinkedListMutex, portMAX_DELAY );
	{
		if( CMD.resched_cnt == 0 )				// If multiple periodic executions are NOT required...
		{
			// Initialize freeRTOS software timer for scheduling commands
			// How can separate timers have the same handle, id, etc?
			if( getOBC_time() < CMD.exe_time )
			{
				CMDsched_timer =  xTimerCreate
						( "CMDsched_timer",
								( CMD.exe_time - getOBC_time() )*1000/portTICK_RATE_MS,
								pdFALSE,
								( void * ) 1,
								CMDsched_Callback );

				// Store CMD in the linked list for extraction by timer callback function
				currentNode = rootNode;

				// Check if no other commands are pending execution
				if( rootNode->next == NULL )
				{
					rootNode->next = (NODE *)malloc( sizeof(NODE) );	// Create a new node
					currentNode = rootNode->next;
					if( currentNode != NULL )
					{
						currentNode->CMD_entry = CMD;						// Store scheduled CMD
						currentNode->next = NULL;							// Next pointer is NULL
					}
					else
					{
						// out of memory error
					}
				}
				else
				{
					// Increment past root node to prevent comparison to rootNodes exe time of zero
					do{
						prevNode = currentNode;
						currentNode = currentNode->next;
					}while( CMD.exe_time < currentNode->CMD_entry.exe_time );		// Find point in the list to enter new node

					tempNode = (NODE *)malloc( sizeof(NODE) );						// assign new memory for the new CMD

					if( tempNode != NULL )
					{
						prevNode->next = tempNode;
						tempNode->next = currentNode;
						tempNode->CMD_entry = CMD;
					}
					else
					{
						// out of memory error
					}
				}

				// Start timer
				xTimerStart( CMDsched_timer, 0 );
			}
		}
		else											// ...else initialise timer with a reload
		{
			// Initialise timers with autoreload capabilities
		}
	}
	xSemaphoreGive( LinkedListMutex );
}


/***************************************************************************//**
 * @author Andre Heunis
 * @date   10/10/2013
 * This function is called when a timer used to schedule a CMD expires. It
 * assumes that the queue containing all the scheduled commands is in order
 * and that the front item is the next to be executed.
 * @param[in] xTimer
 				Timer from which the callback function was run
 ******************************************************************************/
// TODO: Possibly use mutual exclusion to prevent this function reading from the LL while a new CMD is being added to it
static void CMDsched_Callback( xTimerHandle xTimer )
{
	CDH_CMD_TypeDef CMD;

	xSemaphoreTake( LinkedListMutex, portMAX_DELAY );
	{
		//loop through list to find the end
		currentNode = rootNode;

		while( currentNode->next != NULL )
		{
			prevNode = currentNode;
			currentNode = currentNode->next;
		}

		// Extract the last element of the list and execute it
		CMD = currentNode->CMD_entry;
		CMD.exe_time = 0;										// Resend CMD with an exe_time of 0, resulting in instant execution
		xQueueSendToBack( FSW_CDH_CMDqueue, &CMD, 0 );

		// Free the memory used by the last element
		currentNode = prevNode;									// first set currentNode to previous list entry
		free( prevNode->next );									// free memory pointed to by previous entry ( i.e. the currentNode entry )
		currentNode->next = NULL;								// set currentNode entry next pointer to NULL
	}
	xSemaphoreGive( LinkedListMutex );
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   01/10/2013
 *
 * This function initializes the definitions of the commands used for testing
 * using the terminal program
 ******************************************************************************/
static void testCMD_initialize( void )
{
	// Used to command MATLAB to print out mode changes (fsw_modes.c) as well as poll the UART for new data (fsw_comm.c)
	sim_Data.id = 0x03;
	sim_Data.dest = FSW_COMM;
	sim_Data.len = 0;
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function reports the health status of the CDH module to the health and
 * housekeeping module by sending the health status to the housekeeping module's
 * data queue.
 ******************************************************************************/

static void FSW_CDH_reportHealthStatus( void )
{
	addToBuffer_uint8 ( &(uartTxBuffer[0]), FSW_CDH_mode );
	addToBuffer_uint8 ( &(uartTxBuffer[1]), FSW_CDH_MSV );

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

static void FSW_CDH_modeChange( uint8_t newMode )
{
	FSW_CDH_mode = newMode;

	switch( newMode )
	{
	case FSW_MODE_OFF:
		// Turn off power to all nonessential hardware
		// Disable processing of non-essential commands
		break;

	case FSW_MODE_ON:
		// Turn on all required hardware
		// Start tasks to manage ADCS algorithms
		break;

	case FSW_MODE_SAFE:
		// Similar to turning off
		break;

	case FSW_MODE_ERP:
		// Disable non ERP commands
		// Evaluate MSV to determine if any ERP procedures should be run
		break;
	}
}

// TASKS *****************************************************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function continuously processes any commands on the command queue and
 * sends them to their destination modules. The command is processed in this
 * module to see if it is valid with respect to the current system state.
 * Structural integrity is checked in the destination module.
 ******************************************************************************/

// Continuously processes commands on the command queue by sending them to their destination modules
static void FSW_CDH_processCMD( void *pvParameters )
{
	portBASE_TYPE CMD_Q_Status;
	CDH_CMD_TypeDef ReceivedCMD;

	while(1)
	{
		CMD_Q_Status = xQueueReceive( FSW_CDH_CMDqueue, &ReceivedCMD, portMAX_DELAY );

		if(CMD_Q_Status == pdPASS)
		{
			if( ReceivedCMD.exe_time != 0 )													// If scheduling is required...
			{
				scheduleCMD( ReceivedCMD );
			}
			else																			// CMD processing
			{/*
				if( ReceivedCMD.id != 0x03 && ReceivedCMD.id != FSW_COMM )
				{
					FS_LogEntry_TypeDef cmd_LogEntry;										// Log the command before sending it to the relevant module
					cmd_LogEntry.exe_time = OBC_time;
					cmd_LogEntry.type = LOG_CMD;
					cmd_LogEntry.source = ReceivedCMD.dest;
					cmd_LogEntry.id = ReceivedCMD.id;
					xQueueSendToBack( FSW_FS_LOGqueue, &cmd_LogEntry, 0 );
				}
				*/
				switch(ReceivedCMD.dest)
				{
				case FSW_ADCS:
					xQueueSendToBack( FSW_ADCS_CMDqueue, &ReceivedCMD, 0 );
					break;

				case FSW_CDH:
					if( ReceivedCMD.id == 0x01 )											// Return status telemetry
						FSW_CDH_reportHealthStatus();
					else if( ReceivedCMD.id == 0x02 )										// Change the modules mode and run any associated procedures
						FSW_CDH_modeChange( (uint8_t)ReceivedCMD.params[0] );
					break;

				case FSW_COMM:
					xQueueSendToBack( FSW_COMM_CMDqueue, &ReceivedCMD, 0 );
					break;

					//TODO: First check if a SD card is present. For all modules, first check if subsystem is enabled
					/*case FSW_FS:
					xQueueSendToBack( FSW_FS_CMDqueue, &ReceivedCMD, 0 );
					break;*/

				case FSW_HANDH:
					xQueueSendToBack( FSW_HANDH_CMDqueue, &ReceivedCMD, 0 );
					break;

				case FSW_MODES:
					xQueueSendToBack( FSW_MODES_CMDqueue, &ReceivedCMD, 0 );
					break;

				case FSW_PAYLOAD:
					xQueueSendToBack( FSW_PAYLOAD_CMDqueue, &ReceivedCMD, 0 );
					break;

				case FSW_POWER:
					xQueueSendToBack( FSW_POWER_CMDqueue, &ReceivedCMD, 0 );
					break;

				default:
					FSW_CDH_MSV &= ERROR_CMDINV;
					while(1);
					break;
				}
			}
		}

		// Reset CMD_Q_Status to NULL ?
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}


/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * Continuously processes diaries on the diary queue by sending the commands in
 * each diary to their destination modules
 ******************************************************************************/
//TODO: Find out how sets of CMDs will sent from GS to ZA-Aerosat
static void FSW_CDH_processDIARY( void *pvParameters )
{
	portBASE_TYPE DIARY_Q_Status;
	CDH_Diary_TypeDef ReceivedDIARY;
	int DiaryEntry;

	while(1)
	{
		DIARY_Q_Status = xQueueReceive( FSW_CDH_DIARYqueue, &ReceivedDIARY, portMAX_DELAY );

		if(DIARY_Q_Status == pdPASS)
		{
			// Loop through every command in the diary and dispatch it to the relevant module
			for( DiaryEntry = 0; DiaryEntry < CDH_DIARY_CMDCOUNT; DiaryEntry++ )
			{
				switch(ReceivedDIARY.CMDlist[DiaryEntry].dest)
				{
				case FSW_ADCS:
					xQueueSendToBack( FSW_ADCS_CMDqueue, &ReceivedDIARY.CMDlist[DiaryEntry], 0 );
					break;

				case FSW_POWER:
					xQueueSendToBack( FSW_POWER_CMDqueue, &ReceivedDIARY.CMDlist[DiaryEntry], 0 );
					break;

				case FSW_COMM:
					break;

				case FSW_PAYLOAD:
					break;

				default:
					FSW_CDH_MSV &= ERROR_CMDINV;
					break;
				}
			}
		}

		// Reset DIARY_Q_Status to NULL?
		DIARY_Q_Status = NULL;
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}


