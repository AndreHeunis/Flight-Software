/***************************************************************************//**
 * @file	bsp_ebi.c
 * @brief	BSP EBI source file.
 *
 * This file contains all the implementations for the functions defined in \em
 * bsp_ebi.h.
 * @author	Pieter J. Botma
 * @date	28/05/2012
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

#include "bsp_ebi.h"
#include "em_acmp.h"
#include "em_ebi.h"
#include "em_cmu.h"
#include "em_gpio.h"

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup EBI
 * @brief API for CubeComputer's external bus interface.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   28/05/2012
 *
 * This function initialise the CubeComputer's External Bus Interface (EBI)
 * which allows the use of CubeComputer's external memory modules (EEPROM, Flash
 * and SRAM).
 *
 ******************************************************************************/
void BSP_EBI_Init (void)
{
	// Enable clocks
	CMU_ClockEnable(cmuClock_EBI, true);
	CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure Power to SRAM and IO buffers
	GPIO_PinModeSet(gpioPortC,  14, gpioModePushPull, 0); // nOE on BUFFERS for SRAM1
	GPIO_PinModeSet(gpioPortC,  15, gpioModePushPull, 0); // nOE on BUFFERS for SRAM2
#if defined(CubeCompV2B)
	GPIO_PinModeSet(gpioPortC,  12, gpioModePushPull, 1); // POW_SRAM1
	GPIO_PinModeSet(gpioPortC,  13, gpioModePushPull, 1); // POW_SRAM2
#elif defined(CubeCompV3)
	GPIO_PinModeSet(gpioPortD,   7, gpioModePushPull, 1); // POW_SRAM1
	GPIO_PinModeSet(gpioPortD,   8, gpioModePushPull, 1); // POW_SRAM2
#else
#error Unknown CubeComputer!
#endif

#if defined(CubeCompV3)
	// ECC  Control Pins
	GPIO_PinModeSet(gpioPortE,  2, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE,  3, gpioModePushPull, 0);
	// ECC  Error Pins
	GPIO_PinModeSet(gpioPortB,  0, gpioModeInput, 1);
	GPIO_PinModeSet(gpioPortB,  2, gpioModeInput, 1);
#endif

	// Configure GPIO pins as push pull
	// EBI AD9..15
	GPIO_PinModeSet(gpioPortA,  0, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortA,  1, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortA,  2, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortA,  3, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortA,  4, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortA,  5, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortA,  6, gpioModePushPull, 0);

	// EBI AD8
	GPIO_PinModeSet(gpioPortA, 15, gpioModePushPull, 0);

	// EBI CS0..CS3
	GPIO_PinModeSet(gpioPortD,  9, gpioModePushPull, 1);
	GPIO_PinModeSet(gpioPortD, 10, gpioModePushPull, 1);
	GPIO_PinModeSet(gpioPortD, 11, gpioModePushPull, 1);
	GPIO_PinModeSet(gpioPortD, 12, gpioModePushPull, 1);

	// EBI AD0..7
	GPIO_PinModeSet(gpioPortE,  8, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE,  9, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE, 10, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE, 11, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE, 12, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE, 14, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortE, 15, gpioModePushPull, 0);

	// EBI ALEN/Wen/Ren
	GPIO_PinModeSet(gpioPortF,  3, gpioModePushPull, 0);
	GPIO_PinModeSet(gpioPortF,  4, gpioModePushPull, 1);
	GPIO_PinModeSet(gpioPortF,  5, gpioModePushPull, 1);

	// EBI register initializations

	EBI_Init_TypeDef init = EBI_INIT_DEFAULT;

	init.mode = ebiModeD8A24ALE;

	init.banks   = EBI_BANK0 | EBI_BANK1 | EBI_BANK2 | EBI_BANK3;
	init.csLines = EBI_CS0 | EBI_CS1 | EBI_CS2| EBI_CS3;

	init.alePolarity = ebiActiveHigh;

	// Address Setup and hold time
	init.addrHoldCycles  = 3;
	init.addrSetupCycles = 3;

	// Read cycle times
	init.readStrobeCycles = 10;
	init.readHoldCycles   = 3;
	init.readSetupCycles  = 3;

	// Write cycle times
	init.writeStrobeCycles = 10;
	init.writeHoldCycles   = 3;
	init.writeSetupCycles  = 3;

	EBI_Init(&init);

	// Enable GPIO_ODD & GPIO_EVEN interrupt vector in NVIC
	//NVIC_EnableIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);

	// Configure PE1 interrupt on falling edge
	GPIO_IntConfig(gpioPortB, 0, false, true, true);
	//GPIO_IntConfig(gpioPortB, 1, false, true, true);
	GPIO_IntConfig(gpioPortB, 2, false, true, true);
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   23/05/2012
 *
 * This function sets the power to the specified SRAM module. Used to power-
 * cycle the SRAM module.
 *
 * @param[in] module
 *   SRAM module to be power toggled.
 * @param[in] enable
 *   Switch on supply to SRAM if true, else switch off supply to SRAM.
 *
 ******************************************************************************/
void BSP_EBI_setPower (BSP_EBI_SRAMSelect_TypeDef module, bool enable)
{
	switch (module)
	{
	case bspEbiSram1:
		if(enable)
			GPIO_PinOutSet(BSP_EBI_SRAM_POWPORT,BSP_EBI_SRAM1_POWPIN);   // = 1
		else
			GPIO_PinOutClear(BSP_EBI_SRAM_POWPORT,BSP_EBI_SRAM1_POWPIN); // = 0
		break;
	case bspEbiSram2:
		if(enable)
			GPIO_PinOutSet(BSP_EBI_SRAM_POWPORT,BSP_EBI_SRAM2_POWPIN);   // = 1
		else
			GPIO_PinOutClear(BSP_EBI_SRAM_POWPORT,BSP_EBI_SRAM2_POWPIN); // = 0
		break;
	}
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   23/05/2012
 *
 * This function allows or isolates the specified SRAM module from the EBI.
 *
 * @param[in] module
 *   SRAM module to be IO toggled.
 * @param[in] enable
 *   Allows SRAM access to the EBI if true, else isolates the SRAM from the EBI.
 *
 ******************************************************************************/
void BSP_EBI_setBuffer (BSP_EBI_SRAMSelect_TypeDef module, bool enable)
{
	// Buffer is notEnable, thus clear when true, set when false
	switch (module)
	{
	case bspEbiSram1:
		if(enable)
			GPIO_PinOutClear(BSP_EBI_SRAM_BUFPORT,BSP_EBI_SRAM1_BUFPIN); // = 0
		else
			GPIO_PinOutSet(BSP_EBI_SRAM_BUFPORT,BSP_EBI_SRAM1_BUFPIN);   // = 1
		break;

	case bspEbiSram2:
		if(enable)
			GPIO_PinOutClear(BSP_EBI_SRAM_BUFPORT,BSP_EBI_SRAM2_BUFPIN); // = 0
		else
			GPIO_PinOutSet(BSP_EBI_SRAM_BUFPORT,BSP_EBI_SRAM2_BUFPIN);   // = 1
		break;
	}
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   17/07/2013
 *
 * This function enables the specified SRAM module on the EBI. A 1ms blanket
 * period is given to allow in-rush-current to settle down before checking for
 * a latchup current.
 *
 * @param[in] module
 *   SRAM module to be enabled.
 *
 ******************************************************************************/
void BSP_EBI_enableSRAM (BSP_EBI_SRAMSelect_TypeDef module)
{
	uint32_t timeout;

	// Disable IRQ_handler for analog comparator
	NVIC_DisableIRQ(ACMP0_IRQn);

	switch (module)
	{
	//*** SRAM1 ***//
	case bspEbiSram1:
		// enable SRAM module
		GPIO_PinOutSet  (BSP_EBI_SRAM_POWPORT,BSP_EBI_SRAM1_POWPIN); // switch is Enable

		// wait for in-rush-current to pass
		for(timeout = BSP_EBI_TIMEOUT_INRUSH; timeout > 0; timeout--);

		// disable SRAM if latch persists
		if(ACMP0->STATUS & ACMP_STATUS_ACMPOUT)
		{
			GPIO_PinOutClear(BSP_EBI_SRAM_POWPORT,BSP_EBI_SRAM1_POWPIN); // switch is Enable
		}
#if defined (CubeCompV3)
		else
		{
			// connect SRAM module to bus
			GPIO_PinOutClear(BSP_EBI_SRAM_BUFPORT,BSP_EBI_SRAM1_BUFPIN); // buffer is nEnable
			// verify to FPGA that SRAM module is available
			GPIO_PinOutClear(gpioPortE,BSP_EBI_FPGA_CONTROLPIN1);
		}
#endif
		// clear interrupt generated from inrush current
		ACMP0->IFC = ACMP_IFC_EDGE;
		break;

	//*** SRAM2 ***//
	case bspEbiSram2:
		// enable SRAM module
		GPIO_PinOutSet  (BSP_EBI_SRAM_POWPORT,BSP_EBI_SRAM2_POWPIN); // switch is Enable

		// wait for in-rush-current to pass
		for(timeout = BSP_EBI_TIMEOUT_INRUSH; timeout > 0; timeout--);

		// disable SRAM if latch persists
		if(ACMP1->STATUS & ACMP_STATUS_ACMPOUT)
		{
			GPIO_PinOutSet  (BSP_EBI_SRAM_BUFPORT,BSP_EBI_SRAM2_BUFPIN); // buffer is nEnable
		}
#if defined (CubeCompV3)
		else
		{
			// connect SRAM module to bus
			GPIO_PinOutClear(BSP_EBI_SRAM_BUFPORT,BSP_EBI_SRAM2_BUFPIN); // buffer is nEnable
			// verify to FPGA that SRAM module is available
			GPIO_PinOutClear(gpioPortE,BSP_EBI_FPGA_CONTROLPIN2);
		}
#endif
		// clear interrupt generated from inrush current
		ACMP1->IFC = ACMP_IFC_EDGE;
		break;
	}

	// Enable IRQ_handler for analog comparator
	NVIC_ClearPendingIRQ(ACMP0_IRQn);
	NVIC_EnableIRQ(ACMP0_IRQn);
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   17/07/2013
 *
 * This function disables the specified SRAM module from the EBI.
 *
 * @param[in] module
 *   SRAM module to be disabled.
 *
 ******************************************************************************/
void BSP_EBI_disableSRAM (BSP_EBI_SRAMSelect_TypeDef module)
{
	switch (module)
	{
	case bspEbiSram1:
#if defined (CubeCompV3)
		GPIO_PinOutSet(BSP_EBI_FPGA_CONTROLPORT, BSP_EBI_FPGA_CONTROLPIN1);
#endif
		GPIO_PinOutSet  (BSP_EBI_SRAM_BUFPORT,BSP_EBI_SRAM1_BUFPIN); // buffer is nEnable
		GPIO_PinOutClear(BSP_EBI_SRAM_POWPORT,BSP_EBI_SRAM1_POWPIN); // switch is Enable
		break;

	case bspEbiSram2:
#if defined (CubeCompV3)
		GPIO_PinOutSet(BSP_EBI_FPGA_CONTROLPORT, BSP_EBI_FPGA_CONTROLPIN2);
#endif
		GPIO_PinOutSet  (BSP_EBI_SRAM_BUFPORT,BSP_EBI_SRAM2_BUFPIN); // buffer is nEnable
		GPIO_PinOutClear(BSP_EBI_SRAM_POWPORT,BSP_EBI_SRAM2_POWPIN); // switch is Enable
		break;
	}
}


/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   07/06/2012
 *
 * This function writes a data \b buffer of length, \b len, to the EEPROM,
 * starting at a specified \b offset.
 *
 * @param[in] offset
 *   The address offset the data buffer should be written to.
 * @param[in] buffer
 *   The pointer to the data buffer to be written to the EEPROM.
 * @param[in] len
 *   The length of the data buffer to be written to the EEPROM.
 *
 ******************************************************************************/
void BSP_EBI_progEEPROM(uint32_t offset, uint8_t *buffer, uint8_t len)
{
	uint8_t *eepromAddr = (uint8_t*)(BSP_EBI_EEPROM_BASE + offset);

	int i = 0;

	// Write data
	do
	{
		// Unlock commands
		*(eepromAddr + 0x5555) = 0xAA;
		*(eepromAddr + 0x2AAA) = 0x55;
		*(eepromAddr + 0x5555) = 0xA0;

		do
		{
			*(eepromAddr + i) = buffer[i];

			i++;
		}
		while(( i < len ) && ( (i % 64) != 0 ));

		// Poll write sequence completion
		while(((*(eepromAddr+i-1)) & BSP_EBI_EEPROM_POLL_MASK) != (buffer[i-1] & BSP_EBI_EEPROM_POLL_MASK));
	}
	while (i<len);
}

/** @} (end addtogroup EBI) */
/** @} (end addtogroup BSP_Library) */
