/***************************************************************************//**
 * @file	bsp_i2c.c
 * @brief	BSP EBI source file.
 *
 * This file contains all the implementations for the functions defined in \em
 * bsp_i2c.h.
 * @author	Pieter J. Botma
 * @date	10/05/2012
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

#include "bsp_i2c.h"
#include "em_i2c.h"
#include "em_cmu.h"
#include "em_gpio.h"

/***************************************************************************//**
 * @addtogroup BSP_Library
 * @brief Board Support Package (<b>BSP</b>) Driver Library for CubeComputer.
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup I2C
 * @brief API for CubeComputer's I2C modules.
 * @{
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

void InitSys (bool master)
{
	// Setup clocks
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_I2C0, true);

	// Using PC6 (SDA) and PC7 (SCL)
	GPIO_PinModeSet(gpioPortC, 7, gpioModeWiredAndPullUpFilter, 1);
	GPIO_PinModeSet(gpioPortC, 6, gpioModeWiredAndPullUpFilter, 1);

	// Enable pins at location 2
	I2C0->ROUTE = I2C_ROUTE_SDAPEN |
			      I2C_ROUTE_SCLPEN |
			      I2C_ROUTE_LOCATION_LOC2;

	// Use default settings, defined in em_i2c.h
	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;

	i2cInit.master = (master) ? 1 : 0; // enable/disable master mode

	// Initializing the I2C
	I2C_Init(I2C0, &i2cInit);

	// Abort if undefined reset
	I2C0->CMD = I2C_CMD_ABORT;

	if(!master) // i.e. slave
	{
		// Setting up to enable slave mode
		I2C0->SADDR = BSP_I2C_SYS_ADDRESS;
		I2C0->SADDRMASK = _I2C_SADDRMASK_MASK;
		I2C0->CTRL |= I2C_CTRL_SLAVE | I2C_CTRL_AUTOACK;

		// enable interrupts for slave mode
		BSP_I2C_setSlaveMode (I2C0, true);

		// Clear and enable interrupt from I2C module
		NVIC_ClearPendingIRQ(I2C0_IRQn);
		NVIC_EnableIRQ(I2C0_IRQn);
	}
}


void InitSub (bool master)
{
	// Setup clocks
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_I2C1, true);

	// Using PC4 (SDA) and PC5 (SCL)
	GPIO_PinModeSet(gpioPortC, 4, gpioModeWiredAndPullUpFilter, 1);
	GPIO_PinModeSet(gpioPortC, 5, gpioModeWiredAndPullUpFilter, 1);

	// Enable pins at location 2
	I2C1->ROUTE = I2C_ROUTE_SDAPEN |
			      I2C_ROUTE_SCLPEN |
			      I2C_ROUTE_LOCATION_LOC0;

	// Use default settings, defined in em_i2c.h
	I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;

	i2cInit.master = (master) ? 1 : 0; // enable/disable master mode

	// Initializing the I2C
	I2C_Init(I2C1, &i2cInit);

	// Abort if undefined reset
	I2C1->CMD = I2C_CMD_ABORT;

	if(!master) // i.e. slave
	{
		// Setting up to enable slave mode
		I2C1->SADDR = BSP_I2C_SYS_ADDRESS;
		I2C1->SADDRMASK = _I2C_SADDRMASK_MASK;
		I2C1->CTRL |= I2C_CTRL_SLAVE | I2C_CTRL_AUTOACK;

		// enable interrupts for slave mode
		BSP_I2C_setSlaveMode (I2C1, true);

		// Clear and enable interrupt from I2C module
		NVIC_ClearPendingIRQ(I2C1_IRQn);
		NVIC_EnableIRQ(I2C1_IRQn);
	}
}

/** @endcond */

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   23/05/2012
 *
 * This function initialises the specified I2C channel as a master transmitters.
 * @param[in] i2c
 *   Pointer of the I2C module to be initialised.
 ******************************************************************************/
