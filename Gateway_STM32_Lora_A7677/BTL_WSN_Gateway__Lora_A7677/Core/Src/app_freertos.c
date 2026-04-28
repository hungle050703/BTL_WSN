/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sx1278.h"
#include "a7677.h"
#include <string.h>
#include <stdio.h>
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Lora_Rx_Task */
osThreadId_t Lora_Rx_TaskHandle;
const osThreadAttr_t Lora_Rx_Task_attributes = {
  .name = "Lora_Rx_Task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for Sim4G_Tx_Task */
osThreadId_t Sim4G_Tx_TaskHandle;
const osThreadAttr_t Sim4G_Tx_Task_attributes = {
  .name = "Sim4G_Tx_Task",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 512 * 4
};
/* Definitions for SensorDataQueue */
osMessageQueueId_t SensorDataQueueHandle;
const osMessageQueueAttr_t SensorDataQueue_attributes = {
  .name = "SensorDataQueue"
};
/* Definitions for Lora_Rx_Sem */
osSemaphoreId_t Lora_Rx_SemHandle;
const osSemaphoreAttr_t Lora_Rx_Sem_attributes = {
  .name = "Lora_Rx_Sem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */
  /* creation of Lora_Rx_Sem */
  Lora_Rx_SemHandle = osSemaphoreNew(1, 1, &Lora_Rx_Sem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of SensorDataQueue */
  SensorDataQueueHandle = osMessageQueueNew (10, sizeof(uint32_t), &SensorDataQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of Lora_Rx_Task */
  Lora_Rx_TaskHandle = osThreadNew(StartDefaultTask, NULL, &Lora_Rx_Task_attributes);

  /* creation of Sim4G_Tx_Task */
  Sim4G_Tx_TaskHandle = osThreadNew(StartSim4GTxTask, NULL, &Sim4G_Tx_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN Lora_Rx_Task */
  // --- TẦNG 3: KHỞI TẠO PHẦN CỨNG LORA ---
  // Khởi động module LoRa với 5 thông số vàng tương thích ESP32
  SX1278_Init(); 

  /* Infinite loop */
  for(;;)
  {
    // --- TẦNG 2: NGỦ & BẮT TIN ---
    // Đứng chờ (Ngủ sâu) thu cờ hiệu Semaphore từ Tầng 1 (Chân LORA_DIO0). 
    // osWaitForever giúp tiết kiệm hoàn toàn MPU khi chưa có sóng tới.
    if (osSemaphoreAcquire(Lora_Rx_SemHandle, osWaitForever) == osOK)
    {
      // Có tín hiệu cờ báo, đánh thức Vi điều khiển -> Yêu cầu kiểm tra gói tin
      int packetSize = SX1278_ParsePacket(0);
      if (packetSize > 0)
      {
        // Giới hạn buffer an toàn (Tối đa 31 byte nội dung + 1 byte NULL chặn cuối)
        if (packetSize > 31) {
            packetSize = 31;
        }

        // Xin cấp phát ngẫu nhiên 32 byte RAM của FreeRTOS để chứa mã mảng chuỗi gói tin
        char *rxString = (char*)pvPortMalloc(32); 
        
        if (rxString != NULL)
        {
          memset(rxString, 0, 32); // Xóa sạch rác trong RAM
          // Lấy tin nhắn Payload từ ESP32 đổ vào mảng nhớ vừa sinh ra
          SX1278_ReadBuffer((uint8_t*)rxString, packetSize);
          
          // Thả "Đia chỉ vùng nhớ chứa tin nhắn (Pointer = 4 byte)" này vào đường ống Queue thay vì cả mảng
          // Để cho luồng 4G tí nữa ra nhận
          if (osMessageQueuePut(SensorDataQueueHandle, &rxString, 0, 100) != osOK)
          {
              // Nếu Queue đầy / Đẩy qua ống thất bại -> Giải phóng xóa RAM kẻo tràn bộ nhớ
              vPortFree(rxString);
          }
        }
      }
      
      // Xử lý xong, Lệnh Lora quay lại trạng thái bắt đầu nghe gói tin tiếp theo (Clear FIFO tự động)
      SX1278_Receive(0);
    }
  }
  /* USER CODE END Lora_Rx_Task */
}

/* USER CODE BEGIN Header_StartSim4GTxTask */
/**
* @brief Function implementing the Sim4G_Tx_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSim4GTxTask */
void StartSim4GTxTask(void *argument)
{
  /* USER CODE BEGIN Sim4G_Tx_Task */
  // --- TẦNG 3: KHỞI ĐỘNG MODULE SIM ---
  A7677_PowerOn(); 

  /* Infinite loop */
  for(;;)
  {
    // --- TẦNG 2: NHẬN DỮ LIỆU & ĐẨY LÊN INTERNET ---
    char *txString; 

    // Đứng thu lu ở cửa đường ống (Queue), hễ Thằng Lora lấy túi nào tống vào:
    // osMessageQueueGet sẽ rút lấy Pointer của vùng RAM chứa khối tin nhắn đó ra.
    if (osMessageQueueGet(SensorDataQueueHandle, &txString, NULL, osWaitForever) == osOK)
    {
      // 1. Phân tích tin nếu cần (Ví dụ: txString = "ID:1,T:35H:80")
      // Do mạch đang chạy thư viện AT Command cơ bản chưa có MQTT...
      // Tạm thời mình cho phép SIM 4G gửi luôn toàn bộ Chuỗi văn bản RAW đó qua SMS 
      // để Demo cho Hội Đồng Thầy Cô thấy kết nối Real-time giữa ESP và STM trong 5 giây!
      
      // Bạn có thể đổi số "0123456789" thành SĐT thực tế của bạn
      A7677_SendSMS("0123456789", txString);
      
      // 2. Đẩy xong hoặc đã xử lý chuỗi -> Xoá vùng nhớ RAM đi là kết thúc 1 gói! 
      vPortFree(txString);
    }
  }
  /* USER CODE END Sim4G_Tx_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

