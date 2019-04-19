/*
 * Copyright (c) 2018, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <stdint.h>
#include <modm/architecture/utils.hpp>
#include <modm/architecture/interface/assert.h>

// ----------------------------------------------------------------------------
void Undefined_Handler(void)
{
	uint32_t irqn;
	asm volatile("mrs %[irqn], ipsr" :[irqn] "=r" (irqn));
	modm_assert(0, "core", "nvic", "undefined", irqn);
}
/* Provide weak aliases for each Exception handler to Undefined_Handler.
 * As they are weak aliases, any function with the same name will override
 * this definition. */
void Reset_Handler(void)						__attribute__((noreturn));
void NMI_Handler(void)							__attribute__((weak, alias("Undefined_Handler")));
void HardFault_Handler(void)					__attribute__((weak, alias("Undefined_Handler")));
void MemManage_Handler(void)					__attribute__((weak, alias("Undefined_Handler")));
void BusFault_Handler(void)						__attribute__((weak, alias("Undefined_Handler")));
void UsageFault_Handler(void)					__attribute__((weak, alias("Undefined_Handler")));
void SVC_Handler(void)							__attribute__((weak, alias("Undefined_Handler")));
void DebugMon_Handler(void)						__attribute__((weak, alias("Undefined_Handler")));
void PendSV_Handler(void)						__attribute__((weak, alias("Undefined_Handler")));
void SysTick_Handler(void)						__attribute__((weak, alias("Undefined_Handler")));
void WWDG_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void PVD_PVM_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void TAMP_STAMP_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void RTC_WKUP_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void FLASH_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void RCC_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void EXTI0_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void EXTI1_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void EXTI2_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void EXTI3_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void EXTI4_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void DMA1_Channel1_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA1_Channel2_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA1_Channel3_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA1_Channel4_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA1_Channel5_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA1_Channel6_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA1_Channel7_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void ADC1_2_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void CAN1_TX_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void CAN1_RX0_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void CAN1_RX1_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void CAN1_SCE_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void EXTI9_5_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void TIM1_BRK_TIM15_IRQHandler(void)			__attribute__((weak, alias("Undefined_Handler")));
void TIM1_UP_TIM16_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void TIM1_TRG_COM_TIM17_IRQHandler(void)		__attribute__((weak, alias("Undefined_Handler")));
void TIM1_CC_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void TIM2_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void TIM3_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void TIM4_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void I2C1_EV_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void I2C1_ER_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void I2C2_EV_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void I2C2_ER_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void SPI1_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void SPI2_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void USART1_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void USART2_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void USART3_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void EXTI15_10_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void RTC_Alarm_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void DFSDM1_FLT3_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void TIM8_BRK_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void TIM8_UP_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void TIM8_TRG_COM_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void TIM8_CC_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void ADC3_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void FMC_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void SDMMC1_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void TIM5_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void SPI3_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void UART4_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void UART5_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void TIM6_DAC_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void TIM7_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void DMA2_Channel1_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA2_Channel2_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA2_Channel3_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA2_Channel4_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA2_Channel5_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DFSDM1_FLT0_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DFSDM1_FLT1_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DFSDM1_FLT2_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void COMP_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void LPTIM1_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void LPTIM2_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void OTG_FS_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void DMA2_Channel6_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void DMA2_Channel7_IRQHandler(void)				__attribute__((weak, alias("Undefined_Handler")));
void LPUART1_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void QUADSPI_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void I2C3_EV_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void I2C3_ER_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void SAI1_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void SAI2_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void SWPMI1_IRQHandler(void)					__attribute__((weak, alias("Undefined_Handler")));
void TSC_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void LCD_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void RNG_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
void FPU_IRQHandler(void)						__attribute__((weak, alias("Undefined_Handler")));
// ----------------------------------------------------------------------------
typedef void (* const FunctionPointer)(void);

// defined in the linkerscript
extern uint32_t __main_stack_top[];
extern uint32_t __process_stack_top[];

