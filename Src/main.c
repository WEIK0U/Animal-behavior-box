#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "rs485.h"
#include "delay.h"
#include "stdio.h"
#include "string.h"
#include "adc.h"

void SystemClock_Config(void);

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
void Cmd_run(u8 *buf)
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
void Cmd_c8run(u8 *buf)
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
void Cmd_stop(u8 *buf)
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
void Cmd_c8stop(u8 *buf)
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
unsigned char verify(u8 *buf)
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
void Cmd_run_speed(u8 *buf)
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
void Cmd_c8run_speed(u8 *buf)
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
u8 rs485buf_run[23];
u8 rs485buf_stop[11];

uint16_t uart_change[2];
int flag_uart = 0;

int main(void)
{
	

	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_USART3_UART_Init();
	MX_USART1_UART_Init();
	MX_ADC1_Init();
	HAL_UART_Receive_IT(&huart3, rx, 11);
	HAL_UART_Receive_IT(&huart4, blue_rx, 19);
	RS485_Init(9600);	
	

	
	
  while (1)
  {
    /* USER CODE END WHILE */
		
		

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{	
	
	if(huart==&huart3)
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
				RS485_Send_Data(rs485buf_run,23);
				break;
				case 0x02://使电机正转
				reversal_control[0]=0x00;
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_run_speed(rs485buf_run);		
				RS485_Send_Data(rs485buf_run,23);
				break;
				
				//c8电机的正反转
				case 0x0A://使电机反转
				reversal_control[0]=0x01;
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_c8run_speed(rs485buf_run);		
				RS485_Send_Data(rs485buf_run,23);
				break;
				case 0x0B://使电机正转
				reversal_control[0]=0x00;
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_c8run_speed(rs485buf_run);		
				RS485_Send_Data(rs485buf_run,23);
				break;
				
				//c0电机启动和停止
				case 0x07://电机启动
					Cmd_run(rs485buf_run);		
					RS485_Send_Data(rs485buf_run,23);//发送23个字节
				break;
				case 0x08://电机停止
					Cmd_stop(rs485buf_stop);		
					RS485_Send_Data(rs485buf_stop,11);//发送11个字节
				break;
				
				//c8电机启动和停止
				case 0x0C://电机启动
					Cmd_c8run(rs485buf_run);		
					RS485_Send_Data(rs485buf_run,23);//发送23个字节
				break;
				case 0x0D://电机停止
					Cmd_c8stop(rs485buf_stop);		
					RS485_Send_Data(rs485buf_stop,11);//发送11个字节
				break;
				
				//c0电机速度控制
				case 0x03://输入框设置速度
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_run_speed(rs485buf_run);		
				RS485_Send_Data(rs485buf_run,23);	
				break;
				case 0x06://滑块控制速度
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_run_speed(rs485buf_run);		
				RS485_Send_Data(rs485buf_run,23);	
				break;
				
				//c8电机速度控制	
				case 0x0E://输入框设置速度
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_c8run_speed(rs485buf_run);		
				RS485_Send_Data(rs485buf_run,23);	
				break;
				case 0x0F://滑块控制速度
				FloatToByte(Hex_to_D(rx[3], rx[2]),byteArry);
				Cmd_c8run_speed(rs485buf_run);		
				RS485_Send_Data(rs485buf_run,23);	
				break;						
			}	
		}	
	HAL_UART_Receive_IT(&huart3, rx, 11);
	}
	

	
	if(huart==&huart4)
	{

			
		//if((blue_rx[0]==0xA5)&&(blue_rx[18]==0x5A))
		//{
				if((blue_rx[3]==1) || (blue_rx[4]==1))
				{
					if(blue_rx[3]==1)
					{
						Cmd_stop(rs485buf_stop);		
						RS485_Send_Data(rs485buf_stop,11);//发送11个字节
					}
					
					if(blue_rx[4]==1)
					{
						Cmd_c8stop(rs485buf_stop);		
						RS485_Send_Data(rs485buf_stop,11);//发送11个字节
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
		HAL_UART_Receive_IT(&huart4, blue_rx, 19);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
