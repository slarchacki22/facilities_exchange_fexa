#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "mc_api.h"
#include "mux.h"
#include "eqep.h"
#include "BBBio_lib/BBBiolib.h"

/*
 * Standard(default) pwm freq to motor.
 * This is a good val given that the idea is
 * to send a dc voltage (approximate) to the
 * vref input on the motor-ic.
 */
#define PWM_HZ  100000.0  // 100 kHz
#define DUTY_A .02
#define DUTY_B .01

#define MAX_MOTORS 5


struct mio {
	EQEP *eqep_ctrl;
};

struct mio mio_handles[MAX_MOTORS];

int _ghome_go(EQEP *p);
int _mp_go(EQEP *p, int delta_pos);
	
void mc_init(void)
{
	int i;

	iolib_init();
	iolib_delay_ms(DELAY_50MS);

	for (i = 0; i < MAX_MOTORS; i ++) {
		mio_handles[i].eqep_ctrl = NULL;
	}
}

int mc_open(MOTOR m_id)
{
	EQEP *eqep_ctrl;
	u_int eqep_base_adrs;

	if (mio_handles[m_id].eqep_ctrl != NULL)
		/* Just return idx into motor-handles array. */
		return m_id;

	switch (m_id) {
	case AZIMUTH:
		eqep_base_adrs = EQEP0_BASE;
		break;
	case ELEVATION:
		eqep_base_adrs = EQEP2_BASE;
	case TILT:
		eqep_base_adrs = EQEP1_BASE;
		printf("Here");
		break;
	default:
		eqep_base_adrs = -1;
	}

	/* Must be either skew motor. */
	if (eqep_base_adrs == -1)
		return m_id;

	if ((eqep_ctrl = eqep_open(eqep_base_adrs)) == NULL) {
		printf("eqep_open failed\n");
		return -1;
	}
	mio_handles[m_id].eqep_ctrl = eqep_ctrl;
	return m_id;
}

int mc_close(MOTOR m_id)
{
	eqep_close(mio_handles[m_id].eqep_ctrl);
	return 0;
}

void mc_mux_reset(void)
{
	mux_init();
	mux_reset();
	mc_led_off(LED1);
	mc_led_off(LED2);
	mux_oe_en();
}

int mc_enable(MOTOR m_id)
{
	DPRINT("start(m_id=%d)\n", m_id);
	switch(m_id) {
	case ALL_MOTORS:
		mux_pinset(0,6,1); // Skew1 Enable On
		mux_pinset(1,6,1); // Skew2 Enable On
		mux_pinset(0,0,1); // Azimuth Enable
		mux_pinset(0,1,1); // Azimuth Brake Off
		mux_pinset(1,0,1); // Elevation Enable On
		mux_pinset(1,1,1); // Elevation Brake Off
		mux_pinset(2,0,1); // Tilt Enable On
		mux_pinset(2,1,1); // Tilt Brake Off
		break;
	case SKEW1:
		mux_pinset(0,6,1); // Skew1 Enable On
		break;
	case SKEW2:
		mux_pinset(1,6,1); //Skew2 Enable On
		break;
	case ELEVATION:
		mux_pinset(1,0,1); // Elevation Enable On
		mux_pinset(1,1,1); // Elevation Brake Off
		break;
	case TILT:
		mux_pinset(2,0,1); // Tilt Enable On
		mux_pinset(2,1,1); // Tilt Brake Off
		break;
	case AZIMUTH:
		mux_pinset(0,0,1); // Azimuth Enable On
		mux_pinset(0,1,1); // Azimuth Brake Off
		break;
	}
	DPRINT("done(m_id=%d)\n", m_id);
	return 0;    
}

int mc_disable(MOTOR m_id)
{
        DPRINT("start(m_id=%d)\n", m_id);
        switch(m_id) {
        case ALL_MOTORS:
			BBBIO_ehrPWM_Disable_A(BBBIO_PWMSS0); //Skew2, Tilt
			BBBIO_ehrPWM_Disable_B(BBBIO_PWMSS0); //Skew2, Tilt
			BBBIO_ehrPWM_Disable_A(BBBIO_PWMSS1); //Azimuth, Elevation
			BBBIO_ehrPWM_Disable_B(BBBIO_PWMSS1); //Azimuth, Elevation
			BBBIO_ehrPWM_Disable_A(BBBIO_PWMSS2); //Skew1, unused
			BBBIO_ehrPWM_Disable_B(BBBIO_PWMSS2); //Skew1, unused
                mux_pinset(0,6,0); // Skew1 Disable
                mux_pinset(1,6,0); // Skew2 Disable
                mux_pinset(0,0,0); // Azimuth Disable
                mux_pinset(1,0,0); // Elevation Disable
                mux_pinset(2,0,0); // Tilt Disable
	        break;
        case SKEW1:
        	BBBIO_ehrPWM_Disable_A(BBBIO_PWMSS2); //Skew1, unused
                mux_pinset(0,6,0); // Skew1 Disable
                break;
        case SKEW2:
        	BBBIO_ehrPWM_Disable_A(BBBIO_PWMSS0); //Skew2, Tilt
                mux_pinset(1,6,0); // Skew2 Disable
                break;
        case ELEVATION:
        	BBBIO_ehrPWM_Disable_B(BBBIO_PWMSS1); // Azimuth, Elevation
                mux_pinset(1,0,0); // Elevation Disable
                break;
        case TILT:
        	BBBIO_ehrPWM_Disable_B(BBBIO_PWMSS0); //Skew2, Tilt
                mux_pinset(2,0,0); // Tilt Disable
                break;
        case AZIMUTH:
        	BBBIO_ehrPWM_Disable_A(BBBIO_PWMSS1); // Azimuth, Elevation
                mux_pinset(0,0,0); // Azimuth Disable
                break;
        }
        DPRINT("done(m_id=%d)\n", m_id);
        return 0;
}

