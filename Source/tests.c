/***************************************************************************//**
 * @file
 * @brief	CubeComputer Test Program Test Implementations.
 *
 * This file contains the implementations for functions written to test the
 * functionality of all the peripherals and subsystems of CubeComputer.
 *
 * @author	Pieter J. Botma
 * @date	04/04/2013
 *
 ******************************************************************************/

#include "tests.h"

/******************************* RTC ***************************************//**
 *
 * @author Pieter J. Botma
 * @date   04/04/2013
 *
 * This function outputs the seconds counter over the debug UART.
 *
 ******************************************************************************/
void TEST_RTC (void)
{
	if(VERBOSE)
		debugLen = sprintf((char*)debugStr,"\n\nTime: %d.%03d", (int)sec, (int)msec);
	else
		debugLen = sprintf((char*)debugStr,"%d.%03d;", (int)sec, (int)msec);

	BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
}


/******************************* EBI ***************************************//**
 *
 * @author Pieter J. Botma
 * @date   04/04/2013
 *
 * This functions test the EBI by writing and reading to the different external
 * memory modules and then displaying the results over the debug UART.
 *
 ******************************************************************************/
void TEST_EBI(void)
{
	uint8_t status;

	bool error = false;
	uint32_t i;

	/* Global variables */
	uint8_t test[TEST_ARRAY_SIZE]   = { 0x7B, 0x1E, 0x2E, 0x9F, 0xE9, 0x7E, 0x73, 0x17,
	                                     0xAE, 0x8A, 0x1E, 0xAC, 0x9E, 0x6F, 0x45, 0x8E,
	                                     0x30, 0x1C, 0xA3, 0xE4, 0xE5, 0xC1, 0x1A, 0x52,
	                                     0xF6, 0x24, 0xDF, 0x9B, 0xAD, 0x41, 0xE6, 0x37 };

	uint8_t answer[TEST_ARRAY_SIZE] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	                                     0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	                                     0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	                                     0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

	//*** EEPROM ***//

	// Write data in the External EEPROM
	BSP_EBI_progEEPROM(0,test,TEST_ARRAY_SIZE);

	// Read external EEPROM
	for (i=0 ; i<TEST_ARRAY_SIZE; i++)
	{
		answer[i] = *(uint8_t*)(BSP_EBI_EEPROM_BASE + i);
	}

	// Test the difference between buffers.
	for (i = 0; i < TEST_ARRAY_SIZE; i++)
	{
		if (test[i] != answer[i])
		{
			error = true;
		}
	}

	if (error)
	{
		// Write and Read operation FAILED
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\n\nEEPROM test unsuccessful!");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}
	else
	{
		// Write and Read operation SUCCESS
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\n\nEEPROM test successful!");
		else
			debugLen = sprintf((char*)debugStr,"1;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}


	//*** Flash test ***//

	error = false;

	status = lld_SectorEraseOp((uint8_t*)BSP_EBI_FLASH_BASE, 0);

	// Write external FLASH
	for (i=0 ; i<TEST_ARRAY_SIZE ; i++)
	{
		status = lld_ProgramOp((uint8_t*)(BSP_EBI_FLASH_BASE), i, test[i]);
	}

	// Read external FLASH
	for (i=0 ; i<TEST_ARRAY_SIZE; i++)
	{
		answer[i] = *(uint8_t*)(BSP_EBI_FLASH_BASE + i);
	}

	// Test the difference between buffers.
	for (i = 0; i < TEST_ARRAY_SIZE; i++)
	{
		if (test[i] != answer[i])
		{
			error = true;
		}
	}

	if (error)
	{
		// Write and Read operation  FAILED
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\nFLASH test unsuccessful!");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}
	else
	{
		// Write and Read operation  SUCCESS
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\nFLASH test successful!");
		else
			debugLen = sprintf((char*)debugStr,"1;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}


	//*** SRAM Test ***//

	error = false;

	// Write external SRAM
	for (i=0 ; i<TEST_ARRAY_SIZE ; i++)
	{
		*(uint8_t*)(BSP_EBI_SRAM1_BASE + i) = test[i];
	}

	// Read external SRAM
	for (i=0 ; i<TEST_ARRAY_SIZE; i++)
	{
		answer[i] = *(uint8_t*)(BSP_EBI_SRAM1_BASE + i);
	}


	// Test the difference between buffers.
	for (i = 0; i < TEST_ARRAY_SIZE; i++)
	{
		if (test[i] != answer[i])
		{
		  error = true;
		}
	}

	if (error)
	{
		// Write and Read operation  FAILED
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\nSRAM test unsuccessful!");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}
	else
	{
		// Write and Read operation  SUCCESS
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\nSRAM test successful!");
		else
			debugLen = sprintf((char*)debugStr,"1;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}

}


/******************************* I2C ***************************************//**
 *
 * @author Pieter J. Botma
 * @date   04/04/2013
 *
 * This function tests the master I2C functionality by writing and reading to an
 * external I2C module (ST M24C64) connected to the main system I2C channel.
 *
 ******************************************************************************/

uint8_t txBuffer[ 5] = {0x00,0x00,0xA1,0xA2,0xA3}; // write commands = address1, addres2, data1, data2, data3
uint8_t txIndex;

uint8_t rxBuffer[32]; // read data
uint8_t rxIndex;

void TEST_I2C(void)
{
  // I2C Master Test (Used with ST M24C64 EEPROM)

  debugLen = sprintf((char*)debugStr,"I2C Test (External EEPROM write and read):\n");
  BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);

  BSP_I2C_masterTX(BSP_I2C_SYS, 0xA0, bspI2cWrite, txBuffer, 5, rxBuffer, 0);
  Delay(100);
  BSP_I2C_masterTX(BSP_I2C_SYS, 0xA0, bspI2cWriteRead, txBuffer, 2, rxBuffer, 3);

  // Display results read from EEPROM (should display 0xA1, 0xA2, 0xA3)

  BSP_UART_txByte(BSP_UART_DEBUG,rxBuffer[0]);
  BSP_UART_txByte(BSP_UART_DEBUG,rxBuffer[1]);
  BSP_UART_txByte(BSP_UART_DEBUG,rxBuffer[2]);

  while(1);
}


/******************************* ADC ***************************************//**
 *
 * @author Pieter J. Botma
 * @date   04/04/2013
 *
 * This function samples, via ADC, and displays the OBC current and voltage
 * values over the debug UART.
 *
 ******************************************************************************/
void TEST_ADC (void)
{
	  BSP_ADC_update(1);

	  if (VERBOSE)
	  {
		  debugLen = sprintf((char*)debugStr,"\n\nChannel 0 (mV): %d",BSP_ADC_getData(CHANNEL0));
		  BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
		  debugLen = sprintf((char*)debugStr,"\nChannel 1 (mV): %d",BSP_ADC_getData(CHANNEL1));
		  BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
		  debugLen = sprintf((char*)debugStr,"\nChannel 2 (mV): %d",BSP_ADC_getData(CHANNEL2));
		  BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
		  debugLen = sprintf((char*)debugStr,"\nChannel 3 (mV): %d",BSP_ADC_getData(CHANNEL3));
		  BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
		  debugLen = sprintf((char*)debugStr,"\nCelcius (C): %.2f",BSP_ADC_temp2Float(BSP_ADC_getData(TEMPERATURE)));
		  BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	  }
	  else
	  {
		  debugLen = sprintf((char*)debugStr,"%d;%d;%d;%d;%d;",BSP_ADC_getData(CHANNEL0),
				  BSP_ADC_getData(CHANNEL1),  BSP_ADC_getData(CHANNEL2),
				  BSP_ADC_getData(CHANNEL3),  BSP_ADC_getData(TEMPERATURE) );
		  BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	  }
}

/***************************** MICROSD *************************************//**
*
* @author Pieter J. Botma
* @date   04/04/2013
*
* This functions tests the microSD functionality by detecting the SD card,
* initialising the file system, writing a test buffer and reading it again.
*
* @note
* 	SD card must be formated as FAT32.
* @note
*   Refer to Energy Micro application note AN0030.
*
*******************************************************************************/
void TEST_microSD(void)
{
	FIL fsrc;							// File objects
	FATFS Fatfs;						// File system specific
	FRESULT res;						// FatFs function common result code
	UINT br, bw;						// File read/write count
	DSTATUS resCard;					// SDcard status
	int8_t ramBufferWrite[BUFFERSIZE];	// Temporary buffer for write file
	int8_t ramBufferRead[BUFFERSIZE];	// Temporary buffer for read file
	int8_t StringBuffer[] = "CubeComputer MicroSD test was successful...";

	int16_t i;
	int16_t filecounter;

	// Step1
	// Initialization file buffer write
	filecounter = sizeof(StringBuffer);

	for(i = 0; i < filecounter ; i++)
	{
	   ramBufferWrite[i] = StringBuffer[i];
	}

	// Step2
	// Detect micro-SD
	while(1)
	{
	  MICROSD_Init();                     // Initialize MicroSD driver

	  resCard = disk_initialize(0);       // Check micro-SD card status

	  switch(resCard)
	  {
	  case STA_NOINIT:                    // Drive not initialized
		break;
	  case STA_NODISK:                    // No medium in the drive
		break;
	  case STA_PROTECT:                   // Write protected
		break;
	  default:
		break;
	  }

	  if (!resCard) break;                // Drive initialized.

	  Delay(1);
	}

	//Step3
	// Initialize filesystem
	if (f_mount(0, &Fatfs) != FR_OK)
	{
		// Error.No micro-SD with FAT32 is present
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 1");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}

	// Step4
	// Open  the file for write
	res = f_open(&fsrc, TEST_FILENAME,  FA_WRITE);
	if (res != FR_OK)
	{
		// If file does not exist create it
		res = f_open(&fsrc, TEST_FILENAME, FA_CREATE_ALWAYS | FA_WRITE );
		if (res != FR_OK)
		{
			// Error. Cannot create the file
			if(VERBOSE)
				debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 2");
			else
				debugLen = sprintf((char*)debugStr,"0;");

			BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
		}
	}

	// Step5
	// Set the file write pointer to first location
	res = f_lseek(&fsrc, 0);
	if (res != FR_OK)
	{
		// Error. Cannot set the file write pointer
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 3");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}

	// Step6
	// Write a buffer to file
	res = f_write(&fsrc, ramBufferWrite, filecounter, &bw);
	if ((res != FR_OK) || (filecounter != bw))
	{
		// Error. Cannot write the file
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 4");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}

	// Step7
	// Close the file
	f_close(&fsrc);
	if (res != FR_OK)
	{
		// Error. Cannot close the file
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 5");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}

	// Step8
	// Open the file for read
	res = f_open(&fsrc, TEST_FILENAME,  FA_READ);
	if (res != FR_OK)
	{
		// Error. Cannot create the file
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 6");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}

	// Step9
	// Set the file read pointer to first location
	res = f_lseek(&fsrc, 0);
	if (res != FR_OK)
	{
		// Error. Cannot set the file pointer
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 7");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}

	// Step10
	// Read some data from file
	res = f_read(&fsrc, ramBufferRead, filecounter, &br);
	 if ((res != FR_OK) || (filecounter != br))
	{
	  // Error. Cannot read the file
	  //while(1);
		 if(VERBOSE)
			 debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 8");
		 else
			 debugLen = sprintf((char*)debugStr,"0;");

		 BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}

	// Step11
	// Close the file
	f_close(&fsrc);
	if (res != FR_OK)
	{
		// Error. Cannot close the file
		//while(1);
		if(VERBOSE)
			debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 9");
		else
			debugLen = sprintf((char*)debugStr,"0;");

		BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	}

	// Step12
	// Compare ramBufferWrite and ramBufferRead
	for(i = 0; i < filecounter ; i++)
	{
	  if ((ramBufferWrite[i]) != (ramBufferRead[i]))
	  {
		  // Error compare buffers
		  //while(1);
		  if(VERBOSE)
			  debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test Unsuccessful: Error 10");
		  else
			  debugLen = sprintf((char*)debugStr,"0;");

		  BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
	  }
	}

	// Step13
	// If this step is reached, MSD test was successful
	if(VERBOSE)
		debugLen = sprintf((char*)debugStr,"\n\nMicroSD Test successful!");
	else
		debugLen = sprintf((char*)debugStr,"1;\n");

	BSP_UART_txBuffer(BSP_UART_DEBUG,(uint8_t*)debugStr,debugLen,true);
}
