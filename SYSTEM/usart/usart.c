#include "sys.h"
#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//STM32F103ZE核心板
//串口1初始化		
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#if defined(__CC_ARM)
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	(void)x;
} 
#endif
//重定义fputc函数 
/*
 * Send one byte through USART1 for stdio retargeting.
 *
 * This keeps the legacy blocking printf path available while the new DMA
 * transmit API is idle. If a DMA transfer is active, the byte write waits
 * until the DMA path and USART shift register have finished to avoid mixing
 * blocking and DMA transmit data.
 *
 * Parameters:
 * ch: Character byte to transmit.
 * f: C library stream pointer, unused by this retarget.
 *
 * Return value:
 * The transmitted character value.
 *
 * Side effects:
 * Blocks until USART1 can accept and finish the byte.
 */
int fputc(int ch, FILE *f)
{
    (void)f;
    while (USART1_DMA_IsBusy() != 0)
    {
    }
    while ((USART1->SR & 0X40) == 0)
    {
    }
    USART1->DR = (u8)ch;
    return ch;
}
#endif 

#define USART1_TX_DMA_CHANNEL        DMA1_Channel4       // USART1 TX DMA channel.
#define USART1_TX_DMA_IRQ            DMA1_Channel4_IRQn   // USART1 TX DMA interrupt.
#define USART1_TX_DMA_CLEAR_IT       DMA1_IT_GL4          // Clears channel 4 pending bits.
#define USART1_TX_DMA_TC_IT          DMA1_IT_TC4          // Transfer complete interrupt.
#define USART1_TX_DMA_TE_IT          DMA1_IT_TE4          // Transfer error interrupt.

static u8 USART1_TX_DMA_DUMMY = 0;
static volatile u8 USART1_TX_DMA_BUSY = 0;

/*使用microLib的方法*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
/*
 * Initialize USART1 with interrupt receive and DMA transmit support.
 *
 * PA9 is configured as USART1_TX, PA10 is configured as USART1_RX, and
 * DMA1 Channel4 is prepared for memory-to-USART1 transmit requests. The
 * DMA channel remains disabled until USART1_DMA_Send starts a transfer.
 *
 * Parameters:
 * bound: USART1 baud rate.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Enables GPIOA, USART1, DMA1, USART1 RX interrupt, and DMA1 Channel4 IRQ.
 */
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  

  NVIC_InitStructure.NVIC_IRQChannel = USART1_TX_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  DMA_DeInit(USART1_TX_DMA_CHANNEL);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&USART1_TX_DMA_DUMMY;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(USART1_TX_DMA_CHANNEL, &DMA_InitStructure);
  DMA_ITConfig(USART1_TX_DMA_CHANNEL, DMA_IT_TC | DMA_IT_TE, ENABLE);
  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
  USART1_TX_DMA_BUSY = 0;

  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}

void USART1_IRQHandler(void)                	//串口1中断服务程序
	{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		Res =USART_ReceiveData(USART1);	//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)//接收未完成
			{
			if(USART_RX_STA&0x4000)//接收到了0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
				}
			else //还没收到0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
					}		 
				}
			}   		 
     } 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif	

/*
 * Start a non-blocking USART1 DMA transmit.
 *
 * The caller must keep the data buffer unchanged until USART1_DMA_IsBusy
 * returns 0. The function refuses a new transfer while DMA1 Channel4 is
 * already active, so bytes from independent callers are not interleaved.
 *
 * Parameters:
 * data: Pointer to the first byte to send.
 * len: Number of bytes to send, from 1 to 65535.
 *
 * Return value:
 * 1: Transfer was started.
 * 0: Parameters are invalid or a previous transfer is still active.
 *
 * Side effects:
 * Reprograms and enables DMA1 Channel4 for USART1_TX.
 */
u8 USART1_DMA_Send(const u8 *data, u16 len)
{
    if ((data == 0) || (len == 0) || (USART1_TX_DMA_BUSY != 0))
    {
        return 0;
    }

    USART1_TX_DMA_BUSY = 1;
    DMA_Cmd(USART1_TX_DMA_CHANNEL, DISABLE);
    DMA_ClearITPendingBit(USART1_TX_DMA_CLEAR_IT);
    USART1_TX_DMA_CHANNEL->CMAR = (u32)data;
    USART1_TX_DMA_CHANNEL->CNDTR = len;
    DMA_Cmd(USART1_TX_DMA_CHANNEL, ENABLE);

    return 1;
}

/*
 * Query whether USART1 DMA transmit is still busy.
 *
 * The DMA interrupt clears the software busy flag when the last byte has
 * been written into USART1->DR. This function also checks USART_FLAG_TC so
 * callers that need the line to be idle can wait until the final stop bit
 * has left the transmitter.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * 1: DMA or USART1 is still transmitting.
 * 0: USART1 transmit path is idle.
 */
u8 USART1_DMA_IsBusy(void)
{
    if ((USART1_TX_DMA_BUSY != 0) || (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET))
    {
        return 1;
    }

    return 0;
}

/*
 * Handle DMA1 Channel4 completion for USART1 transmit.
 *
 * USART1_TX uses DMA1 Channel4 on STM32F103. The handler disables the DMA
 * channel after transfer-complete or transfer-error status, clears all
 * pending channel flags, and releases the software busy state for the next
 * transfer request.
 *
 * Parameters:
 * None.
 *
 * Return value:
 * None.
 *
 * Side effects:
 * Disables DMA1 Channel4 and updates USART1_TX_DMA_BUSY.
 */
void DMA1_Channel4_IRQHandler(void)
{
    if ((DMA_GetITStatus(USART1_TX_DMA_TC_IT) != RESET) ||
        (DMA_GetITStatus(USART1_TX_DMA_TE_IT) != RESET))
    {
        DMA_Cmd(USART1_TX_DMA_CHANNEL, DISABLE);
        USART1_TX_DMA_BUSY = 0;
        DMA_ClearITPendingBit(USART1_TX_DMA_CLEAR_IT);
    }
}