int mc_forward(MOTOR m_id, float duty)
{
	DPRINT("start[m_id=%d] duty=%.2f\n", m_id, duty);
	switch(m_id) {
	case ALL_MOTORS:
		BBBIO_PWMSS_Setting_A(BBBIO_PWMSS0, PWM_HZ, duty);
		BBBIO_PWMSS_Setting_B(BBBIO_PWMSS0, PWM_HZ, duty);
		BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS0); //Skew2, Tilt
		BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS0); //Skew2, Tilt
		BBBIO_PWMSS_Setting_A(BBBIO_PWMSS1, PWM_HZ, duty);
		BBBIO_PWMSS_Setting_B(BBBIO_PWMSS1, PWM_HZ, duty);
		BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS1); //Azimuth, Elevation
		BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS1); //Azimuth, Elevation
		BBBIO_PWMSS_Setting_A(BBBIO_PWMSS2, PWM_HZ, duty);
		BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS2); //Skew1, unused
		BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS2); //Skew1, unused
		mux_pinset(0,7,1); //Skew1 Forward
		mux_pinset(1,7,1); //Skew2 Forward
		mux_pinset(1,2,1); //Elevation Forward
		mux_pinset(2,2,1); //Tilt Forward
		mux_pinset(0,2,1); //Azimuth Forward
		break;
	case SKEW1:
		BBBIO_PWMSS_Setting_A(BBBIO_PWMSS2, PWM_HZ, duty);
		BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS2); //Skew1, unused
		mux_pinset(0,7,1); // Skew1 Forward
		break;
	case SKEW2:
		BBBIO_PWMSS_Setting_A(BBBIO_PWMSS0, PWM_HZ, duty);
		BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS0); //Skew2, Tilt
		mux_pinset(1,7,1); // Skew2 Forward
		break;
	case ELEVATION:
		DPRINT("start[m_id=%d] duty=%.2f", m_id, duty);
		BBBIO_PWMSS_Setting_B(BBBIO_PWMSS1, PWM_HZ, duty);
		BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS1); // Azimuth, Elevation

		mux_pinset(1,2,1); // Elevation Forward
		DPRINT("here1\n");
		break;
	case TILT:
		BBBIO_PWMSS_Setting_B(BBBIO_PWMSS0, PWM_HZ, duty);
		BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS0); //Skew2, Tilt
		mux_pinset(2,2,1); // Tilt Forward
		break;
	case AZIMUTH:
		DPRINT("start[m_id=%d] duty=%.2f", m_id, duty);
		BBBIO_PWMSS_Setting_A(BBBIO_PWMSS1, PWM_HZ, duty);
		BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS1); // Azimuth, Elevation
		
		mux_pinset(0,2,1); // Azimuth Forward
		break;
	}
	DPRINT("done[m_id=%d]\n", m_id);
	return 0;    
}

