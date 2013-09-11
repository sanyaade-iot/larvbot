/*
 * processor.c
 *
 *  Created on: Sep 7, 2013
 *      Author: petera
 */
#include "system.h"
#include "gpio.h"

static void rcc_config(void)
{
  RCC_PCLK1Config(RCC_HCLK_Div2); // Clock APB1 with AHB clock / 2

  // SRAM
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

  // TIM2
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

#ifdef CONFIG_UART
#ifdef CONFIG_UART2
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif
#ifdef CONFIG_UART4
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
#endif
#endif
#ifdef CONFIG_I2C
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
#endif
}

static void nvic_config(void)
{
  // Configure the NVIC Preemption Priority Bits.
  // preempt prios 0..3, subpriorities  0..3
  uint32_t prioGrp = (8-__NVIC_PRIO_BITS) + (2-1); // priogroup 5 for stm32f4xx
  NVIC_SetPriorityGrouping(prioGrp);

  //NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(prioGrp, 0, 0));
  //NVIC_EnableIRQ(SysTick_IRQn);

  // enable TIM2 interrupt, supahigh
  NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(prioGrp, 0, 0));
  NVIC_EnableIRQ(TIM2_IRQn);

#ifdef CONFIG_USB_VCD
  NVIC_SetPriority(OTG_FS_IRQn, NVIC_EncodePriority(prioGrp, 2, 0));
#endif

#ifdef CONFIG_UART
#ifdef CONFIG_UART2
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(prioGrp, 3, 0));
  NVIC_EnableIRQ(USART2_IRQn);
#endif
#endif

#ifdef CONFIG_UART
#ifdef CONFIG_UART2
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(prioGrp, 3, 0));
  NVIC_EnableIRQ(USART2_IRQn);
#endif
#ifdef CONFIG_UART2
  NVIC_SetPriority(UART4_IRQn, NVIC_EncodePriority(prioGrp, 3, 0));
  NVIC_EnableIRQ(UART4_IRQn);
#endif
#endif

#ifdef CONFIG_I2C
  NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(prioGrp, 1, 1));
  NVIC_EnableIRQ(I2C1_EV_IRQn);
  NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(prioGrp, 1, 1));
  NVIC_EnableIRQ(I2C1_ER_IRQn);
#endif
}

static void dbgmcu_config(void)
{
  // keep jtag clock running during WFI instruction -> STANDBY_MODE
  DBGMCU_Config(DBGMCU_STANDBY, ENABLE);
  DBGMCU_Config(DBGMCU_SLEEP | DBGMCU_STOP, DISABLE);
}

static void sram_config() {
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  p;

/*-- GPIOs Configuration -----------------------------------------------------*/
/*
 +-------------------+--------------------+------------------+------------------+
 | PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0 <-> FSMC_A0  | PG0 <-> FSMC_A10 |
 | PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1 <-> FSMC_A1  | PG1 <-> FSMC_A11 |
 | PD4  <-> FSMC_NOE | PE2  <-> FSMC_A23  | PF2 <-> FSMC_A2  | PG2 <-> FSMC_A12 |
 | PD5  <-> FSMC_NWE | PE3  <-> FSMC_A19  | PF3 <-> FSMC_A3  | PG3 <-> FSMC_A13 |
 | PD8  <-> FSMC_D13 | PE4  <-> FSMC_A20  | PF4 <-> FSMC_A4  | PG4 <-> FSMC_A14 |
 | PD9  <-> FSMC_D14 | PE5  <-> FSMC_A21  | PF5 <-> FSMC_A5  | PG5 <-> FSMC_A15 |
 | PD10 <-> FSMC_D15 | PE6  <-> FSMC_A22  | PF12 <-> FSMC_A6 | PG9 <-> FSMC_NE2 |
 | PD11 <-> FSMC_A16 | PE7  <-> FSMC_D4   | PF13 <-> FSMC_A7 |------------------+
 | PD12 <-> FSMC_A17 | PE8  <-> FSMC_D5   | PF14 <-> FSMC_A8 |
 | PD13 <-> FSMC_A18 | PE9  <-> FSMC_D6   | PF15 <-> FSMC_A9 |
 | PD14 <-> FSMC_D0  | PE10 <-> FSMC_D7   |------------------+
 | PD15 <-> FSMC_D1  | PE11 <-> FSMC_D8   |
 +-------------------| PE12 <-> FSMC_D9   |
                     | PE13 <-> FSMC_D10  |
                     | PE14 <-> FSMC_D11  |
                     | PE15 <-> FSMC_D12  |
                     +--------------------+
*/


  // AF12 = GPIO_AF_FSMC
  io_config(PORTD, PIN0, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN1, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN4, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN5, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN8, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN9, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN10, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN11, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN12, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN13, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN14, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTD, PIN15, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);

  io_config(PORTE, PIN0, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN1, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN2, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN3, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN4, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN5, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN6, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN7, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN8, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN9, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN10, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN11, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN12, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN13, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN14, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTE, PIN15, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);

  io_config(PORTF, PIN0, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTF, PIN1, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTF, PIN2, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTF, PIN3, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTF, PIN4, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTF, PIN5, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTF, PIN12, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTF, PIN13, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTF, PIN14, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTF, PIN15, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);

  io_config(PORTG, PIN0, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTG, PIN1, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTG, PIN2, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTG, PIN3, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTG, PIN4, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTG, PIN5, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);
  io_config(PORTG, PIN9, IO_100MHZ, AF, AF12, PUSHPULL, NOPULL);

  // 1 / 168MHz ~= 6ns
  // Maximum FSMC_CLK frequency for synchronous accesses is 60 MHz.

  p.FSMC_AddressSetupTime = 0x3;      // 0x0..0xf
  p.FSMC_AddressHoldTime = 0x0;       // 0x0..0xf
  p.FSMC_DataSetupTime = 0x06;        // 0x00..0xff
  p.FSMC_BusTurnAroundDuration = 0x1; // 0x0..0xf

  p.FSMC_CLKDivision = 0;             // not used in sram
  p.FSMC_DataLatency = 0;             // not used in sram
  p.FSMC_AccessMode = FSMC_AccessMode_A;

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM2;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

  /*!< Enable FSMC Bank1_SRAM1 Bank */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM2, ENABLE);
}

