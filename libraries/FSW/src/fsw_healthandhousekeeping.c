/***************************************************************************//**
 * @file	fsw_healthandhousekeeping.c
 * @brief	FSW Health and Housekeeping source file.
 *
 * This file acts as the module for health and housekeeping. It contains
 * functions including retrieving the health of various subsystems, determining if
 * corrective procedures should be run, and running housekeeping procedures
 * such as maintaining the date and time.
 * @author	Andre Heunis
 * @date	02/09/2013
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

#include "fsw_healthandhousekeeping.h"

#define CMD_Qlen	6
#define DATA_Qlen	6

#define ERROR_INIT 		0x01		///< Module initialization error.
#define ERROR_CMDINV 	0x02		///< Invalid command received.
//#define ERROR_<*>		  0x04
//#define ERROR_<*>		  0x08

#define TLMID_V1			0x01
#define TLMID_V2			0x02
#define TLMID_OBCTEMP		0x03

// For sending data of variable length over UART
#define UART_ESCAPECHAR 0x1F
#define UART_SOM        0x7F
#define UART_EOM        0xFF

static xTaskHandle IncrementOBCTime_handle;

static uint8_t FSW_HANDH_MSV = 0;			///< Health status byte for HandH module.
static uint8_t FSW_HANDH_mode = 0;

HANDH_EnviroTLM_Typedef HAND_EnviroTLM;			///< Structure to hold environmental TLM
HANDH_EnviroTLMselection_Typedef HANDH_EnviroTLMselection;	///< Flags to indicate which telemetry to send

// TODO: These should possibly be moved to their corresponding modules
// Health
static void process_ADCShealth( uint8_t healthStatus );
static void process_CDHhealth( uint8_t healthStatus );
static void process_COMMhealth( uint8_t healthStatus );
static void process_FShealth( uint8_t healthStatus );
static void process_HANDHhealth( uint8_t healthStatus );
static void process_MODEShealth( uint8_t healthStatus );
static void process_PAYLOADhealth( uint8_t healthStatus );
static void process_POWERhealth( uint8_t healthStatus );
static void FSW_HANDH_reportHealthStatus( void );				///< Reports the subsystem's mode and MSV.
static void FSW_HANDH_modeChange( uint8_t newMode );			///< Changes the module's mode and runs any associated procedures
static void FSW_HANDH_reportSysHealth( void );

// OBC time
static void setDate_and_Time( time_t epoch_num );
static void printOBCtime( void );
static void IncrementOBCTime( void *pvParameters );

// Satellite and module management
static void FSW_HANDH_CMDmanager( void *pvParameters );			///< Subsystem command manager for the Health and Housekeeping module
static void FSW_HANDH_DATAmanager( void *pvParameters );		///< Subsystem data manager for the Health and Housekeeping module

// Real-time telemetry stream
static void FSW_HANDH_TLMSTREAMmanager( void *pvParameters );

/*ADDED FOR HIL SIMULATION PURPOSES******************************************/
//#ifdef HIL_sim
extern uint8_t adcs_H, cdh_H, comm_H, handh_H, payload_H, power_H;
//#endif


// FUNCTIONS *************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   09/10/2013
 *
 * Getter function for the OBC_time
 * @param[out] OBC_time
 				Current OBC_time
 ******************************************************************************/

