#include "main.h"
#include "usb_device.h"
#include "usbd_hid.h"

TIM_HandleTypeDef htim3;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);

void delay_us(uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	while (__HAL_TIM_GET_COUNTER(&htim3) < us);
}

extern USBD_HandleTypeDef hUsbDeviceFS;

typedef struct
{
	uint8_t MODIFIER;
	uint8_t RESERVED;
	uint8_t KEYCODE1;
	uint8_t KEYCODE2;
	uint8_t KEYCODE3;
	uint8_t KEYCODE4;
	uint8_t KEYCODE5;
	uint8_t KEYCODE6;
} keyboardHID;

keyboardHID keyboardhid = {0, 0, 0, 0, 0, 0, 0, 0};

uint16_t lastdata1 = 0;
uint16_t lastdata2 = 0;

uint8_t keyboardKeys1[]={44,8,21,0,9,6,27,20,0,40,0,0,26,22,4,7};
uint8_t keyboardKeys2[]={89,90,91,0,92,93,94,98,0,88,0,0,82,81,80,79};

enum SEGA_KEYS {
	SMD_A = 0,
	SMD_B,
	SMD_C,
	SMD_EMPTY_1,
	SMD_X,
	SMD_Y,
	SMD_Z,
	SMD_MODE,
	SMD_EMPTY_2,
	SMD_START,
	SMD_EMPTY_3,
	SMD_EMPTY_4,
	SMD_UP,
	SMD_DOWN,
	SMD_LEFT,
	SMD_RIGHT,
	SMD_MAX_KEYS
};

void readJoysticState(uint16_t *data1, uint16_t *data2)
{
	*data1 = 0;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	delay_us(1);

	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) << SMD_A;
	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) << SMD_START;
	*data1 += !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) << SMD_UP;
	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) << SMD_DOWN;

	delay_us(1200);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_us(1);

	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) << SMD_LEFT;
	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) << SMD_RIGHT;
	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) << SMD_B;
	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) << SMD_C;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	delay_us(1200);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	delay_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_us(1);

	*data1 += !HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) << SMD_Z;
	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) << SMD_Y;
	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) << SMD_X;
	*data1 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) << SMD_MODE;


	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	delay_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_us(100);
	*data2 = 0;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	delay_us(1);

	*data2 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) << SMD_A;
	*data2 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13) << SMD_START;
	*data2 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) << SMD_UP;
	*data2 += !HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0) << SMD_DOWN;

	delay_us(1200);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_us(1);

	*data2 += !HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) << SMD_LEFT;
	*data2 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_14) << SMD_RIGHT;
	*data2 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) << SMD_B;
	*data2 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13) << SMD_C;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	delay_us(1200);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	delay_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_us(1);

	*data2 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) << SMD_Z;
	*data2 += !HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_0) << SMD_Y;
	*data2 += !HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_1) << SMD_X;
	*data2 += !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_14) << SMD_MODE;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	delay_us(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_us(100);
}

void setKeyboardState(uint8_t key, _Bool state)
{
	if (state)
	{
		if (keyboardhid.KEYCODE1 == 0)	keyboardhid.KEYCODE1 = key;
		else if (keyboardhid.KEYCODE2 == 0)	keyboardhid.KEYCODE2 = key;
		else if (keyboardhid.KEYCODE3 == 0)	keyboardhid.KEYCODE3 = key;
		else if (keyboardhid.KEYCODE4 == 0)	keyboardhid.KEYCODE4 = key;
		else if (keyboardhid.KEYCODE5 == 0)	keyboardhid.KEYCODE5 = key;
		else if (keyboardhid.KEYCODE6 == 0)	keyboardhid.KEYCODE6 = key;
		else return;

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
	}
	else
	{
		if (keyboardhid.KEYCODE1 == key) keyboardhid.KEYCODE1 = 0;
		else if (keyboardhid.KEYCODE2 == key) keyboardhid.KEYCODE2 = 0;
		else if (keyboardhid.KEYCODE3 == key) keyboardhid.KEYCODE3 = 0;
		else if (keyboardhid.KEYCODE4 == key) keyboardhid.KEYCODE4 = 0;
		else if (keyboardhid.KEYCODE5 == key) keyboardhid.KEYCODE5 = 0;
		else if (keyboardhid.KEYCODE6 == key) keyboardhid.KEYCODE6 = 0;
		else return;

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	}
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_TIM3_Init();

  HAL_TIM_Base_Start(&htim3);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);

  while (1)//
  {
		uint16_t data1 = 0;
		uint16_t data2 = 0;
		readJoysticState(&data1, &data2);

		for (int i=0;i<SMD_MAX_KEYS;i++)
		{
			if (keyboardKeys1[i] == 0) continue;

			_Bool c1 = (data1 >> i) & 1;

			if (c1 != ((lastdata1 >> i) & 1))
			{
				setKeyboardState(keyboardKeys1[i], c1);
			}

			_Bool c2 = (data2 >> i) & 1;

			if (c2 != ((lastdata2 >> i) & 1))
			{
				setKeyboardState(keyboardKeys2[i], c2);
			}
		}

		lastdata1 = data1;
		lastdata2 = data2;


		USBD_HID_SendReport(&hUsbDeviceFS, &keyboardhid, sizeof(keyboardhid));
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure. */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable the SYSCFG APB clock */
  __HAL_RCC_CRS_CLK_ENABLE();
  /** Configures CRS */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,1000);
  RCC_CRSInitStruct.ErrorLimitValue = 34;
  RCC_CRSInitStruct.HSI48CalibrationValue = 32;

  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}

static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 41;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);

	/*Configure GPIO pins : PA0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB8 */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PB1 */
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PF0 PF1 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : PA1 PA2 PA3 PA4 PA5
						   PA6 PA7 PA13 PA14 */
	GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5
						  |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_13|GPIO_PIN_14;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
