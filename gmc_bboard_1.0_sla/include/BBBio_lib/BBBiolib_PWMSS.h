#ifndef BBBIO_PWMSS_H
#define BBBIO_PWMSS_H
/* ---------------------------------------------------------------------------- */
#define BBBIO_PWMSS_COUNT	3
#define BBBIO_PWMSS0	0
#define BBBIO_PWMSS1	1
#define BBBIO_PWMSS2	2

/* ---------------------------------------------------------------------------- */
int BBBIO_PWMSS_Setting_A(unsigned int PWMID , float HZ ,float dutyA);
int BBBIO_PWMSS_Setting_B(unsigned int PWMID , float HZ ,float dutyB);
int BBBIO_PWM_Init();
void BBBIO_PWM_Release();
int BBBIO_PWMSS_Status(unsigned int PWMID);
void BBBIO_ehrPWM_Enable_A(unsigned int PWMSS_ID);
void BBBIO_ehrPWM_Enable_B(unsigned int PWMSS_ID);
void BBBIO_ehrPWM_Disable_A(unsigned int PWMSS_ID);
void BBBIO_ehrPWM_Disable_B(unsigned int PWMSS_ID);
/* ---------------------------------------------------------------------------- */
#endif