int mc_reverse(MOTOR m_id, float duty)
{
	DPRINT("start[m_id=%d] duty=%.2f\n", m_id, duty);
        switch(m_id) {
        case ALL_MOTORS:
			BBBIO_PWMSS_Setting_A(BBBIO_PWMSS0, PWM_HZ, duty);
			BBBIO_PWMSS_Setting_B(BBBIO_PWMSS0, PWM_HZ, duty);
			BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS0); //Skew2, Tilt
			BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS0); //Skew2, Tilt
			BBBIO_PWMSS_Setting_A(BBBIO_PWMSS1, PWM_HZ, duty);
			BBBIO_PWMSS_Setting_B(BBBIO_PWMSS1, PWM_HZ, duty);
			BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS1); //Azimuth, Elevation
			BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS1); //Azimuth, Elevation
			BBBIO_PWMSS_Setting_A(BBBIO_PWMSS2, PWM_HZ, duty);
			BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS2); //Skew1, unused
			BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS2); //Skew1, unused
            mux_pinset(0,7,0); // Skew1 Reverse
            mux_pinset(1,7,0); // Skew2 Reverse
            mux_pinset(1,2,0); // Elevation Reverse
            mux_pinset(2,2,0); // Tilt Reverse
            mux_pinset(0,2,0); //Azimuth Reverse
            break;
        case SKEW1:
			BBBIO_PWMSS_Setting_A(BBBIO_PWMSS2, PWM_HZ, duty);
			BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS2); //Skew1, unused
            mux_pinset(0,7,0); // Skew1 Reverse
            break;
        case SKEW2:
			BBBIO_PWMSS_Setting_A(BBBIO_PWMSS0, PWM_HZ, duty);
			BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS0); //Skew2, Tilt
            mux_pinset(1,7,0); // Skew2 Reverse
            break;
        case ELEVATION:
        	DPRINT("here0\n");
			BBBIO_PWMSS_Setting_B(BBBIO_PWMSS1, PWM_HZ, duty);
			BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS1); // Azimuth, Elevation
            mux_pinset(1,2,0); // Elevation Reverse
            break;
        case TILT:
			BBBIO_PWMSS_Setting_B(BBBIO_PWMSS0, PWM_HZ, duty);
			BBBIO_ehrPWM_Enable_B(BBBIO_PWMSS0); //Skew2, Tilt
            mux_pinset(2,2,0); // Tilt Reverse
            break;
        case AZIMUTH:
			BBBIO_PWMSS_Setting_A(BBBIO_PWMSS1, PWM_HZ, duty);
			BBBIO_ehrPWM_Enable_A(BBBIO_PWMSS1); // Azimuth, Elevation
            mux_pinset(0,2,0); // Azimuth Reverse
            break;
        }
        DPRINT("done(m_id=%d)\n", m_id);
        return 0;    
}

int mc_led_on(LED led_t, LED_CLR lclr)
{
	int iclr = led_t + lclr - 1;
	DPRINT("led_t:%d  lclr:%d, iclr:%d\n", led_t, lclr, iclr);
	mux_pinset(3, iclr, 0);
        return 0;
}

int mc_led_off(LED led_t)
{
	/* Must turn off all color-bits? */
        mux_pinset(3, led_t, 1);
        mux_pinset(3, led_t + 1, 1);
        mux_pinset(3, led_t + 2, 1);
        return 0;
}

int mc_ghome(MOTOR m_id)
{
	EQEP *p = mio_handles[m_id].eqep_ctrl;
	int qslat;

	mc_enable(m_id);
	mc_reverse(m_id, DEFAULT_PWM_DUTY);
	qslat = _ghome_go(p);
	mc_disable(m_id);

	printf("=> qpos at home: %d\n", qslat);
	return 0;
}

int mc_xghome(MOTOR m_id)
{
	EQEP *p = mio_handles[m_id].eqep_ctrl;
	int qslat0;
	int qslat1;
	int qslat2;

	mc_enable(m_id);

	mc_reverse(m_id, DEFAULT_PWM_DUTY);
	qslat0 = _ghome_go(p);
	_mp_go(p, 300);

	mc_forward(m_id, DEFAULT_PWM_DUTY);
	qslat1 = _ghome_go(p);
	_mp_go(p, 300);

	mc_reverse(m_id, DEFAULT_PWM_DUTY);
	qslat2 = _ghome_go(p);
	_mp_go(p, 800);

	mc_disable(m_id);

	printf("=> qpos at home: [%d, %d, %d]\n", qslat0, qslat1, qslat2);
	return 0;
}

int _ghome_go(EQEP *p)
{
	int max_reads;
	int j;
	int k;
#if DEBUG
	u_int qpos;
#endif
	u_int qslat;
	u_int qslat_curr;

	qslat_curr = *p->p_qpslat;
	max_reads = 100000;
	for (j = 0; j < max_reads; j ++) {
	        usleep(1000);
		for (k = 0; k < 10; k ++) {
#ifdef DEBUG
			qpos = *p->p_qpos;
#endif
			qslat = *p->p_qpslat;
			if (qslat_curr != qslat) {
				qslat_curr = qslat;
				return qslat;
			}
		}
		DPRINT("qpos: %d\n", qpos);
	}

	return -1;
}

int _mp_go(EQEP *p, int delta_qpos)
{
	int max_reads;
	int j;
	int k;
	int is_done;
	int qpos_start;
	int qpos;

	max_reads = 100000;
	qpos_start = *p->p_qpos;

	DPRINT("initial qpos: %d\n", qpos_start);

	for (j = 0, is_done = 0; j < max_reads; j ++) {
	        usleep(1000);
		for (k = 0; k < 10; k ++) {
			qpos = *p->p_qpos;
			if (abs(qpos - qpos_start) >= delta_qpos) {
				is_done = 1;
				break;
			}
		}
		if (is_done == 1)
			break;

		DPRINT("qpos: %d\n", qpos);

	}

	return qpos;
}

int mc_mp(MOTOR m_id, int delta_pos)
{
	EQEP *p = mio_handles[m_id].eqep_ctrl;
	int qpos;

	mc_reverse(m_id, DEFAULT_PWM_DUTY);
	mc_enable(m_id);
	qpos = _mp_go(p, delta_pos);
	mc_disable(m_id);

	printf("=> stopped at qpos: %d\n", qpos);
	return 0;
}
