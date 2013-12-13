/***************************************************************************//**
 * @file	fsw_modes.h
 * @brief	Flight software modes header file
 *
 * This header file contains the interface to configure the flight software
 * to operate in different modes
 * @author	Andre Heunis
 * @date	2013/09/13
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

#ifndef FSW_MODES_H_
#define FSW_MODES_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "fsw_cdh.h"						// for command typedef

/***************************************************************************//**
 * @addtogroup FSW_Library
 * @brief Flight Sofware (<b>BSP</b>) Module Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Modes
 * @brief API for the Modes module.
 * @{
 ******************************************************************************/

//	STATE_1 = DETUMBLING MODE
//  STATE_2 = SAFE_MODE
// 	STATE_3 = NOMINAL OPERATION
//  STATE_4 = DOWNLINK/UPLINK
// 	STATE_5 = ERROR RECOVERY PROCEDURE

// 	EVENT_1 = MODEsafe:		Put the satellite in safe mode
// 	EVENT_2 = MODEnominal: 	Put the satellite in nominal mode
// 	EVENT_3 = MODEerp:		Put the satellite in ERP mode

enum states { DETUMBLING_MODE, SAFE_MODE, NOMINAL_MODE, LINK_MODE, ERP_MODE, MAX_STATES } current_state;
//enum events { procCHANGE, cmdCHANGE, MAX_EVENTS } new_event; //outdated method
enum events { MODEsafe, MODEnominal, MODElink, MODEerp, MAX_EVENTS } new_event;

xQueueHandle FSW_MODES_CMDqueue;			///< Modes module command queue.

void FSW_MODES_Init( void );				///< Initialize the modes module.

#endif /* FSW_MODES_H_ */
