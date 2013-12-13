/***************************************************************************//**
 * @file	fsw_cdh.h
 * @brief	Flight software Command and Data Handling header file
 *
 * This file contains all the function and structure definitions used in
 * command processing on the OBC
 * @author	Andre Heunis
 * @date	2013/09/05
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

#ifndef FSW_CDH_H_
#define FSW_CDH_H_

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


#include "comms.h"				// for printing
#include "fsw_modes.h"			// for mode/event definitions
#include "fsw_filesystem.h"		// for log typedef

/***************************************************************************//**
 * @addtogroup FSW_Library
 * @brief Flight Sofware (<b>BSP</b>) Module Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup C&DH
 * @brief API for the Command and Data Handling module.
 * @{
 ******************************************************************************/

/// Definitions for Destinations/Sources in command/data structures
#define FSW_ADCS 	1
#define FSW_CDH		2
#define FSW_COMM 	3
#define FSW_FS      4
#define FSW_HANDH 	5
#define FSW_MODES  	6
#define FSW_PAYLOAD 7
#define FSW_POWER 	8

/// Definitions for module modes
#define FSW_MODE_OFF 	0
#define FSW_MODE_ON 	1
#define FSW_MODE_SAFE 	2
#define FSW_MODE_ERP 	3

/// Definitions for data types in data structures
#define FSW_HEALTHSTATUS	1

#define CDH_CMD_PARAMLEN 	1			///< Maximum number of parameters to be included in a single command.
#define CDH_DIARY_CMDCOUNT	4			///< Maximum number of commands held in a single diary structure.

xQueueHandle FSW_CDH_CMDqueue;			///< Main management level command queue
xQueueHandle FSW_CDH_DIARYqueue;		///< Main management level Diary queue.

/// Structure used for all commands in the FSW.
typedef struct{
	uint32_t params[CDH_CMD_PARAMLEN];	///< Parameters needed for command execution. made long for epoch time
	uint32_t exe_time;					///< Time the command is scheduled for.
	uint8_t id;							///< I.D. of the specific command.
	uint8_t dest;						///< Destination module for the command.
	uint8_t len;						///< number of parameters
	uint8_t error;						///< Error flag
	uint8_t processed;					///< Processed flag
	uint8_t resched_cnt;				///< Number of times the CMD needs to be periodically executed
} CDH_CMD_TypeDef;

/// Structure used for a diary entry containing multiple commands.
typedef struct{
	CDH_CMD_TypeDef CMDlist[CDH_DIARY_CMDCOUNT];
}CDH_Diary_TypeDef;

/// Structure used for passing data between tasks
typedef struct{
	uint8_t data;						///< The data being passed
	uint8_t datatype;					///< What the data represents
	uint8_t source;						///< The module where the data originates
}CDH_DATA_TypeDef;

/*COMMANDS USED FOR TESTING. INITIALIZED IN testCMD_initialize( void );*********************************************************/
CDH_CMD_TypeDef sim_Data;
/*******************************************************************************************************************************/

void FSW_CDH_Init( void );					///< Initialize the C&DH module.

#endif /* FSW_CDH_H_ */
