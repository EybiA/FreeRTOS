/*
 * FreeRTOS V202112.00
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


 /******************************************************************************
 *
 * http://www.FreeRTOS.org/cli
 *
 ******************************************************************************/


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"
#include "cmsis_os.h"
#include "main.h"


#ifndef  configINCLUDE_TRACE_RELATED_CLI_COMMANDS
	#define configINCLUDE_TRACE_RELATED_CLI_COMMANDS 0
#endif

#ifndef configINCLUDE_QUERY_HEAP_COMMAND
	#define configINCLUDE_QUERY_HEAP_COMMAND 0
#endif

/*
 * The function that registers the commands that are defined within this file.
 */
//void vRegisterCLICommands( void );

/*
 * Implements the task-stats command.
 */
static BaseType_t prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvTaskRegsCommand();
static BaseType_t prvTaskGPIOCommand();

#ifdef SENSORS
	static BaseType_t prvTempCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
	static const CLI_Command_Definition_t xTemp =
		{
			"temp", /* The command string to type. */
			"",
			prvTempCommand, /* The function to run. */
			0 /* No parameters are expected. */
		};
#endif

/*

/*
 * Implements the echo-three-parameters command.
 */
static BaseType_t prvReadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvWriteCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvDumpCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the "query heap" command.
 */
#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
	static BaseType_t prvQueryHeapCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif


/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xRegsStats =
{
	"regs", /* The command string to type. */
	"",
	prvTaskRegsCommand, /* The function to run. */
	0 /* No parameters are expected. */
};


static const CLI_Command_Definition_t xRdRegsiter =
{
	"rd",
	"",
	prvReadCommand, /* The function to run. */
	1 /* One parameter are expected, which can take any value. */
};

static const CLI_Command_Definition_t xWrRegsiter =
{
	"wr",
	"",
	prvWriteCommand, /* The function to run. */
	2 /* One parameter are expected, which can take any value. */
};

static const CLI_Command_Definition_t xDumpRegsiter =
{
	"dump",
	"",
	prvDumpCommand, /* The function to run. */
	2 /* One parameter are expected, which can take any value. */
};


