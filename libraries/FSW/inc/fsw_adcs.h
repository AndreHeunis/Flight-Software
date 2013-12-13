/***************************************************************************//**
 * @file	fsw_adcs.h
 * @brief	Flight software ADCS header file
 *
 * This header file contains the interface to the ADCS software/hardware
 * @author	Andre Heunis
 * @date	2013/08/27
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

#ifndef FSW_ADCS_H_
#define FSW_ADCS_H_

#include <stdint.h>

#include "fsw_cdh.h"						// for command typedef
#include "fsw_healthandhousekeeping.h"		// for error flag masks
#include "comms.h"							// for debugging printing
#include "fsw_modes.h"						// for mode definitions

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/***************************************************************************//**
 * @addtogroup FSW_Library
 * @brief Flight Sofware (<b>BSP</b>) Module Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup ADCS
 * @brief API for the ADCS module.
 * @{
 ******************************************************************************/

xQueueHandle FSW_ADCS_CMDqueue;			///< ADCS module command queue

void FSW_ADCS_Init( void );				///< Initialize the ADCS module.

#endif /* FSW_ADCS_H_ */
