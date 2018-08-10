#include <stdio.h>
#include <stdlib.h>

#include "mux.h"
#include "BBBio_lib/BBBiolib.h"

/**
 * Mux-Table [wired mods]
 *
 *    0        1       2       3       4       5           6           7
 * 0  az_en    az_br   az_dir  f1_en   f1_br   f1_dir      s1_en       s1_dir
 * 1  el_en    el_br   el_dir  f2_en   f2_br   f2_dir      s2_en       s2_dir
 * 2  ti_en    ti_br   ti_dir  oe      rf_sw1  rf_sw2      hpt_discr   629_yn
 * 3  629_rst  led1_r  led1_g  led1_b  led2_r  led2_g      led2_b      <na>
 */

static int IO_OUT[4][8] = {{0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 1},
                           {0, 1, 1, 1, 1, 1, 1, 0}};

/**
 * This array contains pin header information
 * All latch data pins are on P8, 
 * the mux address is the array position
 * the content is the pin number
 *          0	1   2   3
 * LATCH = 17  19  13  14
 */
static int LATCH[4] = {17, 19, 13, 14};

/*********************************************
 * To set one line,
 * IO_OUTY and IO_OUTX can be seen in the table
 * for the IO_OUT array.
 */
void mux_pinset(int IO_OUT_Y,int IO_OUT_X, int val)
{
	int i;
	IO_OUT[IO_OUT_Y][IO_OUT_X] = val;

	if ((IO_OUT_X & 0b00000001) != 0) 
		pin_high(8, 27);
	else 
		pin_low(8, 27);

	if ((IO_OUT_X & 0b00000010) != 0) 
		pin_high(8, 29);
	else 
		pin_low(8, 29);

	if ((IO_OUT_X & 0b00000100) != 0) 
		pin_high(8, 28);
	else 
		pin_low(8, 28);

	for (i = 0; i < 4; i++) {
		if (IO_OUT[i][IO_OUT_X] != 0) 
			pin_high(8,LATCH[i]);
		else 
			pin_low(8,LATCH[i]);
        }

#ifdef notdef 
	pin_high(8, 14); 
	iolib_delay_ms(1);
#endif

	pin_low(8, 30);    //enable: to latch
	iolib_delay_ms(DELAY_1MS); 
	pin_high(8, 30);   //enable: to memory
}

void mux_oe_en(void)
{
	mux_pinset(2,3,1);   // Output Enable
}

void mux_refsw1_en(void)
{
	mux_pinset(2,4,1);   
}

void mux_ref_sw1_dis(void)
{
	mux_pinset(2,4,0);
}

void mux_ref_sw2_en(void)
{
	mux_pinset(2,5,1);
}

void mux_ref_sw2_dis(void)
{
	mux_pinset(2,5,0);
}

void mux_hpt_discr_en(void)
{
	mux_pinset(2,6,1);
}

void mux_hpt_discr_dis(void)
{
	mux_pinset(2,6,0);
}

/* Writing the data set on IO_OUT 0 to 3 to the latch */
void mux_init(void)
{
     // gpio 30 - Addr 629 to activate mux 4 - cs at the 4th mux, no chip there
	iolib_setdir(9, 11, BBBIO_DIR_OUT);
	iolib_setdir(9, 13, BBBIO_DIR_OUT); // gpio 31 

	// enable latch
	iolib_setdir(8, 18, BBBIO_DIR_OUT); // gpio 65 - latch reset 
	iolib_setdir(8, 30, BBBIO_DIR_OUT); // gpio 89 - DOUT_EN

	// addr latch
	iolib_setdir(8, 27, BBBIO_DIR_OUT); // gpio 86 - DOUT_MUX0
	iolib_setdir(8, 29, BBBIO_DIR_OUT); // gpio 87 - DOUT_MUX1
	iolib_setdir(8, 28, BBBIO_DIR_OUT); // gpio 88 - DOUT_MUX2

	// data latch
	iolib_setdir(8, 17, BBBIO_DIR_OUT); // gpio 27 - IO_OUT_0
	iolib_setdir(8, 19, BBBIO_DIR_OUT); // gpio 22 - IO_OUT_1
	iolib_setdir(8, 13, BBBIO_DIR_OUT); // gpio 23 - IO_OUT_2
	iolib_setdir(8, 14, BBBIO_DIR_OUT); // gpio 26 - IO_OUT_3

#ifdef notdef
	// addr latch
	iolib_setdir(9, 31, BBBIO_DIR_OUT); // IN_MUX0
	iolib_setdir(9, 29, BBBIO_DIR_OUT); // IN_MUX1
	iolib_setdir(9, 30, BBBIO_DIR_OUT); // IN_MUX2

	// data digital
	iolib_setdir(9, 12, BBBIO_DIR_IN); // DIN1
	iolib_setdir(9, 15, BBBIO_DIR_IN); // DIN2
	iolib_setdir(9, 17, BBBIO_DIR_IN); // DIN3
	iolib_setdir(9, 18, BBBIO_DIR_IN); // DIN0
#endif
        
	// activate mux 4
	pin_high(9, 11); 
	pin_high(9, 13);
}

void mux_reset(void)
{	
        // sets everything -> low

        pin_low(8, 18);    // reset 
        pin_high(8, 30);   // enable:to reset
        iolib_delay_ms(DELAY_1MS); 

        pin_high(8, 18);
        pin_high(8, 30);   // enable:to memory
        iolib_delay_ms(DELAY_1MS); 
}

#ifdef notdef
/* writing the data set on IO_Out 0 to 3 to the latch*/
void latch_write_OLD()
{
        pin_low(8, 30);   // gpio 65 - Latch enable -> low to latch
        iolib_delay_ms(DELAY_1MS); 
        pin_high(8, 30);  // gpio 89 - Latch enable to memory
}
#endif

// Ch between 0 and 7
void mx_set_in_mux(int chan)
{
        if ((chan & 0b00000001) != 0) 
                pin_high(9,30);
        else
                pin_low(9,30);
        if ((chan & 0b00000010) != 0) 
                pin_high(9,31);
        else 
                pin_low(9,31);
        if ((chan & 0b00000100) != 0) 
                pin_high(9,29);
        else 
                pin_low(9,29);
}
