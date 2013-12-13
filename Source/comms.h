/***************************************************************************//**
 * @file
 * @brief	CubeComputer Comms Definitions.
 *
 * This file contains the definitions and functions to react to specified
 * telemetry and telcommand.
 *
 * @author	Pieter J. Botma
 * @date	02/04/2013
 *
 ******************************************************************************/

#ifndef __COMMS_H
#define __COMMS_H

#include "includes.h"
#include "z_CMDdefs.h"
// cdh.h to include definitions for simulation commands

//#define HIL_sim		///< Include this macro to configure the FSW for testing with the HIL simulation in MATLAB
						///< ( functions in fsw_comm.c, disable UART interrupt)
						///< Exclude this macro for testing with terminal
						///< ( functions in comms.c, enable UART interrupt)

#define COMMS_I2C_TYPE  0x01
#define COMMS_I2C_READ  0x01
#define COMMS_I2C_WRITE 0x00

#define COMMS_ID_TYPE 0x80
#define COMMS_ID_TLM  0x80
#define COMMS_ID_TCMD 0x00

#define COMMS_ERROR_TCMDBUFOF 	1
#define COMMS_ERROR_UARTTLM		2
#define COMMS_ERROR_I2CTLM  	3

#define COMMS_TCMDERR_PARAMUF 	1
#define COMMS_TCMDERR_PARAMOF 	2

#define COMMS_TCMD_BUFFLEN   	4
#define COMMS_TCMD_PARAMLEN  	8

uint16_t commsErr;

extern uint8_t debugStr[64], debugLen;

uint8_t uartTxBuffer[64];	// moved here by AH

void addToBuffer_uint8 (uint8_t *buffer, uint8_t data);
void addToBuffer_uint32 (uint8_t *buffer, uint32_t data);
void COMMS_init(void);
void COMMS_processTCMD(void);
void printString(const char * format);

#endif
