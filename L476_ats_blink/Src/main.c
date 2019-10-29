
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_gpio.h"
#include "stm32l476xx.h"
#include "core_cm4.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_pwr.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

// #if defined(USE_FULL_ASSERT)
// #include "stm32_assert.h"
// #endif /* USE_FULL_ASSERT */

//Registre RTC pour sauvegarder des données
//

#include "gpio.h"

void SystemClock_Config(void);
void SystemClock_Config_PLL(void);
void GPIOC_init(void);
void PC10_Freq(int val); //Fonction pour mettre à 1 ou a 0 le pin PC10
void initSystic(void);
void SysTick_Handler(void);
void MSICalibration_Process(void);

int Led_on=1;
int btnbleu=0;
int sortie_Systick = 0;

int main(void)
{
/* Configure the system clock */
SystemClock_Config();
MSICalibration_Process();
//SystemClock_Config_PLL();
//initSystic();

// config GPIO
GPIO_init();
GPIOC_init();

// init timer pour utiliser la fonction LL_mDelay() de stm32l4xx_ll_utils.c
//LL_Init1msTick( SystemCoreClock );

while (1)
 {

	if (BLUE_BUTTON())
	{
		initSystic();
		//entrer en mode sleep
		LL_LPM_EnableSleep();
		__WFI();
	}
	if (sortie_Systick == 1)
	{
		sortie_Systick = 0;
		LL_LPM_EnableSleep();
		__WFI();
	}

	/*
	if	( BLUE_BUTTON() )
		LED_GREEN(1);
	else {
		LED_GREEN(0);
		LL_mDelay(950);
		LED_GREEN(1);
		LL_mDelay(50);
		}*/
	}
}

//Réponse question 3

void SystemClock_Config(void)
{
/* MSI configuration and activation */
LL_FLASH_SetLatency(LL_FLASH_LATENCY_4); //Demander à quoi sert la flash latency
LL_RCC_MSI_Enable();
while	(LL_RCC_MSI_IsReady() != 1)
	{ };
  
/* Main PLL desactivation */
LL_RCC_PLL_Disable();

/* Update the global variable called SystemCoreClock */
SystemCoreClockUpdate();
}




/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config_PLL(void)
{
  /* MSI configuration and activation */
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
  LL_RCC_MSI_Enable();
  while(LL_RCC_MSI_IsReady() != 1)
  {
  };

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };

  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };

  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 80MHz */
  /* This frequency can be calculated through LL RCC macro */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ(__LL_RCC_CALC_MSI_FREQ(LL_RCC_MSIRANGESEL_RUN, LL_RCC_MSIRANGE_6),
                                  LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2)*/
  LL_Init1msTick(80000000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(80000000);
}

void MSICalibration_Process(void)
{
  /* LSE configuration and activation */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_EnableBkUpAccess();

  /* Enable LSE */
  LL_RCC_ForceBackupDomainReset();
  LL_RCC_ReleaseBackupDomainReset();
  LL_RCC_LSE_Enable();

  while (LL_RCC_LSE_IsReady() != 1)
  {
  };

  /* Enable MSI clock PLL */
  LL_RCC_MSI_EnablePLLMode();
}

void GPIOC_init(void)
{
	LL_GPIO_SetPinMode(GPIOC,LL_GPIO_PIN_10, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinOutputType( GPIOC, LL_GPIO_PIN_10, LL_GPIO_OUTPUT_PUSHPULL );
}

void initSystic(void)
{

	SysTick->LOAD = 0x00009C40; //24 bits timer  7F0000
	NVIC_SetPriority(SysTick_IRQn, -1); //Set interupt priority
	SysTick->VAL = 0;
	SysTick->CTRL = 0b111;

	//SysTick_Config()
	NVIC_EnableIRQ(SysTick_IRQn); //Enabling the interuption
}

 void SysTick_Handler(void)
 {
	 if(Led_on == 1)
	 {
		LED_GREEN(0);
		PC10_Freq(0); //Mise à 0 du GPIOc Pin 10
		Led_on = 0;
	 }
	 else
	 {
		 LED_GREEN(1);
		 PC10_Freq(1); //Mise à 1 du GPIOc Pin 10
		 Led_on = 1;
	 }

	 sortie_Systick = 1;
 }

 void PC10_Freq(int val)
 {
	 if (val)
	 {
		 LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_10);
	 }
	 else
	 {
		 LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_10);
	 }
 }


