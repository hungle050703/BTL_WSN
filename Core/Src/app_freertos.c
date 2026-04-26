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
#include "sensor_service.h"
#include "alarm_logic.h"
#include "mq2.h"    
#include <stdio.h> 
#include "a7677.h" // Thư viện 4G A7677 tự viết
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
extern SensorData_t myData;
osEventFlagsId_t alarmEventHandle; // Cờ sự kiện để báo từ Sensor->4G
#define EVENT_FIRE_DETECTED 0x01   // Mã cờ hiệu Báo cháy (Bit số 1)
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for myQueue01 */
osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = {
  .name = "myQueue01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Start4GTask(void *argument);
osThreadId_t Task_4G_Handle;
const osThreadAttr_t Task_4G_attributes = {
  .name = "Task_4G",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 256 * 4
};
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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of myQueue01 */
  myQueue01Handle = osMessageQueueNew (16, sizeof(uint16_t), &myQueue01_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* Tạo một cái cờ chờ, dùng để báo cháy ngay lập tức sang luồng 4G */
  alarmEventHandle = osEventFlagsNew(NULL);
  
  /* Tạo luồng 4G chạy phụ. Gán priority thấp hơn để ưu tiên đọc cảm biến liên tục */
  Task_4G_Handle = osThreadNew(Start4GTask, NULL, &Task_4G_attributes);
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
  /* USER CODE BEGIN defaultTask */
  /* Infinite loop */
  for(;;)
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
          
          /* Nếu phát hiện sự cố khẩn cấp, "Đá đít/dựng dậy" thằng luồng 4G */
          osEventFlagsSet(alarmEventHandle, EVENT_FIRE_DETECTED);
      } else {
          // Khi bình thường HOẶC đang cháy nhưng ĐÃ bấm nút tắt báo động
          HAL_GPIO_WritePin(LED_RED_ALARM_GPIO_Port, LED_RED_ALARM_Pin, GPIO_PIN_RESET); // Tắt RED
          HAL_GPIO_WritePin(BUZZER_ALARM_GPIO_Port, BUZZER_ALARM_Pin, GPIO_PIN_RESET);   // Tắt Còi
          HAL_GPIO_WritePin(LED_GREEN_ON_GPIO_Port, LED_GREEN_ON_Pin, GPIO_PIN_SET);     // Bật GREEN (Ổn định)
      }

      osDelay(1000); // Đợi 1 giây, PHẢI DÙNG osDelay trong FreeRTOS thay vì HAL_Delay
  }
  /* USER CODE END defaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* MỘT LUỒNG (THREAD) HOÀN TOÀN TÁCH BIỆT DÀNH RIÊNG CHO MẠNG 4G */
void Start4GTask(void *argument)
{
    // 1. Giai đoạn khởi động khi cấp điện: Khởi động nóng cấu cứng A7677
    printf("\r\n[4G_TASK] Dang kich hoat module 4G A7677...\r\n");
    // (Bỏ comment hàm dưới nếu bạn đã cấu hình chân cứng kích nguồn trên mạch in):
    // A7677_PowerOn(); 
    
    char send_buffer[128];
    uint32_t flags;

    for(;;)
    {
        // 2. Chờ Cờ (Flag) "Báo Cháy" từ SensorTask suốt đời, thời gian Timeout cao (như 30 giây rảnh tay làm IoT)
        flags = osEventFlagsWait(alarmEventHandle, EVENT_FIRE_DETECTED, osFlagsWaitAny, 30000);

        if (flags == EVENT_FIRE_DETECTED) {
            /* === CẤP CỨU MẠNG MẠNH NHẤT === */
            printf("\r\n[4G_TASK] NHAN DUOC TIN HIEU CHAY! Dang gui tin nhan & Goi dien...\r\n");
            
            // Xây dựng chuỗi SMS có mang Data luôn
            sprintf(send_buffer, "CANH BAO CHAY KHA CAP!\r\n Nhiet do: %.2fC \r\n Da bam loa!", myData.temperature);
            
            // Thay bằng số điện thoại của bạn
            A7677_SendSMS("0912345678", send_buffer); 
            osDelay(3000); // Đợi mạng dồn gửi đi (3 giây)
            
            // Lập tức lấy quyền gọi réo chuông
            A7677_Call("0912345678");
            
            // Ngủ lâu chút để tránh Spam SMS liên tục khi cháy chưa dập kịp (3 phút)
            printf("[4G_TASK] Da bao dong, tam nghi ngoi SMS 3 phut truoc khi check lai!\r\n");
            osDelay(180000); 

        } else if (flags == osFlagsErrorTimeout) {
            /* === ĐẦY/BÌNH THƯỜNG - CHỨC NĂNG IOT THEO DÕI QUAN TRỌNG ĐỂ SERVER KHÔNG TƯỞNG MẠCH CHẾT === */
            // Cứ mỗi 30 giây không gọi điện, ta gửi bản tin "Tôi đang ok" lên mạng
            printf("\n[4G_TASK IoT] Nhiet phong %2.1fC - He thong dang binh on.\r\n", myData.temperature);
            
            // Tại vị trí này là mã lệnh HTTP POST hoặc MQTT PUB (AT+HTTPPARA) 
            // - Để bạn viết nhúng mạng Blyn/ThingSpeak
        }
    }
}
/* USER CODE END Application */

