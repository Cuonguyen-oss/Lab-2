/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN 0 */

/* ===================== Global variables ===================== */
const int MAX_LED = 4;
const int MAX_LED_MATRIX = 8;

int index_led = 0;
int index_led_matrix = 0;


uint8_t matrix_buffer[8] = {0x00, 0xFC, 0x12, 0x11, 0x11, 0x12, 0xFC, 0x00};

int led_buffer[4] = {1, 2, 3, 4};

int hour = 15, minute = 8, second = 57;

uint16_t pinControll7SEG[4] = {EN0_Pin, EN1_Pin, EN2_Pin, EN3_Pin};

uint16_t enmPins[8] = {ENM0_Pin, ENM1_Pin, ENM2_Pin, ENM3_Pin,
                       ENM4_Pin, ENM5_Pin, ENM6_Pin, ENM7_Pin};

uint16_t rowPins[8] = {ROW0_Pin, ROW1_Pin, ROW2_Pin, ROW3_Pin,
                       ROW4_Pin, ROW5_Pin, ROW6_Pin, ROW7_Pin};

/* ===================== Software timers ===================== */
#define TIMER_CYCLE 10
volatile int timer1_counter = 0, timer1_flag = 0;
volatile int timer2_counter = 0, timer2_flag = 0;
volatile int timer3_counter = 0, timer3_flag = 0;
volatile int timer4_counter = 0, timer4_flag = 0;
volatile int timer5_counter = 0, timer5_flag = 0;

void setTimer1(int duration) { timer1_counter = duration / TIMER_CYCLE; timer1_flag = 0; }
void setTimer2(int duration) { timer2_counter = duration / TIMER_CYCLE; timer2_flag = 0; }
void setTimer3(int duration) { timer3_counter = duration / TIMER_CYCLE; timer3_flag = 0; }
void setTimer4(int duration) { timer4_counter = duration / TIMER_CYCLE; timer4_flag = 0; }
void setTimer5(int duration) { timer5_counter = duration / TIMER_CYCLE; timer5_flag = 0; }

void timerRun(void)
{
    if (timer1_counter > 0) { timer1_counter--; if (timer1_counter <= 0) timer1_flag = 1; }
    if (timer2_counter > 0) { timer2_counter--; if (timer2_counter <= 0) timer2_flag = 1; }
    if (timer3_counter > 0) { timer3_counter--; if (timer3_counter <= 0) timer3_flag = 1; }
    if (timer4_counter > 0) { timer4_counter--; if (timer4_counter <= 0) timer4_flag = 1; }
    if (timer5_counter > 0) { timer5_counter--; if (timer5_counter <= 0) timer5_flag = 1; }
}

/* ===================== 4 x 7SEG ===================== */
void clearEnable(void)
{
    HAL_GPIO_WritePin(GPIOA, EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin, GPIO_PIN_SET);
}

void enablePin(int index)
{
    HAL_GPIO_WritePin(GPIOA, pinControll7SEG[index], GPIO_PIN_RESET);
}

void display7SEG(int num)
{
    uint8_t segNumber[10] = {
        0xC0, 0xF9, 0xA4, 0xB0, 0x99,
        0x92, 0x82, 0xF8, 0x80, 0x90
    };

    for (int i = 0; i < 7; ++i)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 << i,
                          ((segNumber[num] >> i) & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void update7SEG(int index)
{
    if (index < 0 || index >= MAX_LED) return;
    clearEnable();
    display7SEG(led_buffer[index]);
    enablePin(index);
}

void updateClockBuffer(void)
{
    led_buffer[0] = hour / 10;
    led_buffer[1] = hour % 10;
    led_buffer[2] = minute / 10;
    led_buffer[3] = minute % 10;
}

/* ===================== LED MATRIX ===================== */

void setCol(uint8_t val)
{
    for (int i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(GPIOA, enmPins[i],
            ((val >> (7 - i)) & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void setMatrix(void)
{
    for (int i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(GPIOB, rowPins[i], GPIO_PIN_RESET);
    }
}

void updateLEDMatrix(int index)
{
    if (index < 0 || index >= MAX_LED_MATRIX) return;
    setMatrix();
    setCol(matrix_buffer[index]);
    HAL_GPIO_WritePin(GPIOB, rowPins[index], GPIO_PIN_SET);
}

void shiftMatrixLeft(void)
{
    uint8_t first = matrix_buffer[0];
    for (int i = 0; i < 7; i++)
    {
        matrix_buffer[i] = matrix_buffer[i + 1];
    }
    matrix_buffer[7] = first;
}

/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  updateClockBuffer();

  setTimer1(1000);
  setTimer2(250);
  setTimer3(1000);
  setTimer4(10);
  /* USER CODE END 2 */

  while (1)
  {
      if (timer1_flag == 1)
      {
          HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
          HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);
          setTimer1(1000);
      }

      if (timer2_flag == 1)
      {
          update7SEG(index_led);
          index_led++;
          if (index_led >= MAX_LED) index_led = 0;
          setTimer2(250);
      }

      if (timer4_flag == 1)
      {
          updateLEDMatrix(index_led_matrix);
          index_led_matrix++;
          if (index_led_matrix >= MAX_LED_MATRIX) index_led_matrix = 0;
          setTimer4(10);
      }

      if (timer5_flag == 1)
      {
          shiftMatrixLeft();
          setTimer5(500);
      }

      if (timer3_flag == 1)
      {
          second++;
          if (second >= 60) { second = 0; minute++; }
          if (minute >= 60) { minute = 0; hour++; }
          if (hour >= 24)   { hour = 0; }
          updateClockBuffer();
          setTimer3(1000);
      }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin, GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB, SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM2) return;
    timerRun();
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
