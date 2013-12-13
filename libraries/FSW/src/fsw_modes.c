/***************************************************************************//**
 * @file	fsw_modes.c
 * @brief	FSW ADCS source file
 *
 * This header file contains the interface to the state machine that controls
 * the overall mode of the satellite. The overall mode of the satellite has
 * little influence on it's operation and is currently implemented primarily
 * to assist in returning a recovering module to the correct state.
 * @author	Andre Heunis
 * @date	13/09/2013
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

#include "fsw_modes.h"

/***************************************************************************//**
 * @addtogroup FSW_Library
 * @brief Flight Software (<b>FSW</b>) Module Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup MODES
 * @brief API for the modes module.
 * @{
 ******************************************************************************/

#define CMD_Qlen	6									///< Length for this modules command queue

/// Definitions for FSW_MODES_MSV masks
#define ERROR_INIT 		0x01		///< Module initialization error.
#define ERROR_CMDINV 	0x02		///< Invalid command received.

/*************************STATE MACHINE DEFINITIONS***********************************************************************************/
/* Provide the function prototypes for each action procedure. In a real
program, you might have a separate source file for the action procedures of
each state. Then you could create a .h file for each of the source files,
and put the function prototypes for the source file in the .h file. Instead
of listing the prototypes here, you would just #include the .h files. */

/*
void Detumble_proc (void);
void Detumble_cmdCHANGE (void);
void Safe_proc (void);
void Safe_cmdCHANGE (void);
void Nominal_proc (void);
void Nominal_cmdCHANGE (void);
void Link_proc (void);
void Link_cmdCHANGE (void);
void ERP_proc (void);
void ERP_cmdCHANGE (void);
 */
void empty ( void );				// Used if an event must be ignored in a particular state
void MODE_safe( void );			// Used to transfer to safe mode
void MODE_nominal( void );		// Used to transfer to nominal mode
static void MODE_link( void );			// Used to transfer to downlink mode
static void MODE_erp( void );			// Used to transfer to ERP mode
//enum events get_new_event (void);

/* Define the state/event lookup table. The state/event order must be the
same as the enum definitions. Also, the arrays must be completely filled -
don't leave out any events/states. If a particular event should be ignored in
a particular state, just call a "do-nothing" function. */

void (*const state_table [MAX_STATES][MAX_EVENTS]) (void) = {

		{ MODE_safe, 	empty,  		empty,		MODE_erp		}, 		// Event actions for detumbling
		{ empty, 		MODE_nominal, 	empty, 		MODE_erp		},		// Event actions for safe
		{ MODE_safe,	empty, 			MODE_link,	MODE_erp 		},		// Event actions for nominal
		{ MODE_safe,	MODE_nominal, 	empty,		MODE_erp	 	},		// Event actions for link
		{ MODE_safe,	empty, 			empty, 		empty			}		// Event actions for erp
};
/*************************************************************************************************************************************/

static uint8_t FSW_MODES_MSV = 0;							///< Health status byte for ADCS module.
static uint8_t FSW_MODES_mode = 1;

static void FSW_MODES_reportHealthStatus( void );

static void FSW_MODES_manager( void *pvParameters );		///< Processes and executes commands on the ADCS command queue
static void FSW_MODES_SatModeMan( void *pvParameters );		///<


// FUNCTIONS *************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function initializes the FSW's modes module. The satellite is
 * initialized into a detumbling mode.
 ******************************************************************************/
void FSW_MODES_Init( void )
{
	current_state = DETUMBLING_MODE;

	FSW_MODES_CMDqueue = xQueueCreate( CMD_Qlen, sizeof( CDH_CMD_TypeDef ) );

	if( FSW_MODES_CMDqueue == NULL )
	{
		// If the queue could not be created, set the reinit flag for the module.
		FSW_MODES_MSV &= ERROR_INIT;
	}
	else
	{
		xTaskCreate( FSW_MODES_manager, "MODESmanager", 240, NULL, 1, NULL );
		xTaskCreate( FSW_MODES_SatModeMan, "HANDH_SatModeMan", 240, NULL, 1, NULL );

		FSW_MODES_MSV = 0;
		FSW_MODES_mode = 1;
	}
}

/*
// Satellite is sufficiently detumbled. Transfer to safe mode
void Detumble_proc ( void )
{
	current_state = SAFE_MODE;

	// Send message to MatLab to print the mode change
	sim_Data.params[0] = 0x02;
	xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
}

// No TCMDS during detumbling
void Detumble_cmdCHANGE ( void )
{
	// Do nothing
}

// Satellite finished detumbling, rebooting, or exiting recovery from a major error
void Safe_proc ( void )
{
	current_state = NOMINAL_MODE;

	// Send message to MatLab to print the mode change
	sim_Data.params[0] = 0x03;
	xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
}

// User command to enter nominal mode
void Safe_cmdCHANGE ( void )
{
	current_state = NOMINAL_MODE;

	// Send message to MatLab to print the mode change
	sim_Data.params[0] = 0x03;
	xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
}

// Satellite reaches time when it should orientate for communication
void Nominal_proc ( void )
{
	current_state = LINK_MODE;

	// Send message to MatLab to print the mode change
	sim_Data.params[0] = 0x04;
	xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
}

// No user commands can be received during nominal mode (no downlink)
void Nominal_cmdCHANGE ( void )
{
	// Do nothing
}

// Communication section of orbit completed
void Link_proc ( void )
{
	current_state = NOMINAL_MODE;

	// Send message to MatLab to print the mode change
	sim_Data.params[0] = 0x03;
	xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
}

// TCMD to end communication orientation
void Link_cmdCHANGE ( void )
{
	current_state = NOMINAL_MODE;
}

// ERP completed and error removed
void ERP_proc ( void )
{
	current_state = NOMINAL_MODE;
}

// TCMD to end ERP
void ERP_cmdCHANGE ( void )
{
	// Do nothing
}
 */

