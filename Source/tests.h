/***************************************************************************//**
 * @file
 * @brief	CubeComputer Test Program Test Definitions.
 *
 * This file contains the definitions for functions written to test the
 * functionality of all the peripherals and subsystems of CubeComputer.
 *
 * @author	Pieter J. Botma
 * @date	27/02/2013
 *
 ******************************************************************************/

#ifndef __TEST_H
#define __TEST_H

#include "includes.h"

#define VERBOSE 1

//*** MSD Defines ***//

#define BUFFERSIZE    512			// BUFFERSIZE should be between 512 and 1024, depending on available ram on efm32
#define TEST_FILENAME "test.txt" 	// Filename to open/write/read from SD-card

//*******************//

//*** SRAM Defines ***//

#define TEST_ARRAY_SIZE 32

//********************//

void Delay(uint32_t dlyTicks);

void TEST_RTC(void);
void TEST_EBI(void);
void TEST_I2C(void);
void TEST_ADC(void);
void TEST_microSD(void);

#endif
