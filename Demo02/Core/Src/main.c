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

/* 开始任务 */
#define START_TASK_PRIO		3								//任务优先级
#define START_STK_SIZE 		64							//任务堆栈大小
OS_TCB StartTaskTCB;											//任务控制块
CPU_STK START_TASK_STK[START_STK_SIZE];		//任务堆栈
void start_task(void *p_arg);							//任务执行函数

/* LED0任务 */
#define LED0_TASK_PRIO						4
#define LED0_STK_SIZE							64
OS_TCB Led0TaskTCB;
CPU_STK LED0_TASK_STK[LED0_STK_SIZE];
void led0_task(void *p_arg);

/* LED1任务 */
#define LED1_TASK_PRIO						5
#define LED1_STK_SIZE							64
OS_TCB Led1TaskTCB;
CPU_STK LED1_TASK_STK[LED1_STK_SIZE];
void led1_task(void *p_arg);

/* 串口发送任务 */
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
	OSInit(&err);										//初始化UCOS-3
  HAL_Init();											//初始化HAL库
	SystemClock_Config();						//系统时钟配置
	//MX_GPIO_Init(); 这个在BSP的初始化里也会初始化
  MX_USART1_UART_Init();					//串口初始化
	/* 创建开始任务 */
	OSTaskCreate((OS_TCB     *)&StartTaskTCB,   //任务控制块             /* Create the start task                                */
				 (CPU_CHAR   *)"start task",					//任务名字
				 (OS_TASK_PTR ) start_task,						//任务函数
				 (void       *) 0,										//传递给任务函数的参数
				 (OS_PRIO     ) START_TASK_PRIO,			//任务优先级
				 (CPU_STK    *)&START_TASK_STK[0],		//任务堆栈基地址
				 (CPU_STK_SIZE) START_STK_SIZE/10,		//任务堆栈深度限位
				 (CPU_STK_SIZE) START_STK_SIZE,				//任务堆栈大小
				 (OS_MSG_QTY  ) 0,										//禁止接收消息
				 (OS_TICK     ) 0,										//时间片长度默认
				 (void       *) 0,										//用户补充的存储区
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);
	/* 启动多任务系统，控制权交给uC/OS-III */
	OSStart(&err);            /* Start multitasking (i.e. give control to uC/OS-III). */
               
}


void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();			//定义一个变量cpu_sr用于存储CPU状态寄存器用的
	p_arg = p_arg;
	
  BSP_Init();                                                   /* Initialize BSP functions */
  //CPU_Init();
  //Mem_Init();                                                 /* Initialize Memory Management Module */

#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  		//统计任务                
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN			//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  		//当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//进入临界区

/* 创建led0任务 */
	OSTaskCreate((OS_TCB     *)&Led0TaskTCB,   	//任务控制块             /* Create the start task                                */
				 (CPU_CHAR   *)"led0 task",						//任务名字
				 (OS_TASK_PTR ) led0_task,						//任务函数
				 (void       *) 0,										//传递给任务函数的参数
				 (OS_PRIO     ) LED0_TASK_PRIO,			//任务优先级
				 (CPU_STK    *)&LED0_TASK_STK[0],		//任务堆栈基地址
				 (CPU_STK_SIZE) LED0_STK_SIZE/10,		//任务堆栈深度限位
				 (CPU_STK_SIZE) LED0_STK_SIZE,				//任务堆栈大小
				 (OS_MSG_QTY  ) 0,										//禁止接收消息
				 (OS_TICK     ) 0,										//时间片长度默认
				 (void       *) 0,										//用户补充的存储区
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);
/* 创建led1任务 */
	OSTaskCreate((OS_TCB     *)&Led1TaskTCB,   	//任务控制块             /* Create the start task                                */
				 (CPU_CHAR   *)"led1 task",						//任务名字
				 (OS_TASK_PTR ) led1_task,						//任务函数
				 (void       *) 0,										//传递给任务函数的参数
				 (OS_PRIO     ) LED1_TASK_PRIO,				//任务优先级
				 (CPU_STK    *)&LED1_TASK_STK[0],			//任务堆栈基地址
				 (CPU_STK_SIZE) LED1_STK_SIZE/10,			//任务堆栈深度限位
				 (CPU_STK_SIZE) LED1_STK_SIZE,				//任务堆栈大小
				 (OS_MSG_QTY  ) 0,										//禁止接收消息
				 (OS_TICK     ) 0,										//时间片长度默认
				 (void       *) 0,										//用户补充的存储区
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);
 /* 创建串口发送任务 */
	OSTaskCreate((OS_TCB     *)&UartSendTaskTCB,   	//任务控制块             /* Create the start task                                */
				 (CPU_CHAR   *)"uart task",						//任务名字
				 (OS_TASK_PTR ) uart_send_task,						//任务函数
				 (void       *) 0,										//传递给任务函数的参数
				 (OS_PRIO     ) UART_SEND_TASK_PRIO,				//任务优先级
				 (CPU_STK    *)&UART_SEND_TASK_STK[0],			//任务堆栈基地址
				 (CPU_STK_SIZE) UART_SEND_STK_SIZE/10,			//任务堆栈深度限位
				 (CPU_STK_SIZE) UART_SEND_STK_SIZE,				//任务堆栈大小
				 (OS_MSG_QTY  ) 0,										//禁止接收消息
				 (OS_TICK     ) 0,										//时间片长度默认
				 (void       *) 0,										//用户补充的存储区
				 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR     *)&err);

	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//退出临界区
}

void led0_task(void *p_arg)
{
	uint8_t cnt = 0;
	OS_ERR err;
	CPU_SR_ALLOC();			//定义一个变量cpu_sr用于存储CPU状态寄存器用的
	p_arg = p_arg;
	OS_CRITICAL_ENTER();
	OS_CRITICAL_EXIT();	//退出临界区
	while(DEF_TRUE){
		if(++cnt == 8){
			printf("任务LED1挂起，执行次数%d\n", cnt/2);
			OSTaskSuspend((OS_TCB *)&Led1TaskTCB, &err);			//挂起任务LED1
		}else if(cnt == 16){
			printf("任务LED1恢复，执行次数%d\n", cnt/2);
			OSTaskResume((OS_TCB *)&Led1TaskTCB, &err);				//恢复任务LED0
		}
		printf("任务LED0执行\n");
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_8);
		OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err);
	}
	
}
void led1_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();			//定义一个变量cpu_sr用于存储CPU状态寄存器用的
	p_arg = p_arg;
	OS_CRITICAL_ENTER();
	OS_CRITICAL_EXIT();	//退出临界区
	while(DEF_TRUE){
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_2);
		printf("任务LED1执行\n");
		OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_STRICT, &err);
	}
}
void uart_send_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();			//定义一个变量cpu_sr用于存储CPU状态寄存器用的
	p_arg = p_arg;
	OS_CRITICAL_ENTER();
	OS_CRITICAL_EXIT();	//退出临界区
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