void empty ( void )
{
	//Do nothing
}

void MODE_safe( void )
{
	current_state = SAFE_MODE;

	// Send message to MatLab to print the mode change
	sim_Data.params[0] = 0x02;
	xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
}

void MODE_nominal( void )
{
	current_state = NOMINAL_MODE;

	// Send message to MatLab to print the mode change
	sim_Data.params[0] = 0x03;
	xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
}

static void MODE_link( void )
{
	current_state = LINK_MODE;

	// Send message to MatLab to print the mode change
	sim_Data.params[0] = 0x04;
	xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
}

static void MODE_erp( void )
{
	current_state = ERP_MODE;

	// Send message to MatLab to print the mode change
	sim_Data.params[0] = 0x05;
	xQueueSendToBack( FSW_CDH_CMDqueue, &sim_Data, 0 );
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function reports the health status of the Modes module to the health and
 * housekeeping module by sending the health status to the housekeeping module's
 * data queue.
 ******************************************************************************/

static void FSW_MODES_reportHealthStatus( void )
{
	portBASE_TYPE Status;
	CDH_DATA_TypeDef HealthStatus;
	HealthStatus.source		= FSW_MODES;
	HealthStatus.data 		= FSW_MODES_MSV;
	HealthStatus.datatype 	= FSW_HEALTHSTATUS;

	xQueueSendToBack( FSW_HANDH_DATAqueue, &HealthStatus, 0 );
}


// TASKS *****************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * Subsystem manager for ADCS interface module. Processes and executes commands
 * on the ADCS command queue.
 ******************************************************************************/

static void FSW_MODES_manager( void *pvParameters )
{
	portBASE_TYPE Status;
	CDH_CMD_TypeDef ReceivedCMD;

	while(1)
	{
		Status = xQueueReceive( FSW_MODES_CMDqueue, &ReceivedCMD, portMAX_DELAY );

		if(Status == pdPASS)
		{
			switch(ReceivedCMD.id)
			{
			case 0x01:															// Report module health
				FSW_MODES_reportHealthStatus();
				break;

			case 0x03:															// Process a state event
				state_table [current_state][ReceivedCMD.params[0]] (); 			/* call the action procedure */
				break;

			default:
				FSW_MODES_MSV &= ERROR_CMDINV;
				break;
			}
		}
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   29/10/2013
 *
 * Task to manage mode actions and mode transitions for the entire satellite.
 ******************************************************************************/

static void FSW_MODES_SatModeMan( void *pvParameters )
{
	CDH_CMD_TypeDef MODE_change;
	MODE_change.dest = FSW_MODES;
	MODE_change.id = 0x03;

	while(1)
	{
		switch( current_state )
		{
		case DETUMBLING_MODE:
			// check if all subsystems running correctly
			// check if satellite is stable enough for nominal operation
			// check power levels are high enough to turn on remaining systems
			// If above conditions are valid, progress to nominal operation

			vTaskDelay( 1000/portTICK_RATE_MS );

			// Progress to safe mode after 1 second
			MODE_change.params[0] = 0;
			xQueueSendToBack( FSW_MODES_CMDqueue, &MODE_change, 0 );
			break;

		case SAFE_MODE:
			// Poll transceiver board for received commands
			// Process any initialization commands or mode change to nominal mode
			// Change ADCS algorithm to nominal (sun pointing)
			// Progress to Nominal

			vTaskDelay( 1000/portTICK_RATE_MS );

			break;

		case  NOMINAL_MODE:
			// Accept any TCMDs
			// Monitor subsystem status
			// Enter ERP if subsystem status shows errors
			// At a certain position (SGP4), enter uplink mode (first check battery power levels)

			vTaskDelay( 5000/portTICK_RATE_MS );

			// Progress to Link mode after 5 seconds
			MODE_change.params[0] = 2;
			xQueueSendToBack( FSW_MODES_CMDqueue, &MODE_change, 0 );
			break;

		case LINK_MODE:
			// High priority task running to accept data uplinked from the GS
			// Enter ERP if subsystem status shows errors
			// At a certain position (SGP4) or due to TCMD to end link, transfer to Nominal

			vTaskDelay( 1000/portTICK_RATE_MS );

			break;

		case ERP_MODE:
			// Run particular ERP for a detected error
			// Reject all non-ERP commands (remember to log that a command was missed)
			// Enter safe mode once procedure is complete


			break;

		default:

			break;
		}
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}
