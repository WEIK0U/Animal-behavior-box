#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "string.h"
#include "adc.h"
#include "DS18B20.h"


void SystemClock_Config(void);

char wendu[30];
uint8_t DS18B20_Init(void);
short DS18B20_Get_Temperature(void);
float temperature;

//ph检测
uint32_t sensorValue, avgValue = 0;
uint16_t buf[10], temp;
float ph=0;

//蓝牙
uint8_t rx[11];
uint8_t blue_rx[19];

//16进制转10进制浮点数
float Hex_to_D(unsigned char byte_high, unsigned char byte_low)
{
	float d = 0;
	d += (byte_high/16)*4096 + (byte_high%16)*256;
	d += (byte_low/16)*16 + (byte_low%16)*1;	
	return d;
} 

unsigned char byteArry[4];//16进制数
//10进制浮点数到十六进制转换
void FloatToByte(float floatNum,unsigned char* byteArry)
{
    char* pchar=(char*)&floatNum;
    int i = 0;
    for( i=0;i<sizeof(float);i++)
    {
  *byteArry=*pchar;
  pchar++;
  byteArry++;
 
    }
}



//开始运行
void Cmd_run(uint8_t *buf)
{
		buf[0]=0xFE;
		buf[1]=0xFE;
		buf[2]=0x68;
		buf[3]=0xC0;
		buf[4]=0x68;
		buf[5]=0x02;
		buf[6]=0x0E;
		buf[7]=0xC0;
		buf[8]=0x01;
		buf[9]=0x01;
		buf[10]=0x00;
		buf[11]=0x00;
		buf[12]=0x00;
		buf[13]=0x80;
		buf[14]=0x3F;
		buf[15]=0x00;
		buf[16]=0x00;
		buf[17]=0xC8;
		buf[18]=0x42;
		buf[19]=0x05;
		buf[20]=0x00;
		buf[21]=0x30;
		buf[22]=0x16;
}
void Cmd_c8run(uint8_t *buf)
{
		buf[0]=0xFE;
		buf[1]=0xFE;
		buf[2]=0x68;
		buf[3]=0xC8;
		buf[4]=0x68;
		buf[5]=0x02;
		buf[6]=0x0E;
		buf[7]=0xC8;
		buf[8]=0x01;
		buf[9]=0x01;
		buf[10]=0x00;
		buf[11]=0x00;
		buf[12]=0x00;
		buf[13]=0x80;
		buf[14]=0x3F;
		buf[15]=0x00;
		buf[16]=0x00;
		buf[17]=0xC8;
		buf[18]=0x42;
		buf[19]=0x05;
		buf[20]=0x00;
		buf[21]=0x40;
		buf[22]=0x16;
}

//停止运行
void Cmd_stop(uint8_t *buf)
{
		buf[0]=0xFE;
		buf[1]=0xFE;
		buf[2]=0x68;
		buf[3]=0xC0;
		buf[4]=0x68;
		buf[5]=0x02;
		buf[6]=0x02;
		buf[7]=0xC0;
		buf[8]=0x07;
		buf[9]=0x5B;
		buf[10]=0x16;
}
void Cmd_c8stop(uint8_t *buf)
{
		buf[0]=0xFE;
		buf[1]=0xFE;
		buf[2]=0x68;
		buf[3]=0xC8;
		buf[4]=0x68;
		buf[5]=0x02;
		buf[6]=0x02;
		buf[7]=0xC8;
		buf[8]=0x07;
		buf[9]=0x6B;
		buf[10]=0x16;
}
//校验码
unsigned char verify(uint8_t *buf)
{
	int verify = 0;
	for(int i = 2; i<=20; i++)
	{
		verify += buf[i];
	}	
	verify = verify%256;	
	return (unsigned char)verify;
}
//电机反转控制
unsigned char reversal_control[1];