static void usb_config(void) {
#ifdef CONFIG_USB_VCD
  // AF10 = GPIO_AF_OTG_FS
  io_config_in(PORTA, PIN9, IO_25MHZ);
  io_config(PORTA, PIN10, IO_100MHZ, AF, AF10, PUSHPULL, NOPULL);
  io_config(PORTA, PIN11, IO_100MHZ, AF, AF10, PUSHPULL, NOPULL);
  io_config(PORTA, PIN12, IO_100MHZ, AF, AF10, PUSHPULL, NOPULL);
#endif
}

static void uart_config(void) {
#ifdef CONFIG_UART
  USART_InitTypeDef USART_InitStructure;
#ifdef CONFIG_UART2
  // AF7 = GPIO_AF_USART2
  // tx
  io_config(PORTA, PIN2, IO_50MHZ, AF, AF7, PUSHPULL, NOPULL);
  // rx
  io_config(PORTA, PIN3, IO_50MHZ, AF, AF7, PUSHPULL, NOPULL);

  // USART configuration
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);

  // Enable USART interrupt
  USART_ITConfig(USART2, USART_IT_TC, DISABLE);
  USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

  // Enable USART
  USART_Cmd(USART2, ENABLE);
#endif
#ifdef CONFIG_UART4
  // AF8 = GPIO_AF_UART4
  // tx
  io_config(PORTA, PIN0, IO_50MHZ, AF, AF8, PUSHPULL, NOPULL);
  // rx
  io_config(PORTA, PIN1, IO_50MHZ, AF, AF8, PUSHPULL, NOPULL);

  // USART configuration
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(UART4, &USART_InitStructure);

  // Enable USART interrupt
  USART_ITConfig(UART4, USART_IT_TC, DISABLE);
  USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
  USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);

  // Enable USART
  USART_Cmd(UART4, ENABLE);
#endif
#endif
}

static void i2c_config(void) {
#ifdef CONFIG_I2C
  // AF4 = GPIO_AF_I2C1
  // scl
  io_config(PORTB, PIN6, IO_50MHZ, AF, AF4, OPENDRAIN, NOPULL);
  // sda
  io_config(PORTB, PIN7, IO_50MHZ, AF, AF4, OPENDRAIN, NOPULL);
#endif
}

static void tim_config(void) {
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  u16_t prescaler = 0;

  // Time base configuration
  TIM_TimeBaseStructure.TIM_Period = SystemCoreClock/SYS_MAIN_TIMER_FREQ;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  // Prescaler configuration
  TIM_PrescalerConfig(TIM2, prescaler, TIM_PSCReloadMode_Immediate);

  // TIM IT enable
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  // TIM2 enable counter
  TIM_Cmd(TIM2, ENABLE);
}


void PROC_init() {
  rcc_config();
  nvic_config();
  dbgmcu_config();
  io_init();
  sram_config();
  usb_config();
  uart_config();
  i2c_config();
  tim_config();
}