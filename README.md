SwiftSolar firmware for REV.C hardware. This is targeted for the STM32U575VGTXQ microcontroller. This is the official release version. See various branches for code targeting specific hardware features



\*\*\*Tips to get the code to work\*\*\*\*

\*\*\*TIP ONE\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*

The code deinitializes the MDF interface when not being used by calling HAL\_MDF\_DeInit(\&MdfHandle0) and HAL\_MDF\_DeInit(\&MdfHandle1). This function is found in file stm32u5xx\_hal\_mdf.c

Look for the following lines of code:

\#else /\* USE\_HAL\_MDF\_REGISTER\_CALLBACKS \*/

&#x20;     //HAL\_MDF\_MspDeInit(hmdf);

\#endif /\* USE\_HAL\_MDF\_REGISTER\_CALLBACKS \*/



Notice that HAL\_MDF\_MspDeInit(hmdf) is commented out above (line 550). This is the way it should be. Add HAL\_MDF\_MspDeInit(hmdf) as shown in the code below:

if (((v\_mdf1InstanceCounter == 1U) \&\& IS\_MDF\_INSTANCE(hmdf->Instance)) ||

&#x20;   ((v\_adf1InstanceCounter == 1U) \&\& IS\_ADF\_INSTANCE(hmdf->Instance)))

{

&#x09;MDF\_TypeDef \*p\_mdf\_base;

&#x20;       /\* Get MDF base according instance \*/

&#x20;       p\_mdf\_base = (IS\_ADF\_INSTANCE(hmdf->Instance)) ? ADF1 : MDF1;



&#x20;       /\* Disable clock generator \*/

&#x20;       p\_mdf\_base->CKGCR \&= \~(MDF\_CKGCR\_CKDEN);



&#x20;       HAL\_MDF\_MspDeInit(hmdf); //<=\*\*\*ADD THIS FUNCTION CALL HERE\*\*\*

}



\*\*\*TIP TWO\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*

The code initializes and de-initializes the I2C2 peripheral attached to the display depending on whether the lid is open or closed. An issue was discovered when re-initializing the I2C2 clock. The problem code can be found in the i2c.c file in the HAL\_I2C\_MspInit() function. We need to test RCC->CCIPR1 register to see if the clock has already been initialized. We can do this by adding an if statement around the clock initialization section as follows:



if((RCC->CCIPR1 \& 12288) != 12288)

{

&#x20; PeriphClkInit.PeriphClockSelection = RCC\_PERIPHCLK\_I2C2;

&#x20; PeriphClkInit.I2c2ClockSelection = RCC\_I2C2CLKSOURCE\_MSIK;

&#x20; if (HAL\_RCCEx\_PeriphCLKConfig(\&PeriphClkInit) != HAL\_OK)

&#x20; {

&#x20;   Error\_Handler();

&#x20; }



&#x20; \_\_HAL\_RCC\_GPIOB\_CLK\_ENABLE();

}

