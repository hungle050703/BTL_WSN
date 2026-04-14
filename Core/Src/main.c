/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "icache.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sensor_service.h"
#include "alarm_logic.h"
#include "mq2.h"    // Đã thêm thư viện mq2.h để nhận diện được hàm MQ2_GetBaseVoltage()
#include <stdio.h> // Để dùng printf
#include <string.h> // Để dùng strlen
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SensorData_t myData;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern UART_HandleTypeDef huart3; // Dựa vào cấu hình chân USART3 của NUCLEO
int __io_putchar(int ch) {
    if (huart3.Instance != NULL) { // Tránh lỗi con trỏ null nếu bạn quên cấu hình UART3
        HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 10);
    }
    return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ICACHE_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */
  printf("System Starting...\r\n");
  HAL_TIM_Base_Start(&htim2); 
  SensorService_Init();
  AlarmLogic_Init();
  printf("Init Done! BTL He Thong Nhung 2026\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // Cập nhật cảm biến (Non-blocking)
      SensorService_Update(&myData);
      float temp = myData.temperature;
      float smoke_v = myData.smoke_conc;
      
      // Chuyển đổi điện áp thành phần trăm (%) để hiển thị trực quan
      float smoke_percentage = (smoke_v / 5.0f) * 100.0f;
      
      // --- Phân mức độ nguy hiểm Khói/Khí Gas bằng Calibration Động ---
      float base_v = MQ2_GetBaseVoltage(); // Lấy mức nền của lúc khởi động
      float danger_threshold = base_v + 0.6f; // Nguy hiểm khi vọt lên +0.6V so với nền
      char* smoke_status;
      
      if (smoke_v < base_v + 0.2f) { 
          smoke_status = "SACH"; // Gần mức nền +0.2V (chấp nhận trôi nhiệt)
      } else if (smoke_v < danger_threshold) {
          smoke_status = "CO KHOI NHE";
      } else {
          smoke_status = "NGUY HIEM!";
      }

      // Xử lý cờ Hồng ngoại / Lửa từ con MH-Sensor (DO pin - PA3)
      char* mh_status = (myData.mh_sensor_do == 0) ? "CO LUA/VAT CAN!!" : "BINH THUONG";

      // Màn hình giao diện quản lý toàn bộ Sensors
      printf("Nhiet do: %5.2f C \r\n", temp);
      printf("  Khoi MQ-2:   %4.1f %% - %s (Base: %.2fV | Now: %.2fV)\r\n", smoke_percentage, smoke_status, base_v, smoke_v);
      printf("  Lua MH-SENS: %s (AO: %.2fV)\r\n", mh_status, myData.mh_sensor_ao_volt);
      printf("----------------------------------------\r\n");

      static uint8_t alarm_muted = 0; // Biến lưu trạng thái đã tắt báo động

      // Nếu nhiệt độ và khói và MH trở lại bình thường, reset trạng thái Mute để lần sau báo cháy tiếp
      if (temp <= 50.0f && smoke_v <= danger_threshold && myData.mh_sensor_do == 1) {
          alarm_muted = 0;
      }

      // Kiểm tra nút nhấn BTN1_USER (PC13 - Nút B1 màu xanh trên board) để TẮT BÁO ĐỘNG (Mute)
      if (HAL_GPIO_ReadPin(BTN1_USER_GPIO_Port, BTN1_USER_Pin) == GPIO_PIN_SET) {
          alarm_muted = 1; // Ghi nhớ là người dùng đã tắt báo động
          printf("--- DA TAT BAO DONG BANG NUT NHAN! ---\r\n");
      }

      // Logic điều khiển LED và Còi
      // Còi kêu khi: Lửa, quá nhiệt, HOẶC phát hiện có nhiều khói (vượt danger_threshold)
      uint8_t is_fire = (temp > 50.0f || myData.mh_sensor_do == 0);
      uint8_t is_heavy_smoke = (smoke_v > danger_threshold); 
      
      if ((is_fire || is_heavy_smoke) && alarm_muted == 0) {
          // Khi cháy và CHƯA bị tắt báo động
          HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_SET);   // Bật RED
          HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_SET);     // Bật Còi
          HAL_GPIO_WritePin(LED_GREEN_ON_GPIO_Port, LED_GREEN_ON_Pin, GPIO_PIN_RESET);   // Tắt GREEN
          printf("--- CANH BAO HOA HOAN! ---\r\n");
      } else {
          // Khi bình thường HOẶC đang cháy nhưng ĐÃ bấm nút tắt báo động
          HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET); // Tắt RED
          HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);   // Tắt Còi
          HAL_GPIO_WritePin(LED_GREEN_ON_GPIO_Port, LED_GREEN_ON_Pin, GPIO_PIN_SET);     // Bật GREEN (Ổn định)
      }

      HAL_Delay(1000); // Đợi 1 giây để log không bị trôi quá nhanh
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 250;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the programming delay
  */
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};
  MPU_Attributes_InitTypeDef MPU_AttributesInit = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region 0 and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x08FFF000;
  MPU_InitStruct.LimitAddress = 0x08FFFFFF;
  MPU_InitStruct.AttributesIndex = MPU_ATTRIBUTES_NUMBER0;
  MPU_InitStruct.AccessPermission = MPU_REGION_ALL_RO;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Attribute 0 and the memory to be protected
  */
  MPU_AttributesInit.Number = MPU_ATTRIBUTES_NUMBER0;
  MPU_AttributesInit.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);

  HAL_MPU_ConfigMemoryAttributes(&MPU_AttributesInit);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
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
#ifdef USE_FULL_ASSERT
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
