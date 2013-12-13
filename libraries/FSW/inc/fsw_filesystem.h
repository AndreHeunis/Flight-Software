/***************************************************************************//**
 * @file	fsw_filesystem.h
 * @brief	Flight software file system header file
 *
 * This file contains an interface to a file system on Cubeomputer
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
/*
 * fsw_filesystem.h
 *
 *  Created on: 13 Sep 2013
 *      Author: 15731979
 */

#ifndef FSW_FILESYSTEM_H_
#define FSW_FILESYSTEM_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "fsw_cdh.h"		// for command typedef
#include "comms.h"			// for printing

/***************************************************************************//**
 * @addtogroup FSW_Library
 * @brief Flight Sofware (<b>BSP</b>) Module Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Filesystem
 * @brief API for the file system interface module.
 * @{
 ******************************************************************************/
#define BUFSIZE 512						///< BUFSIZE should be between 512 and 1024, depending on available ram on efm32
#define MAX_LOG_SIZE 1000				///< Maximum size of a log file in bytes

#define LOG_CMD 	1					///< Macros for the 'type' field in the logentry structure
#define LOG_WOD 	2
#define LOG_ERROR 	3

typedef struct{
	uint32_t exe_time;					///< execution time of cmd or time of error detected
	uint8_t type;						///< ERROR, CMD, or WOD
	uint8_t source;						///< The source of the log entry
	uint8_t id;							///< Indicator for what error, cmd, or telemetry is being logged
}FS_LogEntry_TypeDef;

xQueueHandle FSW_FS_LOGqueue;			///< FS queue in which pending log entries wait to be logged
xQueueHandle FSW_FS_CMDqueue;			///< FS command queue

FS_LogEntry_TypeDef log_entry;

void FSW_FS_Init( void );

#endif /* FSW_FILESYSTEM_H_ */
