/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "usart.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <includes.h>
#include "stm32f1xx_hal.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* ��ʼ���� */
#define START_TASK_PRIO		3								//�������ȼ�
#define START_STK_SIZE 		64							//�����ջ��С
OS_TCB StartTaskTCB;											//������ƿ�
CPU_STK START_TASK_STK[START_STK_SIZE];		//�����ջ
void start_task(void *p_arg);							//����ִ�к���

/* LED0���� */
#define LED0_TASK_PRIO						4
#define LED0_STK_SIZE							64
OS_TCB Led0TaskTCB;
CPU_STK LED0_TASK_STK[LED0_STK_SIZE];
void led0_task(void *p_arg);

/* LED1���� */
#define LED1_TASK_PRIO						5
#define LED1_STK_SIZE							64
OS_TCB Led1TaskTCB;
CPU_STK LED1_TASK_STK[LED1_STK_SIZE];
void led1_task(void *p_arg);

/* ���ڷ������� */
#define UART_SEND_TASK_PRIO				3
#define UART_SEND_STK_SIZE				64
OS_TCB UartSendTaskTCB;
CPU_STK UART_SEND_TASK_STK[UART_SEND_STK_SIZE];
void uart_send_task(void *p_arg);

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /**Initializes the CPU, AHB and APB busses clocks 
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
  /**Initializes the CPU, AHB and APB busses clocks 
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

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	OS_ERR  err;
	OSInit(&err);										//��ʼ��UCOS-3
  HAL_Init();											//��ʼ��HAL��
	SystemClock_Config();						//ϵͳʱ������
	//MX_GPIO_Init(); �����BSP�ĳ�ʼ����Ҳ���ʼ��
  MX_USART1_UART_Init();					//���ڳ�ʼ��
	/* ������ʼ���� */
	OSTaskCreate((OS_TCB     *)&StartTaskTCB,   //������ƿ�             /* Create the start task                                */
				 (CPU_CHAR   *)"start task",					//��������
				 (OS_TASK_PTR ) start_task,						//������
				 (void       *) 0,										//���ݸ��������Ĳ���
				 (OS_PRIO     ) START_TASK_PRIO,			//�������ȼ�
				 (CPU_STK    *)&START_TASK_STK[0],		//�����ջ����ַ
				 (CPU_STK_SIZE) START_STK_SIZE/10,		//�����ջ�����λ
				 (CPU_STK_SIZE) START_STK_SIZE,				//�����ջ��С
				 (OS_MSG_QTY  ) 0,										//��ֹ������Ϣ
				 (OS_TICK     ) 0,										//ʱ��Ƭ����Ĭ��
				 (void       *) 0,										//�û�����Ĵ洢��
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);
	/* ����������ϵͳ������Ȩ����uC/OS-III */
	OSStart(&err);            /* Start multitasking (i.e. give control to uC/OS-III). */
               
}


void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();			//����һ������cpu_sr���ڴ洢CPU״̬�Ĵ����õ�
	p_arg = p_arg;
	
  BSP_Init();                                                   /* Initialize BSP functions */
  //CPU_Init();
  //Mem_Init();                                                 /* Initialize Memory Management Module */

#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  		//ͳ������                
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN			//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  		//��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//�����ٽ���

