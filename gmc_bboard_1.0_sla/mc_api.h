#ifndef __INCmc_apih
#define __INCmc_apih

#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

#include "BBBio_lib/BBBiolib.h"

typedef enum {
	ALL_MOTORS,
	AZIMUTH,
	ELEVATION,
	TILT,
	SKEW1,
	SKEW2 } MOTOR;

typedef enum { LED1 = 1, LED2 = 4 } LED;
typedef enum { RED = 1, GRN, BLU } LED_CLR;

/* Default for now */
#define DEFAULT_PWM_DUTY 20.0

void mc_init(void);
int mc_open(MOTOR m_id);
int mc_close(MOTOR m_id);
void mc_mux_reset(void);

int mc_enable(MOTOR m_id);
int mc_disable(MOTOR m_id);
int mc_forward(MOTOR m_id, float duty);
int mc_reverse(MOTOR m_id, float duty);
int mc_ghome(MOTOR m_id);
int mc_xghome(MOTOR m_id);
int mc_mp(MOTOR m_id, int delta_pos);
int mc_led_on(LED led_t, LED_CLR lclr);
int mc_led_off(LED led_t);

#endif /* __INCmc_apih */