// Define the vector table
modm_section(".vector_rom")
FunctionPointer vectorsRom[] =
{
	(FunctionPointer)__main_stack_top,		// -16: stack pointer
	Reset_Handler,							// -15: code entry point
	NMI_Handler,							// -14: Non Maskable Interrupt handler
	HardFault_Handler,						// -13: hard fault handler
	MemManage_Handler,						// -12
	BusFault_Handler,						// -11
	UsageFault_Handler,						// -10
	Undefined_Handler,						//  -9
	Undefined_Handler,						//  -8
	Undefined_Handler,						//  -7
	Undefined_Handler,						//  -6
	SVC_Handler,							//  -5
	DebugMon_Handler,						//  -4
	Undefined_Handler,						//  -3
	PendSV_Handler,							//  -2
	SysTick_Handler,						//  -1
	WWDG_IRQHandler,						//   0
	PVD_PVM_IRQHandler,						//   1
	TAMP_STAMP_IRQHandler,					//   2
	RTC_WKUP_IRQHandler,					//   3
	FLASH_IRQHandler,						//   4
	RCC_IRQHandler,							//   5
	EXTI0_IRQHandler,						//   6
	EXTI1_IRQHandler,						//   7
	EXTI2_IRQHandler,						//   8
	EXTI3_IRQHandler,						//   9
	EXTI4_IRQHandler,						//  10
	DMA1_Channel1_IRQHandler,				//  11
	DMA1_Channel2_IRQHandler,				//  12
	DMA1_Channel3_IRQHandler,				//  13
	DMA1_Channel4_IRQHandler,				//  14
	DMA1_Channel5_IRQHandler,				//  15
	DMA1_Channel6_IRQHandler,				//  16
	DMA1_Channel7_IRQHandler,				//  17
	ADC1_2_IRQHandler,						//  18
	CAN1_TX_IRQHandler,						//  19
	CAN1_RX0_IRQHandler,					//  20
	CAN1_RX1_IRQHandler,					//  21
	CAN1_SCE_IRQHandler,					//  22
	EXTI9_5_IRQHandler,						//  23
	TIM1_BRK_TIM15_IRQHandler,				//  24
	TIM1_UP_TIM16_IRQHandler,				//  25
	TIM1_TRG_COM_TIM17_IRQHandler,			//  26
	TIM1_CC_IRQHandler,						//  27
	TIM2_IRQHandler,						//  28
	TIM3_IRQHandler,						//  29
	TIM4_IRQHandler,						//  30
	I2C1_EV_IRQHandler,						//  31
	I2C1_ER_IRQHandler,						//  32
	I2C2_EV_IRQHandler,						//  33
	I2C2_ER_IRQHandler,						//  34
	SPI1_IRQHandler,						//  35
	SPI2_IRQHandler,						//  36
	USART1_IRQHandler,						//  37
	USART2_IRQHandler,						//  38
	USART3_IRQHandler,						//  39
	EXTI15_10_IRQHandler,					//  40
	RTC_Alarm_IRQHandler,					//  41
	DFSDM1_FLT3_IRQHandler,					//  42
	TIM8_BRK_IRQHandler,					//  43
	TIM8_UP_IRQHandler,						//  44
	TIM8_TRG_COM_IRQHandler,				//  45
	TIM8_CC_IRQHandler,						//  46
	ADC3_IRQHandler,						//  47
	FMC_IRQHandler,							//  48
	SDMMC1_IRQHandler,						//  49
	TIM5_IRQHandler,						//  50
	SPI3_IRQHandler,						//  51
	UART4_IRQHandler,						//  52
	UART5_IRQHandler,						//  53
	TIM6_DAC_IRQHandler,					//  54
	TIM7_IRQHandler,						//  55
	DMA2_Channel1_IRQHandler,				//  56
	DMA2_Channel2_IRQHandler,				//  57
	DMA2_Channel3_IRQHandler,				//  58
	DMA2_Channel4_IRQHandler,				//  59
	DMA2_Channel5_IRQHandler,				//  60
	DFSDM1_FLT0_IRQHandler,					//  61
	DFSDM1_FLT1_IRQHandler,					//  62
	DFSDM1_FLT2_IRQHandler,					//  63
	COMP_IRQHandler,						//  64
	LPTIM1_IRQHandler,						//  65
	LPTIM2_IRQHandler,						//  66
	OTG_FS_IRQHandler,						//  67
	DMA2_Channel6_IRQHandler,				//  68
	DMA2_Channel7_IRQHandler,				//  69
	LPUART1_IRQHandler,						//  70
	QUADSPI_IRQHandler,						//  71
	I2C3_EV_IRQHandler,						//  72
	I2C3_ER_IRQHandler,						//  73
	SAI1_IRQHandler,						//  74
	SAI2_IRQHandler,						//  75
	SWPMI1_IRQHandler,						//  76
	TSC_IRQHandler,							//  77
	LCD_IRQHandler,							//  78
	Undefined_Handler,						//  79
	RNG_IRQHandler,							//  80
	FPU_IRQHandler,							//  81
};
