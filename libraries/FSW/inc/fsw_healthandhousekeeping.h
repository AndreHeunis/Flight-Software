/***************************************************************************//**
 * @file	fsw_healthandhousekeeping.h
 * @brief	Flight software Health and Housekeeping header file
 *
 * This is the API for the Health and Housekeeping module.
 * @author	Andre Heunis
 * @date	2013/09/02
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

#ifndef FSW_HEALTHANDHOUSEKEEPING_H_
#define FSW_HEALTHANDHOUSEKEEPING_H_

#include <time.h>			// used in converting unix epoch times
#include <stdint.h>			// for receiving data
#include "FreeRTOS.h"		// for creating tasks and queues
#include "task.h"
#include "queue.h"
#include "bsp_uart.h"		// for printing OBC time
#include "fsw_cdh.h"		// for processing commands
#include "fsw_payload.h"	// for payload CMD queue
#include "fsw_cdh.h"		// for defining commands

/****************************************************
 * OBC Environmental Telemetry selection structure
 *
 * Flags to indicate which telemetry must be streamed
 ****************************************************/
typedef struct{
	uint8_t HANDH_V1_flag;
	uint8_t HANDH_V2_flag;
	uint8_t HANDH_OBCtemp_flag;
}HANDH_EnviroTLMselection_Typedef;

/****************************************************
 * OBC Environmental Telemetry structure
 *
 * Holds two voltages and the OBC temperature
 ****************************************************/
typedef struct{
	uint8_t HANDH_V1;
	uint8_t HANDH_V2;
	uint8_t HANDH_OBCtemp;
}HANDH_EnviroTLM_Typedef;

xQueueHandle FSW_HANDH_CMDqueue;		///< Health and Housekeeping module command queue
xQueueHandle FSW_HANDH_DATAqueue;		///< Health and Housekeeping module command queue

time_t OBC_time;						///< Date and time for the OBC

void FSW_HandH_Init( void );
time_t getOBC_time( void );				///< Getter function for OBC_time

#endif /* FSW_HEALTHANDHOUSEKEEPING_H_ */