time_t getOBC_time( void )
{
	return OBC_time;
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   11/09/2013
 *
 * This function initializes the FSW's HandH interface module. Two queues are
 * initialized to receive commands and data. Two manager tasks are initialized
 * to read from these queues. An additional task is initialized to manage time
 * on the OBC.
 ******************************************************************************/
void FSW_HandH_Init( void )
{
	OBC_time = 0;

	FSW_HANDH_CMDqueue = xQueueCreate( CMD_Qlen, sizeof( CDH_CMD_TypeDef ) );
	FSW_HANDH_DATAqueue = xQueueCreate( DATA_Qlen, sizeof( CDH_CMD_TypeDef ) );

	if( FSW_HANDH_CMDqueue == NULL || FSW_HANDH_DATAqueue == NULL )
	{
		// If the queue could not be created, set the reinit flag for the module.
		FSW_HANDH_MSV &= ERROR_INIT;
	}
	else
	{
		xTaskCreate( IncrementOBCTime, "IncrementOBCTime", 240, NULL, 1, &IncrementOBCTime_handle );
		xTaskCreate( FSW_HANDH_CMDmanager, "HANDH_CMDmanager", 240, NULL, 1, NULL );
		xTaskCreate( FSW_HANDH_DATAmanager, "HANDH_DATAmanager", 240, NULL, 1, NULL );
#ifdef HIL_sim
		xTaskCreate( FSW_HANDH_TLMSTREAMmanager, "FSW_HANDH_TLMSTREAMmanager", 240, NULL, 1, NULL );
#endif

		FSW_HANDH_MSV = 0;
		FSW_HANDH_mode = 1;
	}
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   11/09/2013
 *
 * Set the OBC date and time to the specified UNIX time stamp value
 ******************************************************************************/

static void setDate_and_Time( time_t epoch_num )
{
	// suspend the task incrementing the date and time
	vTaskSuspend( IncrementOBCTime_handle );

	// set the new value for date and time
	OBC_time = epoch_num;

	// resume the task incrementing date and time
	vTaskResume( IncrementOBCTime_handle );
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   11/09/2013
 *
 * Send commands to all subsystems requesting the health status of the subsystem.
 * Processing of the status of each subsystem and calling of required corrective
 * procedures is done in a separate function
 ******************************************************************************/
/*
static void FSW_HANDH_reportSysHealth( void )
{
	portBASE_TYPE sendStatus;

	// TODO: Rethink this function
	// Send commands to all subsystems. Make 0x01 the general return health command
	sendStatus = xQueueSendToBack( FSW_CDH_CMDqueue, &FSW_ADCS_ReportHealth, 0 );
	sendStatus = xQueueSendToBack( FSW_CDH_CMDqueue, &FSW_CDH_ReportHealth, 0 );
	sendStatus = xQueueSendToBack( FSW_CDH_CMDqueue, &FSW_COMM_ReportHealth, 0 );
	FSW_HANDH_reportHealthStatus();
	sendStatus = xQueueSendToBack( FSW_CDH_CMDqueue, &FSW_PAYLOAD_ReportHealth, 0 );
	sendStatus = xQueueSendToBack( FSW_CDH_CMDqueue, &FSW_POWER_ReportHealth, 0 );
}
*/

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * This function reports the health status of the HandH interface module to
 * the health and housekeeping module by sending the health status to the
 * housekeeping module's data queue.
 ******************************************************************************/

static void FSW_HANDH_reportHealthStatus( void )
{
	addToBuffer_uint8 ( &(uartTxBuffer[0]), FSW_HANDH_mode );
	addToBuffer_uint8 ( &(uartTxBuffer[1]), FSW_HANDH_MSV );

	// Transmit the 2 values
	if( !BSP_UART_txInProgress() )
	{
		BSP_UART_txBuffer(BSP_UART_DEBUG, uartTxBuffer, 2, true);
	}
	else
	{
		while(1);
	}
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   8/11/2013
 *
 * This function runs any procedures that might be associated with changing
 * the mode
 ******************************************************************************/

static void FSW_HANDH_modeChange( uint8_t newMode )
{
	FSW_HANDH_mode = newMode;

	// Run required procedures to complete the mode change
	switch( newMode )
	{
	case FSW_MODE_OFF:

	break;

	case FSW_MODE_ON:

	break;

	case FSW_MODE_SAFE:

	break;

	case FSW_MODE_ERP:

	break;
	}
}


/***************************************************************************//**
 * @author Andre Heunis
 * @date   11/09/2013
 *
 * The functions below are called by this subsystem's data manager to process
 * received health status data.
 ******************************************************************************/

static void process_ADCShealth( uint8_t healthStatus )
{
	if( healthStatus == 0 )
		adcs_H = 1;
	//printString("No errors in ADCS subsystem\n");
	else
		adcs_H = 2;
	//printString("ADCS ERROR!!!\n");
}

static void process_CDHhealth( uint8_t healthStatus )
{
	if( healthStatus == 0 )
		cdh_H = 1;
	//printString("No errors in CDH subsystem\n");
	else
		cdh_H = 2;
	//printString("CDH ERROR!!!\n");
}

static void process_COMMhealth( uint8_t healthStatus )
{
	if( healthStatus == 0 )
		comm_H = 1;
	//printString("No errors in communication subsystem\n");
	else
		comm_H = 2;
	//printString("Telecommunication ERROR!!!\n");
}

static void process_FShealth( uint8_t healthStatus )
{
	if( healthStatus == 0 )
		printString("No errors in file system module\n");
	else
		printString("File system module ERROR!!!\n");
}

static void process_HANDHhealth( uint8_t healthStatus )
{
	if( healthStatus == 0 )
		handh_H = 1;
	//printString("No errors in HandH subsystem\n");
	else
		handh_H = 2;
	//printString("HandH error ERROR!!!\n");
}

static void process_MODEShealth( uint8_t healthStatus )
{
	if( healthStatus == 0 )
		printString("No errors in Modes module\n");
	else
		printString("Modes module ERROR!!!\n");
}

static void process_PAYLOADhealth( uint8_t healthStatus )
{
	if( healthStatus == 0 )
		payload_H = 1;
	//printString("No errors in payload interface\n");
	else
		payload_H = 2;
	//printString("Payload interface ERROR!!!\n");
}

static void process_POWERhealth( uint8_t healthStatus )
{
	if( healthStatus == 0 )
		power_H = 1;
	//printString("No errors in EPS subsystem\n");
	else
		power_H = 2;
	//printString("EPS ERROR!!!\n");
}

// TASKS *******************************************************************************************************************************************************************

/***************************************************************************//**
 * @author Andre Heunis
 * @date   11/09/2013
 *
 * Task to update the OBC date and time every second.
 ******************************************************************************/

static void IncrementOBCTime( void *pvParameters )
{
	while(1)
	{
		vTaskDelay(1000/portTICK_RATE_MS);	// more accurate to use delay until?

		OBC_time++;

#ifndef HIL_sim
		printOBCtime();
#endif
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}


/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * Subsystem manager for HandH module. Processes and executes commands
 * on the HandH command queue.
 ******************************************************************************/

static void FSW_HANDH_CMDmanager( void *pvParameters )
{
	CDH_CMD_TypeDef ReceivedCMD;
	portBASE_TYPE Status;

	while(1)
	{
		Status = xQueueReceive( FSW_HANDH_CMDqueue, &ReceivedCMD, portMAX_DELAY );

		if(Status == pdPASS)
		{
			switch(ReceivedCMD.id)
			{
			case 0x01:	// Report the health of this subsystem
				FSW_HANDH_reportHealthStatus();
				break;

			// TODO: In the simulation, 0x02 is for setting the satellite mode
			case 0x02:	// Set OBC date and time
				//HandH_logCMD( ReceivedCMD );
				setDate_and_Time((time_t)(ReceivedCMD.params[0]));
				break;

			case 0x03: // Return requested OBC date and time
				addToBuffer_uint32 ( uartTxBuffer, (uint32_t)OBC_time );

				if( !BSP_UART_txInProgress() )
				{
					BSP_UART_txBuffer(BSP_UART_DEBUG, uartTxBuffer, 4, true);
				}
				else
				{
					while(1);
				}
				break;
				/*
			case 0x04:	// request the health of all subsystems
				FSW_HANDH_reportSysHealth();
				break;
				*/
			default:	// Set the 2nd bit to indicate an unknown CMD was received
				FSW_HANDH_MSV &= ERROR_CMDINV;
				while(1);
				break;
			}
		}
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}


static void FSW_HANDH_DATAmanager( void *pvParameters )
{
	CDH_DATA_TypeDef ReceivedDATA;

	portBASE_TYPE Status;

	while(1)
	{
		Status = xQueueReceive( FSW_HANDH_DATAqueue, &ReceivedDATA, portMAX_DELAY );

		if(Status == pdPASS)
		{
			switch(ReceivedDATA.source)
			{
			case FSW_ADCS:
				if( ReceivedDATA.datatype == FSW_HEALTHSTATUS )
					process_ADCShealth( ReceivedDATA.data );
				break;

			case FSW_CDH:
				if( ReceivedDATA.datatype == FSW_HEALTHSTATUS )
					process_CDHhealth( ReceivedDATA.data );
				break;

			case FSW_COMM:
				if( ReceivedDATA.datatype == FSW_HEALTHSTATUS )
					process_COMMhealth( ReceivedDATA.data );
				break;

			case FSW_HANDH:
				if( ReceivedDATA.datatype == FSW_HEALTHSTATUS )
					process_HANDHhealth( ReceivedDATA.data );
				break;

			case FSW_PAYLOAD:
				if( ReceivedDATA.datatype == FSW_HEALTHSTATUS )
					process_PAYLOADhealth( ReceivedDATA.data );
				break;

			case FSW_POWER:
				if( ReceivedDATA.datatype == FSW_HEALTHSTATUS )
					process_POWERhealth( ReceivedDATA.data );
				break;

			default:
				FSW_HANDH_MSV &= ERROR_CMDINV;
				break;
			}
		}
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}

/***************************************************************************//**
 * @author Andre Heunis
 * @date   05/09/2013
 *
 * Task to handle the real-time TLM stream. This stream transmits the TLM
 * specified by the user at the rate specified by the user.
 ******************************************************************************/

static void FSW_HANDH_TLMSTREAMmanager( void *pvParameters )
{
	CDH_CMD_TypeDef Telemetry;
	uint8_t TLM_buffer[50];						///< TLM buf to send to fsw_comm for transmission
	uint8_t TLM_buffer_index = 0;
	float OBCTEMP = 0;
	unsigned long long_OBCTEMP = 0;

	// Always enabled for testing
	HANDH_EnviroTLMselection.HANDH_V1_flag = 1;
	HANDH_EnviroTLMselection.HANDH_V2_flag = 1;
	HANDH_EnviroTLMselection.HANDH_OBCtemp_flag = 1;

	while(1)
	{
		// TLM is sent every 3 seconds
		vTaskDelay(3000/portTICK_RATE_MS);
		BSP_ADC_update(1);

		// Update all the telemetry fields
		HAND_EnviroTLM.HANDH_V1 = BSP_ADC_getData(CHANNEL0);
		HAND_EnviroTLM.HANDH_V2 = BSP_ADC_getData(CHANNEL1);
		//HAND_EnviroTLM.HANDH_OBCtemp = BSP_ADC_getData(TEMPERATURE);

		// Construct buffer to send
		TLM_buffer[TLM_buffer_index++] = UART_ESCAPECHAR;
		TLM_buffer[TLM_buffer_index++] = UART_SOM;

		if( HANDH_EnviroTLMselection.HANDH_V1_flag )
		{
			TLM_buffer[TLM_buffer_index++] = TLMID_V1;
			TLM_buffer[TLM_buffer_index++] = (BSP_ADC_getData(CHANNEL0)&0x00FF);
			TLM_buffer[TLM_buffer_index++] = (BSP_ADC_getData(CHANNEL0)&0xFF00)>>8;
		}
		if( HANDH_EnviroTLMselection.HANDH_V2_flag )
		{
			TLM_buffer[TLM_buffer_index++] = TLMID_V2;
			TLM_buffer[TLM_buffer_index++] = (BSP_ADC_getData(CHANNEL1)&0x00FF);
			TLM_buffer[TLM_buffer_index++] = (BSP_ADC_getData(CHANNEL1)&0xFF00)>>8;
		}
		if( HANDH_EnviroTLMselection.HANDH_OBCtemp_flag )
		{
			TLM_buffer[TLM_buffer_index++] = TLMID_OBCTEMP;

			// Break the float into 4 bytes
			OBCTEMP = BSP_ADC_temp2Float(BSP_ADC_getData(TEMPERATURE));
			long_OBCTEMP = *(unsigned long*)&OBCTEMP;

			TLM_buffer[TLM_buffer_index++] = (long_OBCTEMP & 0xFF);
			TLM_buffer[TLM_buffer_index++] = (long_OBCTEMP & 0xFF00) >> 8;
			TLM_buffer[TLM_buffer_index++] = (long_OBCTEMP & 0xFF0000) >> 16;
			TLM_buffer[TLM_buffer_index++] = (long_OBCTEMP & 0xFF000000) >> 24;
		}


		TLM_buffer[TLM_buffer_index++] = UART_ESCAPECHAR;
		TLM_buffer[TLM_buffer_index++] = UART_EOM;

		// Send telemetry. Currently just a command with a telemetry value as the parameter.
		// Dont allow a different module (e.g fsw_comms.c) to gather the telemetry. Must be done here
		Telemetry.id = 0x05;
		Telemetry.params[0] = TLM_buffer;
		Telemetry.len = TLM_buffer_index;
		//Telemetry.params[0] = BSP_ADC_getData(CHANNEL1);	// Sample a voltage on the OBC
		//Telemetry.params[0] = BSP_ADC_temp2Float(BSP_ADC_getData(TEMPERATURE));				// requires different conversions to byte arrays

/*
		addToBuffer_uint16(&(txBuffer[0]),BSP_ADC_getData(CHANNEL0));
		addToBuffer_uint16(&(txBuffer[2]),BSP_ADC_getData(CHANNEL1));
		addToBuffer_uint16(&(txBuffer[4]),BSP_ADC_getData(CHANNEL2));
		addToBuffer_uint16(&(txBuffer[6]),BSP_ADC_getData(CHANNEL3));
		addToBuffer_uint16(&(txBuffer[8]),BSP_ADC_getData(TEMPERATURE));*/
		//tlmLen = 10;

		xQueueSendToBack( FSW_COMM_CMDqueue, &Telemetry, 0 );
		TLM_buffer_index = 0;
	}

	// Delete the task if it ever breaks out of the loop above
	vTaskDelete( NULL );
}
// TEST FUNCTIONS *********************************************************************************************************************

// Prints the current OBC date and time
static void printOBCtime( void )
{
	char buf[21];

	time_t now = OBC_time;

	struct tm ts;

	// Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
	ts = *gmtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S\n", &ts);

	BSP_UART_txBuffer( BSP_UART_DEBUG, (uint8_t*)buf, sizeof(buf) - 1, true );
}