//运行指定速度
void Cmd_run_speed(uint8_t *buf)
{
	buf[0]=0xFE;
	buf[1]=0xFE;
	buf[2]=0x68;
	buf[3]=0xC0;
	buf[4]=0x68;
	buf[5]=0x02;
	buf[6]=0x0E;
	buf[7]=0xC0;
	buf[8]=0x01;
	buf[9]=0x01;
	buf[10]=reversal_control[0];
	buf[11]=0x00;
	buf[12]=0x00;
	buf[13]=0x80;
	buf[14]=0x3F;
	buf[15]=byteArry[0];
	buf[16]=byteArry[1];
	buf[17]=byteArry[2];
	buf[18]=byteArry[3];
	buf[19]=0x05;
	buf[20]=0x00;
	buf[21]=verify(buf);
	buf[22]=0x16;
}
void Cmd_c8run_speed(uint8_t *buf)
{
	buf[0]=0xFE;
	buf[1]=0xFE;
	buf[2]=0x68;
	buf[3]=0xC8;
	buf[4]=0x68;
	buf[5]=0x02;
	buf[6]=0x0E;
	buf[7]=0xC8;
	buf[8]=0x01;
	buf[9]=0x01;
	buf[10]=reversal_control[0];
	buf[11]=0x00;
	buf[12]=0x00;
	buf[13]=0x80;
	buf[14]=0x3F;
	buf[15]=byteArry[0];
	buf[16]=byteArry[1];
	buf[17]=byteArry[2];
	buf[18]=byteArry[3];
	buf[19]=0x05;
	buf[20]=0x00;
	buf[21]=verify(buf);
	buf[22]=0x16;

}
uint8_t rs485buf_run[23];
uint8_t rs485buf_stop[11];