void BSP_I2C_Init (I2C_TypeDef *i2c, bool master)
{
	if(i2c == BSP_I2C_SYS)
		InitSys(master); // Initialise main I2C channel
	else if (i2c == BSP_I2C_SUB)
		InitSub(master); // Initialise subsystem I2C channel
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   23/05/2012
 *
 * This function enables or disables the interrupts needed for slave mode
 * operation of the specified I2C module.
 *
 * @param[in] i2c
 *   Pointer to the specified I2C module.
 * @param[in] enable
 *   Indicates if slave mode should be enabled or disabled for the specified I2C.
 ******************************************************************************/
void BSP_I2C_setSlaveMode(I2C_TypeDef *i2c, bool enable)
{
	if(enable)
	{
		// Clear flags + set and enable interrupts
		i2c->IFC = ~0;
		i2c->IEN = I2C_IEN_ADDR | I2C_IEN_RXDATAV | I2C_IEN_ACK | I2C_IEN_NACK | I2C_IEN_SSTOP;
		if (i2c == I2C0)
			NVIC_EnableIRQ(I2C0_IRQn);
		else
			NVIC_EnableIRQ(I2C1_IRQn);
	}
	else
	{
		// Disable + clear interrupts and flags
		if (i2c == I2C0)
			NVIC_DisableIRQ(I2C0_IRQn);
		else
			NVIC_DisableIRQ(I2C1_IRQn);
		i2c->IEN = 0;
		i2c->IFC = ~0;
	}
}

/***************************************************************************//**
 * @author Pieter J. Botma
 * @date   23/05/2012
 *
 * This function starts a communication sequence, of type \b flag, with a slave
 * module, \b address, by transmitting \b txBuffer and or receiving \b rxBuffer
 * over the specified I2C channel.
 *
 * @param[in] i2c
 *   Pointer to the I2C module to be used.
 * @param[in] address
 *   I2C addres of slave to initiate communication with.
 * @param[in] flag
 *   Indicates the type of communication sequence.
 * @param[in] txBuffer
 *   Pointer to data buffer to be transmitted over the specified I2C channel.
 * @param[in] txBufferSize
 *   Size of data buffer to be tranmistted.
 * @param[out] rxBuffer
 *   Pointer to buffer where data, received over the specified I2C channel, should be saved.
 * @param[in] rxBufferSize
 *   Number of bytes received over the specified I2C channel.
 ******************************************************************************/
uint32_t BSP_I2C_masterTX (I2C_TypeDef *i2c, uint16_t address, BSP_I2C_ModeSelect_TydeDef flag,
					  uint8_t *txBuffer, uint16_t txBufferSize,
					  uint8_t *rxBuffer, uint16_t rxBufferSize)
{
  //i2cDisableSlaveInterrupts();
  //BSP_I2C_setSlaveMode(i2c,0);

  I2C_TransferReturn_TypeDef result;
  I2C_TransferSeq_TypeDef i2cTransfer;

  // Initializing I2C transfer
  i2cTransfer.addr        = address;
  i2cTransfer.flags       = flag;
  i2cTransfer.buf[0].data = txBuffer;
  i2cTransfer.buf[0].len  = txBufferSize;
  i2cTransfer.buf[1].data = rxBuffer;
  i2cTransfer.buf[1].len  = rxBufferSize;

  I2C_TransferInit(i2c, &i2cTransfer);

  uint32_t timeout = 0x0FFFFF;

  /* Sending data */
  do
  {
	  timeout--;

	  result = I2C_Transfer(i2c);
  }
  while (result == i2cTransferInProgress && timeout != 0);

  return timeout;
}

/** @} (end addtogroup I2C) */
/** @} (end addtogroup BSP_Library) */

/*void BSP_I2C_MasterWriteRead (uint8_t address, uint8_t* txBuf, uint8_t txLen, uint8_t* rxBuf, uint8_t rxLen)
{
	uint32_t int_flags;
	uint32_t tmp;

	uint8_t txIndex, rxIndex;

	// Check if in busy state. Since this SW assumes single master, we can
	// just issue an abort. The BUSY state is normal after a reset.
	if (i2c->STATE & I2C_STATE_BUSY)
	{
	  i2c->CMD = I2C_CMD_NACK;
	  i2c->CMD = I2C_CMD_ABORT;
	}

	// Ensure buffers are empty
	i2c->CMD = I2C_CMD_CLEARPC | I2C_CMD_CLEARTX;
	if (i2c->IF & I2C_IF_RXDATAV)
	{
		i2c->RXDATA;
	}

	// Clear all pending interrupts prior to starting transfer.
	i2c->IFC = _I2C_IFC_MASK;


	// STEP 1: transmit start + address

    i2c->TXDATA = address; // Data not transmitted until START sent
    i2c->CMD    = I2C_CMD_START;

    // wait for ack
    while(!(i2c->IF & I2C_IF_ACK));

    i2c->IFC = I2C_IFC_ACK; // clear ack

    // STEP 2: transmit data

    for(txIndex = 0; txIndex < txLen; txIndex++)
    {
    	i2c->TXDATA = txBuf[txIndex];

        // wait for ack
        while(!(i2c->IF & I2C_IF_ACK));

        // clear ack
        i2c->IFC = I2C_IFC_ACK;
    }

    // STEP 3: transmit repeated start + address

    i2c->CMD    = I2C_CMD_START;
    i2c->TXDATA = address | 1; // read

    while(!(i2c->IF & I2C_IF_ACK));

    i2c->IFC = I2C_IFC_ACK; // clear ack

    for(rxIndex = 0; rxIndex < rxLen; rxIndex++)
    {
    	// reading data will clear flag
    	while(!(i2c->IF & I2C_IF_RXDATAV));

    	rxBuf[rxIndex] = i2c->RXDATA;

    	//i2c->IFC = I2C_IFC_ACK;

    	if(rxIndex < rxLen-1)
    		i2c->CMD = I2C_CMD_ACK;
    }

    while (i2c->IF & I2C_IF_RXDATAV)
    	i2c->CMD = I2C_CMD_NACK;

    // STEP 4: transmit nack and stop
    i2c->CMD = I2C_CMD_NACK;
    i2c->CMD = I2C_CMD_STOP;

    while(!(i2c->IF & I2C_IF_MSTOP));

    i2c->IFC = I2C_IF_MSTOP; // clear ack
}


void BSP_I2C_TxTCMD (uint8_t address, uint8_t tcmdId, uint8_t* tcmdBuf, uint8_t tcmdLen)
{
	uint8_t txIndex;

	// Check if in busy state. Since this SW assumes single master, we can
	// just issue an abort. The BUSY state is normal after a reset.
	if (i2c->STATE & I2C_STATE_BUSY)
	{
	  i2c->CMD = I2C_CMD_NACK;
	  i2c->CMD = I2C_CMD_ABORT;
	}

	// Ensure buffers are empty
	i2c->CMD = I2C_CMD_CLEARPC | I2C_CMD_CLEARTX;
	if (i2c->IF & I2C_IF_RXDATAV)
	{
		i2c->RXDATA;
	}

	// Clear all pending interrupts prior to starting transfer.
	i2c->IFC = _I2C_IFC_MASK;


	// STEP 1: transmit start + address
    i2c->TXDATA = address; // Data not transmitted until START sent
    i2c->CMD    = I2C_CMD_START;

    // wait for ack
    while(!(i2c->IF & I2C_IF_ACK));

    i2c->IFC = I2C_IFC_ACK; // clear ack

    // STEP 2: transmit id
	i2c->TXDATA = tcmdId;

	// wait for ack
	while(!(i2c->IF & I2C_IF_ACK));

	// clear ack
	i2c->IFC = I2C_IFC_ACK;

    // STEP 3: transmit tcmd

    for(txIndex = 0; txIndex < tcmdLen; txIndex++)
    {
    	i2c->TXDATA = tcmdBuf[txIndex];

        // wait for ack
        while(!(i2c->IF & I2C_IF_ACK));

        // clear ack
        i2c->IFC = I2C_IFC_ACK;
    }

    // STEP 4: transmit stop
    i2c->CMD = I2C_CMD_STOP;

    // wait until stop sent
    while(!(i2c->IF & I2C_IF_MSTOP));

    i2c->IFC = I2C_IF_MSTOP; // clear stop flag
}*/




