#ifndef __INCmuxh
#define __INCmuxh

/* Possible arg to iolib_delay_ms() [BBB_iolib] */
#define DELAY_1MS 1
#define DELAY_50MS 50

void mux_init(void);
void mux_reset(void);
void mux_pinset(int IO_OUT_Y,int IO_OUT_X, int val);
void mux_oe_en(void);
void mux_refsw1_en(void);
void mux_ref_sw1_dis(void);
void mux_ref_sw2_en(void);
void mux_ref_sw2_dis(void);
void mux_hpt_discr_en(void);
void mux_hpt_discr_dis(void);
void mx_set_in_mux(int chan);

#endif /* __INCmuxh */
