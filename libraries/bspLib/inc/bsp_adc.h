/***************************************************************************//**
 * @file	bsp_adc.h
 * @brief	BSP ADC header file.
 *
 * This header file contains all the required definitions and function
 * prototypes through which to control the CubeComputer's ADC module.
 * @author	Pieter J. Botma
 * @date	15/05/2012
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

#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "em_adc.h"

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup ADC
 * @brief API for CubeComputer's ADC module.
 * @{
 ******************************************************************************/

#define BSP_ADC_temp2Float(temp_u)		(((float)temp_u)/256.0)

/// Available ADC channels.
typedef enum
{
	CHANNEL0 = 0, 	///< Sampled voltage on ADC channel 0.
	CHANNEL1, 		///< Sampled voltage on ADC channel 1.
	CHANNEL2, 		///< Sampled voltage on ADC channel 2.
	CHANNEL3, 		///< Sampled voltage on ADC channel 3.
	TEMPERATURE,  	///< On-chip temperature value in Celsius.
	CHANNELCOUNT		///< Count of ADC channels used
} ADC_Channel_TypeDef;

void      BSP_ADC_Init(void); ///< Initialise the ADC module.
void      BSP_ADC_update(uint8_t wait); ///< Update all ADC values.
uint8_t   BSP_ADC_isUpdateComplete(void); ///< Returns the progress of a update.
uint16_t  BSP_ADC_getData(ADC_Channel_TypeDef channel); ///< Returns the ADC data buffer.
uint16_t* BSP_ADC_getDataBuff(void); ///< Returns the value of a specified ADC channel.

/** @} (end addtogroup ADC) */
/** @} (end addtogroup BSP_Library) */

#endif // __BSP_ADC_H