/* ����led0���� */
	OSTaskCreate((OS_TCB     *)&Led0TaskTCB,   	//������ƿ�             /* Create the start task                                */
				 (CPU_CHAR   *)"led0 task",						//��������
				 (OS_TASK_PTR ) led0_task,						//������
				 (void       *) 0,										//���ݸ��������Ĳ���
				 (OS_PRIO     ) LED0_TASK_PRIO,			//�������ȼ�
				 (CPU_STK    *)&LED0_TASK_STK[0],		//�����ջ����ַ
				 (CPU_STK_SIZE) LED0_STK_SIZE/10,		//�����ջ�����λ
				 (CPU_STK_SIZE) LED0_STK_SIZE,				//�����ջ��С
				 (OS_MSG_QTY  ) 0,										//��ֹ������Ϣ
				 (OS_TICK     ) 0,										//ʱ��Ƭ����Ĭ��
				 (void       *) 0,										//�û�����Ĵ洢��
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);
/* ����led1���� */
	OSTaskCreate((OS_TCB     *)&Led1TaskTCB,   	//������ƿ�             /* Create the start task                                */
				 (CPU_CHAR   *)"led1 task",						//��������
				 (OS_TASK_PTR ) led1_task,						//������
				 (void       *) 0,										//���ݸ��������Ĳ���
				 (OS_PRIO     ) LED1_TASK_PRIO,				//�������ȼ�
				 (CPU_STK    *)&LED1_TASK_STK[0],			//�����ջ����ַ
				 (CPU_STK_SIZE) LED1_STK_SIZE/10,			//�����ջ�����λ
				 (CPU_STK_SIZE) LED1_STK_SIZE,				//�����ջ��С
				 (OS_MSG_QTY  ) 0,										//��ֹ������Ϣ
				 (OS_TICK     ) 0,										//ʱ��Ƭ����Ĭ��
				 (void       *) 0,										//�û�����Ĵ洢��
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);
 /* �������ڷ������� */
	OSTaskCreate((OS_TCB     *)&UartSendTaskTCB,   	//������ƿ�             /* Create the start task                                */
				 (CPU_CHAR   *)"uart task",						//��������
				 (OS_TASK_PTR ) uart_send_task,						//������
				 (void       *) 0,										//���ݸ��������Ĳ���
				 (OS_PRIO     ) UART_SEND_TASK_PRIO,				//�������ȼ�
				 (CPU_STK    *)&UART_SEND_TASK_STK[0],			//�����ջ����ַ
				 (CPU_STK_SIZE) UART_SEND_STK_SIZE/10,			//�����ջ�����λ
				 (CPU_STK_SIZE) UART_SEND_STK_SIZE,				//�����ջ��С
				 (OS_MSG_QTY  ) 0,										//��ֹ������Ϣ
				 (OS_TICK     ) 0,										//ʱ��Ƭ����Ĭ��
				 (void       *) 0,										//�û�����Ĵ洢��
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);

	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//����ʼ����			 
	OS_CRITICAL_EXIT();	//�˳��ٽ���
}

void led0_task(void *p_arg)
{
	uint8_t cnt = 0;
	OS_ERR err;
	CPU_SR_ALLOC();			//����һ������cpu_sr���ڴ洢CPU״̬�Ĵ����õ�
	p_arg = p_arg;
	OS_CRITICAL_ENTER();
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	while(DEF_TRUE){
		if(++cnt == 8){
			printf("����LED1����ִ�д���%d\n", cnt/2);
			OSTaskSuspend((OS_TCB *)&Led1TaskTCB, &err);			//��������LED1
		}else if(cnt == 16){
			printf("����LED1�ָ���ִ�д���%d\n", cnt/2);
			OSTaskResume((OS_TCB *)&Led1TaskTCB, &err);				//�ָ�����LED0
		}
		printf("����LED0ִ��\n");
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
		OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
	}
	
}
void led1_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();			//����һ������cpu_sr���ڴ洢CPU״̬�Ĵ����õ�
	p_arg = p_arg;
	OS_CRITICAL_ENTER();
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	while(DEF_TRUE){
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
		printf("����LED1ִ��\n");
		OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_STRICT, &err);
	}
}
void uart_send_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();			//����һ������cpu_sr���ڴ洢CPU״̬�Ĵ����õ�
	p_arg = p_arg;
	OS_CRITICAL_ENTER();
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	while(DEF_TRUE){
		printf("UCOS-3 Hello World\n");
		OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_STRICT, &err);
	}
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