static const CLI_Command_Definition_t xGpio =
{
	"gpio", /* The command string to type. */
	"",
	prvTaskGPIOCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

	/* Structure that defines the "run-time-stats" command line command.   This
	generates a table that shows how much run time each task has */
	static const CLI_Command_Definition_t xRunTimeStats =
{
	"run-time-stats", /* The command string to type. */
	"",
	prvTaskStatsCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
	/* Structure that defines the "query_heap" command line command. */
	static const CLI_Command_Definition_t xQueryHeap =
	{
		"query-heap",
		"\r\nquery-heap:\r\n Displays the free heap space, and minimum ever free heap space.\r\n",
		prvQueryHeapCommand, /* The function to run. */
		0 /* The user can enter any number of commands. */
	};
#endif /* configQUERY_HEAP_COMMAND */

/*-----------------------------------------------------------*/

void vRegisterCLICommands( void )
{
	FreeRTOS_CLIRegisterCommand( &xRunTimeStats );	
	FreeRTOS_CLIRegisterCommand( &xRegsStats );
	FreeRTOS_CLIRegisterCommand( &xRdRegsiter);
	FreeRTOS_CLIRegisterCommand( &xWrRegsiter);
	FreeRTOS_CLIRegisterCommand( &xDumpRegsiter);
	FreeRTOS_CLIRegisterCommand( &xGpio);
	
	#ifdef SENSORS
		FreeRTOS_CLIRegisterCommand( &xTemp);
	#endif


	#if( configGENERATE_RUN_TIME_STATS == 1 )
	{
		FreeRTOS_CLIRegisterCommand( &xRunTimeStats );
	}
	#endif
	
	#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )
	{
		FreeRTOS_CLIRegisterCommand( &xQueryHeap );
	}
	#endif

	#if( configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1 )
	{
		FreeRTOS_CLIRegisterCommand( &xStartStopTrace );
	}
	#endif
}
/*-----------------------------------------------------------*/

static BaseType_t prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char *const pcHeader = " State  Priority  Stack    #\r\n************************************************\r\n";
BaseType_t xSpacePadding;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	pcWriteBuffer="";

	/* Generate a table of task stats. */
	strcpy( pcWriteBuffer, "Task" );
	pcWriteBuffer += strlen( pcWriteBuffer );

	/* Minus three for the null terminator and half the number of characters in
	"Task" so the column lines up with the centre of the heading. */
	configASSERT( configMAX_TASK_NAME_LEN > 3 );
	for( xSpacePadding = strlen( "Task" ); xSpacePadding < ( configMAX_TASK_NAME_LEN - 3 ); xSpacePadding++ )
	{
		/* Add a space to align columns after the task's name. */
		*pcWriteBuffer = ' ';
		pcWriteBuffer++;

		/* Ensure always terminated. */
		*pcWriteBuffer = 0x00;
	}
	strncpy( pcWriteBuffer, pcHeader, strlen(pcHeader) );
	vTaskList( pcWriteBuffer + strlen( pcHeader ) );

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

#if( configINCLUDE_QUERY_HEAP_COMMAND == 1 )

	static BaseType_t prvQueryHeapCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
	{
		/* Remove compile time warnings about unused parameters, and check the
		write buffer is not NULL.  NOTE - for simplicity, this example assumes the
		write buffer length is adequate, so does not check for buffer overflows. */
		( void ) pcCommandString;
		( void ) xWriteBufferLen;
		configASSERT( pcWriteBuffer );

		sprintf( pcWriteBuffer, "Current free heap %d bytes, minimum ever free heap %d bytes\r\n", ( int ) xPortGetFreeHeapSize(), ( int ) xPortGetMinimumEverFreeHeapSize() );

		/* There is no more data to return after this single string, so return
		pdFALSE. */
		return pdFALSE;
	}

#endif /* configINCLUDE_QUERY_HEAP */
/*-----------------------------------------------------------*/

static BaseType_t prvReadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char *pcParameter;
unsigned int  data;
BaseType_t xParameterStringLength, xReturn;
static UBaseType_t uxParameterNumber = 0;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	/* Obtain the parameter string. */
	pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,		/* The command string itself. */
							1,		                /* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);

		/* Sanity check something was returned. */
		configASSERT( pcParameter );

		/* Return the parameter string. */
		
		uintptr_t addr = (uintptr_t)strtoul(pcParameter, NULL, 0);
		data=(*(volatile unsigned int*)(addr));

		sprintf( pcWriteBuffer, "\r\nRegister 0x%x, value : 0x%x \r"  ,addr,data);
		xReturn = pdFALSE;

	return xReturn;
}
/*-----------------------------------------------------------*/

static BaseType_t prvWriteCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char *pcParameter;
BaseType_t xParameterStringLength, xReturn;
static UBaseType_t uxParameterNumber = 0;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	/* Obtain the parameter string. */
	pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,		/* The command string itself. */
							1,		                /* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);

		configASSERT( pcParameter );
		uintptr_t addr = (uintptr_t)strtoul(pcParameter, NULL, 0);

		pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,		/* The command string itself. */
							2,		                /* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);

		configASSERT( pcParameter );
		uintptr_t data = (uintptr_t)strtoul(pcParameter, NULL, 0);

	    *((unsigned int *)addr)=((unsigned int *)data);

		sprintf( pcWriteBuffer, "\r\nRegister 0x%x, set to value : 0x%x \r"  ,addr,data);
		xReturn = pdFALSE;

	return xReturn;
}
/*-----------------------------------------------------------*/


static BaseType_t prvDumpCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char *pcParameter;
unsigned int  data;
char *pcWriteTempBuffer;
BaseType_t xParameterStringLength, xReturn;
static UBaseType_t uxParameterNumber = 0;
int i=0;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	pcWriteBuffer="";

	/* Obtain the parameter string. */
	pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,		/* The command string itself. */
							1,		                /* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);

		configASSERT( pcParameter );
		uintptr_t addr = (uintptr_t)strtoul(pcParameter, NULL, 0);

		pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,		/* The command string itself. */
							2,		                /* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);

		configASSERT( pcParameter );
		uintptr_t count = (uintptr_t)strtoul(pcParameter, NULL, 0);

		for (i=0; i<count; i++)
		{
	    	data=(*(volatile unsigned int*)(addr));
			printf("\r\nRegister 0x%x, value : 0x%x \r"  ,addr,data);
			addr=addr+4;
		}

		pcWriteBuffer="";
		printf("\r\n");
		xReturn = pdFALSE;

	return xReturn;
}
/*-----------------------------------------------------------*/

static BaseType_t prvTaskRegsCommand()

{
	  printf("\r\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<ST32F446 Registers map >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
	  printf("\r=========================================================================================================\r\n");
	  printf("\rFLASH_BASE          : 0x08000000\r\n");
	  printf("CCMDATARAM_BASE       : 0x10000000\r\n");
	  printf("SRAM1_BASE            : 0x20000000\r\n");
	  printf("SRAM2_BASE            : 0x2001C000\r\n");
	  printf("SRAM3_BASE            : 0x20020000\r\n");
	  printf("PERIPH_BASE           : 0x40000000\r\n");
	  printf("Timers                : 0x40000000-0x400023FF\r\n");
	  printf("RTC                   : 0x40002800-0x40002BFF\r\n");
	  printf("SPI#2/I2S#2           : 0x40003800-0x40003BFF\r\n");
	  printf("SPI#3/I2S#3           : 0x40003C00-0x40003FFF\r\n");
	  printf("USART#2-UART5         : 0x40004400-0x400053FF\r\n");
	  printf("I2C#1-#3              : 0x40005400-0x40005FFF\r\n");
	  printf("DAC                   : 0x40007400-0x400077FF\r\n");
	  printf("ADC#1-#3              : 0x40012000-0x400123FF\r\n");
	  printf("SPI#1,#4              : 0x40013000-0x400137FF\r\n");
	  printf("SYSCFG                : 0x40013800-0x40013BFF\r\n");
	  printf("SAI #1,#2             : 0x40015800-0x40015FFF\r\n");
	  printf("GPIOs                 : 0x40020000-0x40021FFF\r\n");
	  printf("RCC                   : 0x40023800-0x40023BFF\r\n");
	  printf("DMA#1, #2             : 0x40026000-0x400267FF\r\n");
	  printf("DCMI                  : 0x50050000-0x500503FF\r\n");
	  printf("\r=========================================================================================================\r\n");

}

/*-----------------------------------------------------------*/

static BaseType_t prvTaskGPIOCommand()

{

static UBaseType_t uxParameterNumber = 0;

extern osThreadId_t defaultTaskHandle;
extern bool xGpioMutex;
static osStatus_t task_status;
BaseType_t xReturn;

if (uxParameterNumber==0) {
	uxParameterNumber = 1;	
	printf("\r\nGPIO LED toggling stopped!\r\n");
	xGpioMutex = false;

	}
	else
	{
	uxParameterNumber = 0;
	printf("\r\nGPIO LED toggling resumed!\r\n");
	xGpioMutex =true;
	}

	xReturn = pdFALSE;

}

/*-----------------------------------------------------------*/

static BaseType_t prvTempCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )

{

int16_t temp;
BaseType_t xReturn;


temp = I2C_read_temp_sensor();
sprintf( pcWriteBuffer, "\r\nTemperature is: %d\r\n",temp); 
xReturn = pdFALSE;

}

/*-----------------------------------------------------------*/