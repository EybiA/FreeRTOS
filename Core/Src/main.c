#include "stdio.h"
#include "string.h"
#include "stdint.h"

#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS_CLI.h"
#include "dma.h"
#include "adc.h"
#include "gpio.h"


/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
void MX_USART2_UART_Init(void);
void write_register(unsigned int addr, unsigned int val);
void StartDefaultTask(void *argument);
static void GPIO_blink();

/* ------------------------Private variables -----------------------------------------------*/

UART_HandleTypeDef huart2;
int16_t temp;

/* -----------------------Definitions for defaultTask -------------------------------------- */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* --------------UART input/output configuration---------------------------------------------*/
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)

PUTCHAR_PROTOTYPE
{

	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1,0xFFFF);
  return ch;
}

GETCHAR_PROTOTYPE
{
  uint8_t ch = 0;
  __HAL_UART_CLEAR_OREFLAG(&huart2);
  HAL_UART_Receive (&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;

}

/* ==================================START OF MAIN ROUTINE=============================================== */

int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  vRegisterCLICommands();
  osKernelInitialize();
	 
    write_register (0x4002040c,0x5100); // required for setting I2C #1 pins with internal pull ups
    write_register (0x40020000,0xA80087A0);  // configuring PA5 pin to GPIO
    
    printf("\r\n<<<<<<<Hello from ST32F4466RTE MCU UART (RTOS) terminal>>>>>\r\n");  
   
    #ifdef SENSORS
    temp = I2C_read_temp_sensor();
	  printf("\r\nTemperature is: %d\r\n",temp);  
    #endif

    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

    vUARTCommandConsoleStart(configUART_COMMAND_CONSOLE_STACK_SIZE,configUART_COMMAND_CONSOLE_TASK_PRIORITY);

    osKernelStart();
   
    while(1){}
    
}

/* ==================================END OF MAIN ROUTINE================================================= */

/* ----------------------------------AUX functions------------------------*/
 
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

void write_register(unsigned int addr, unsigned int val)
{

	*((unsigned int *)addr)=((unsigned int *)val);


}

void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  GPIO_PinState PinState = GPIO_PIN_SET;

  /* Infinite loop */
  for(;;)
  {
	PinState = !PinState;
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, PinState);
    osDelay(1000);
  }
  /* USER CODE END 5 */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

void GPIO_blink()
{

	 write_register (0x40020000,0xA80087A0);  // configuring PA5 pin to GPIO
	 write_register (0x40020014,0x20);        // setting the bit to HIGH
	 HAL_Delay(2500);
	 write_register (0x40020014,0x0);         // setting the bit to LOW

	// write_register (0x40020000,0xA8008FA0);  // configuring PA5 GPIO back to
                                              // analog mode
}