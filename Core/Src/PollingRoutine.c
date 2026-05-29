/*
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 Karl Yamashita
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
 */

/*
 * PollingRoutine.c
 *
 *  Created on: Oct 24, 2023
 *      Author: karl.yamashita
 *
 *
 *      Template for projects.
 *
 *      The object of this PollingRoutine.c/h files is to not have to write code in main.c which already has a lot of generated code.
 *      It is cumbersome having to scroll through all the generated code for your own code and having to find a USER CODE section so your code is not erased when CubeMX re-generates code.
 *      
 *      Direction: Call PollingInit before the main while loop. Call PollingRoutine from within the main while loop
 * 
 *      Example;
        // USER CODE BEGIN WHILE
        PollingInit();
        while (1)
        {
            PollingRoutine();
            // USER CODE END WHILE

            // USER CODE BEGIN 3
        }
        // USER CODE END 3

 */


#include "main.h"


#define UART2_DMA_RX_QUEUE_SIZE 10 // rx queue size
#define UART2_DMA_TX_QUEUE_SIZE 4 // tx queue size
UART_DMA_Data uart2_dmaDataRxQueue[UART2_DMA_RX_QUEUE_SIZE] = {0}; // rx queue
UART_DMA_Data uart2_dmaDataTxQueue[UART2_DMA_TX_QUEUE_SIZE] = {0}; // tx queue

UART_DMA_Struct_t uart2_msg =
{
	.huart = &huart2,
	.rx.msgQueue = uart2_dmaDataRxQueue, // assign queue
	.rx.queueSize = UART2_DMA_RX_QUEUE_SIZE,
	.tx.msgQueue = uart2_dmaDataTxQueue, // assign queue
	.tx.queueSize = UART2_DMA_TX_QUEUE_SIZE,

	.rx.packetSize = UART_BINARY_PACKET_SIZE, // the size of the binary packet including checksum
	.uartType = UART_BINARY, // parsing for binary packets
	.dma.dmaPtr.SkipOverFlow = true
};

void PollingInit(void)
{
	UART_DMA_EnableRxInterruptIdle(&uart2_msg);

	HAL_Delay(1);
	RingBuff_Ptr_Reset_V(&uart2_msg.dma.circularPtr);
	RingBuff_Ptr_Reset_V(&uart2_msg.dma.dmaPtr);

	TimerCallbackRegisterOnly(&timerCallback, LED_Toggle);
}

void PollingRoutine(void)
{
	UART_DMA_ParseCircularBuffer(&uart2_msg);

	UART_ParseCommands(&uart2_msg);
}

void UART_ParseCommands(UART_DMA_Struct_t *msg)
{
	while(UART_DMA_RxMsgRdy(msg))
	{
		// echo back packet or string
		if(msg->uartType == UART_BINARY)
		{
			UART_DMA_TX_AddDataToBuffer(&uart2_msg, msg->rx.msgToParse->data, UART_BINARY_PACKET_SIZE);
		}
		else if(msg->uartType == UART_ASCII)
		{
			UART_DMA_NotifyUser(&uart2_msg, (char*)msg->rx.msgToParse->data, msg->rx.msgToParse->size, false);
		}
	}
}

void STM32_Ready(UART_DMA_Struct_t *msg)
{
	char str[UART_DMA_QUEUE_DATA_SIZE] = {0};

	sprintf(str, "Telemetry Parser ready");

	UART_DMA_NotifyUser(msg, str, strlen(str), true);
}

void LED_Toggle(void)
{
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}



