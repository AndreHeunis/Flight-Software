/***************************************************************************//**
 * @file	fsw_comm.h
 * @brief	Flight software Telecommunications interface header file
 *
 * This header file contains all the required definitions and function
 * prototypes through which to setup the CubeComputer's DMA configuration.
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

#ifndef FSW_COMM_H_
#define FSW_COMM_H_

#include "fsw_modes.h"

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

xQueueHandle FSW_COMM_CMDqueue;					///< Telecommunications module command queue

void FSW_COMM_Init( void );						///< Initialize the telecommunications module.

#endif /* FSW_COMM_H_ */
