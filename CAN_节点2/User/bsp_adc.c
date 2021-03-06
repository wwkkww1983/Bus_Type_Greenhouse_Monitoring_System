/**
  ******************************************************************************
  * @file    bsp_adc.c
  * @author  闫金崟
  * @version V1.2
  * @date    2016-4-24
  * @brief   模数转换配置文件
  ******************************************************************************
  * @attention
  * 使用了ADC1的12、13通道采集，通过DMA1传输
  ******************************************************************************
  */


#include "bsp_adc.h"

#define ADC1_DR_Address    ((u32)0x40012400+0x4c)

__IO uint16_t ADC_ConvertedValue[2];

static void ADC1_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);                              //开DMA1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);     //开ADC1和PC时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;                                       //选PC2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                                   //模拟输入模式
	GPIO_Init(GPIOC, &GPIO_InitStructure);				                                  //写入寄存器
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;                                       //选PC3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;                                   //模拟输入模式
	GPIO_Init(GPIOC, &GPIO_InitStructure);				                                  //写入寄存器
}


static void ADC1_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
		
	DMA_DeInit(DMA1_Channel1);                                                      //配置DMA1通道1
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	 			              //ADC1地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue;	              //内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 2; 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	              //外设地址固定
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  				              //内存地址固定
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	    //半字
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;										              //循环传输
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);                                                 //DMA使能

	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;		                         	//独立ADC模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE ; 	 				                        //开启扫描模式，扫描模式用于多通道采集
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;		                        	//开启连续转换模式，即不停地进行ADC转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	            //不使用外部触发转换
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 	                        //采集数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 2;	 								                        //要转换的通道数�2
	ADC_Init(ADC1, &ADC_InitStructure);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);                                               //配置ADC时钟，为PCLK2的8分频，即9MHz
	 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_55Cycles5);    //配置ADC1的通道12为55.	5个采样周期，序列为1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 2, ADC_SampleTime_55Cycles5);    //配置ADC1的通道13为55.	5个采样周期，序列为2
	
	ADC_DMACmd(ADC1, ENABLE);                                                       //ADC1/DMA使能

	ADC_Cmd(ADC1, ENABLE);                                                          //ADC1使能
	
	ADC_ResetCalibration(ADC1);                                                     //复位校准寄存器

	while(ADC_GetResetCalibrationStatus(ADC1));                                     //等待校准寄存器复位完成
	
	ADC_StartCalibration(ADC1);                                                     //ADC校准

	while(ADC_GetCalibrationStatus(ADC1));                                          //等待校准完成
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);                                         //由于没有采用外部触发，所以使用软件触发ADC转换
}

void ADC1_Init(void)
{
	ADC1_GPIO_Config();
	ADC1_Mode_Config();
}