uint16_t uart_change[2];
int flag_uart = 0;
int a=1;
double adc2_value=0;
char shuiwei[20];
//void adc_proc()
//{
//	HAL_ADC_Start(&hadc2);
//	adc2_value=(3.3*HAL_ADC_GetValue(&hadc2))/4096;
//	
//	if(adc2_value>1)
//	{
//		sprintf(shuiwei,"shuiwei =%.2fmm  ok \r",adc2_value);
//		HAL_UART_Transmit(&hlpuart1, (uint8_t*)shuiwei, strlen(shuiwei), 1000);
//	}
//}


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
	MX_LPUART1_UART_Init();
	MX_UART4_Init();
	MX_UART5_Init();
	MX_USART1_UART_Init();
	MX_USART3_UART_Init();
	HAL_UART_Receive_IT(&huart4, rx, 11);
	HAL_UART_Receive_IT(&huart3, blue_rx, 19);
	MX_ADC2_Init();
	MX_ADC1_Init();
		
  while (1)
  {
    /* USER CODE END WHILE */		
	//	adc_proc();
	  temperature = DS18B20_Get_Temperature();
		if(temperature < 0){
		sprintf(wendu,"temperature = -%.2f degree \r",temperature/10);
		HAL_UART_Transmit(&hlpuart1, (uint8_t*)wendu, strlen(wendu), 1000);}
		else
		{
		sprintf(wendu,"temperature = %.2f degree \r",temperature/10);
		HAL_UART_Transmit(&hlpuart1, (uint8_t*)wendu, strlen(wendu), 1000);
		HAL_Delay(200);
		}
		HAL_ADC_Start(&hadc2);
	  for (int i = 0; i < 10; i++) //Get 10 sample value from the sensor for smooth the value
			{
				//HAL_ADC_Start(&hadc1);
				//HAL_ADC_PollForConversion(&hadc1, 100);
				sensorValue = HAL_ADC_GetValue(&hadc2);
				buf[i] = sensorValue;
				//HAL_ADC_Stop(&hadc1);
				HAL_Delay(10);
			}

			HAL_ADC_Stop(&hadc2);

			for (int i = 0; i < 9; i++) //sort the analog from small to large
			{
				for (int j = i + 1; j < 10; j++)
				{
					if (buf[i] > buf[j])
					{
						temp = buf[i];
						buf[i] = buf[j];
						buf[j] = temp;
					}
				}
			}
			avgValue = 0;
			for (int i = 2; i < 8; i++) //take the average value of 6 center sample
			avgValue += buf[i];
			float phValue = (float)avgValue * 5.0 / 4096 / 6; //convert the analog into millivolt
//			phValue = 1.7 * phValue+6.5;                          //convert the millivolt into pH value
//		//	ph=-2.22*phValue+11.7;

			char pHStr[10];
			sprintf(pHStr, "phv:%0.2f\r\n",phValue);
			HAL_UART_Transmit(&hlpuart1, (uint8_t*)pHStr, strlen(pHStr), 1000);		
//			sprintf(pHStr, "ph:%0.2f\r\n",ph);
//			HAL_UART_Transmit(&hlpuart1, (uint8_t*)pHStr, strlen(pHStr), 1000);		
			
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{	
	
	if(huart==&huart4)
	{
		if((rx[0]==0xf1)&&(rx[6]==0xff)&&(rx[7]==0xff)&&(rx[8]==0x00)&&(rx[9]==0xff)&&(rx[10]==0xff))
		{
		switch(rx[1])
			{
				//c0电机的正反转
				case 0x01://使电机反转
				reversal_control[0]=0x01;
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_run_speed(rs485buf_run);		
//			RS485_Send_Data(rs485buf_run,23);
				HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
				break;
				case 0x02://使电机正转
				reversal_control[0]=0x00;
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_run_speed(rs485buf_run);		
				HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
				break;
				
				//c8电机的正反转
				case 0x0A://使电机反转
				reversal_control[0]=0x01;
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_c8run_speed(rs485buf_run);		
				HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
				break;
				case 0x0B://使电机正转
				reversal_control[0]=0x00;
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_c8run_speed(rs485buf_run);		
				HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
				break;
				
				//c0电机启动和停止
				case 0x07://电机启动
					Cmd_run(rs485buf_run);		
					HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);//发送23个字节
				break;
				case 0x08://电机停止
					Cmd_stop(rs485buf_stop);		
					HAL_UART_Transmit(&huart1,rs485buf_stop,11,1000);//发送11个字节
				break;
				
				//c8电机启动和停止
				case 0x0C://电机启动
					Cmd_c8run(rs485buf_run);		
					HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);//发送23个字节
				break;
				case 0x0D://电机停止
					Cmd_c8stop(rs485buf_stop);		
					HAL_UART_Transmit(&huart1,rs485buf_stop,11,1000);//发送11个字节
				break;
				
				//c0电机速度控制
				case 0x03://输入框设置速度
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_run_speed(rs485buf_run);		
				HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);	
				break;
				case 0x06://滑块控制速度
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_run_speed(rs485buf_run);		
				HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
				break;
				
				//c8电机速度控制	
				case 0x0E://输入框设置速度
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_c8run_speed(rs485buf_run);		
				HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
				break;
				case 0x0F://滑块控制速度
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_c8run_speed(rs485buf_run);		
				HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
			}	
		}	
	HAL_UART_Receive_IT(&huart4, rx, 11);
	}
	
	if(huart==&huart3)
	{	
		//if((blue_rx[0]==0xA5)&&(blue_rx[18]==0x5A))
		//{
				if((blue_rx[3]==1) || (blue_rx[4]==1))
				{
					if(blue_rx[3]==1)
					{
						Cmd_stop(rs485buf_stop);		
						HAL_UART_Transmit(&huart1,rs485buf_stop,11,1000);//发送11个字节
					}				
					else if(blue_rx[4]==1)
					{
						Cmd_c8stop(rs485buf_stop);		
						HAL_UART_Transmit(&huart1,rs485buf_stop,11,1000);//发送11个字节
					}
				}			
				
				
				
				else if((blue_rx[3]==0) || (blue_rx[4]==0))
				{	
					if(flag_uart == 0)
					{
						uart_change[0] = (blue_rx[6]<<4) + blue_rx[5];
						uart_change[1] = (blue_rx[8]<<4) + blue_rx[7];
						flag_uart = 1;
					}
					
					else
					{
						if(uart_change[0] != ((blue_rx[6]<<4) + blue_rx[5]))
						{
							if(blue_rx[3]==0)
							{
								//c0正转
								if(blue_rx[1]==1)
								{
									reversal_control[0]=0x00;
									FloatToByte(Hex_to_D(blue_rx[6], blue_rx[5]),byteArry);
									Cmd_run_speed(rs485buf_run);		
									HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
								}
								else if(blue_rx[1]==0)
								{
									reversal_control[0]=0x01;
									FloatToByte(Hex_to_D(blue_rx[6], blue_rx[5]),byteArry);
									Cmd_run_speed(rs485buf_run);		
									HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
								}								
							}
						
							uart_change[0] = (blue_rx[6]<<4) + blue_rx[5];
						}
							
						else if(uart_change[1] != ((blue_rx[8]<<4) + blue_rx[7]))
						{				
							if(blue_rx[4]==0)
							{
								if(blue_rx[2]==1)
								{
									reversal_control[0]=0x00;
									FloatToByte(Hex_to_D(blue_rx[8], blue_rx[7]),byteArry);
									Cmd_c8run_speed(rs485buf_run);		
									HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
								}
								else if(blue_rx[2]==0)
								{
									reversal_control[0]=0x01;
									FloatToByte(Hex_to_D(blue_rx[8], blue_rx[7]),byteArry);
									Cmd_c8run_speed(rs485buf_run);		
									HAL_UART_Transmit(&huart1,rs485buf_run,23,1000);
								}								
							}							
						uart_change[1] = (blue_rx[8]<<4) + blue_rx[7];
						}
						
						/*
						else
						{
							if(blue_rx[3]==0)
							{
								//c0正转
								if(blue_rx[1]==1)
								{
									reversal_control[0]=0x00;
									FloatToByte(Hex_to_D(blue_rx[6], blue_rx[5]),byteArry);
									Cmd_run_speed(rs485buf_run);		
									RS485_Send_Data(rs485buf_run,23);	
								}
								else if(blue_rx[1]==0)
								{
									reversal_control[0]=0x01;
									FloatToByte(Hex_to_D(blue_rx[6], blue_rx[5]),byteArry);
									Cmd_run_speed(rs485buf_run);		
									RS485_Send_Data(rs485buf_run,23);
								}	
								
							}
							
							if(blue_rx[4]==0)
							{
							if(blue_rx[2]==1)
							{
								reversal_control[0]=0x00;
								FloatToByte(Hex_to_D(blue_rx[8], blue_rx[7]),byteArry);
								Cmd_c8run_speed(rs485buf_run);		
								RS485_Send_Data(rs485buf_run,23);	
							}
							else if(blue_rx[2]==0)
							{
								reversal_control[0]=0x01;
								FloatToByte(Hex_to_D(blue_rx[8], blue_rx[7]),byteArry);
								Cmd_c8run_speed(rs485buf_run);		
								RS485_Send_Data(rs485buf_run,23);
							}				
									
							}
						}
						*/
					
					}
//					if(blue_rx[3]==0)
//					{
//						//c0正转
//						if(blue_rx[1]==1)
//						{
//							reversal_control[0]=0x00;
//							FloatToByte(Hex_to_D(blue_rx[6], blue_rx[5]),byteArry);
//							Cmd_run_speed(rs485buf_run);		
//							RS485_Send_Data(rs485buf_run,23);	
//						}
//						else if(blue_rx[1]==0)
//						{
//							reversal_control[0]=0x01;
//							FloatToByte(Hex_to_D(blue_rx[6], blue_rx[5]),byteArry);
//							Cmd_run_speed(rs485buf_run);		
//							RS485_Send_Data(rs485buf_run,23);
//						}								
//					}
						
//					if(blue_rx[4]==0)
//					{
//						if(blue_rx[2]==1)
//						{
//							reversal_control[0]=0x00;
//							FloatToByte(Hex_to_D(blue_rx[8], blue_rx[7]),byteArry);
//							Cmd_c8run_speed(rs485buf_run);		
//							RS485_Send_Data(rs485buf_run,23);	
//						}
//						else if(blue_rx[2]==0)
//						{
//							reversal_control[0]=0x01;
//							FloatToByte(Hex_to_D(blue_rx[8], blue_rx[7]),byteArry);
//							Cmd_c8run_speed(rs485buf_run);		
//							RS485_Send_Data(rs485buf_run,23);
//						}								
//					}
				}
				
				//c8
//				if(blue_rx[4]==1)
//				{
//					Cmd_c8stop(rs485buf_stop);		
//					RS485_Send_Data(rs485buf_stop,11);//发送11个字节
//				}			
//				else if (blue_rx[4]==0)
//				{	
//					if(blue_rx[2]==1)
//					{
//					reversal_control[0]=0x00;
//					FloatToByte(Hex_to_D(blue_rx[8], blue_rx[7]),byteArry);
//					Cmd_c8run_speed(rs485buf_run);		
//					RS485_Send_Data(rs485buf_run,23);
//					}
//					else if(blue_rx[2]==0)
//					{
//					reversal_control[0]=0x01;
//					FloatToByte(Hex_to_D(blue_rx[8], blue_rx[7]),byteArry);
//					Cmd_c8run_speed(rs485buf_run);		
//					RS485_Send_Data(rs485buf_run,23);
//					}					
//				}	
				
				
		//}
		HAL_UART_Receive_IT(&huart3, blue_rx, 19);
	}

}



/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV3;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_UART4
                              |RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_LPUART1
                              |RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
