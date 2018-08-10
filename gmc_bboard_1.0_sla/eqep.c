#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "eqep.h"

EQEP *eqep_open(u_int eqep_base_adrs)
{
	u_int map_size = getpagesize();
	u_int map_mask = map_size - 1;
	volatile u_int *pmmap;
	static EQEP *eqep_ctrl;
	EQEP *p;
	int fdm;

	if ((fdm = open("/dev/mem", O_RDWR, O_SYNC)) < 0) {
		printf("Could not open /dev/mem\n");
		return NULL;
	}

	if ((pmmap = (u_int *) mmap(0, map_size, PROT_READ | PROT_WRITE,
	                            MAP_SHARED, fdm,
				    eqep_base_adrs & ~map_mask)) == NULL) {
		printf("Unable to mmap eqep_A\n");
		return NULL;
	}

	if ((eqep_ctrl = (EQEP *) malloc(sizeof(EQEP))) == NULL)
		return NULL;

	/* For convenience. */
	p = eqep_ctrl;
	p->fdm = fdm;
	p->base_adrs = eqep_base_adrs;

	/* Derive base address (ptrs are at offsets to this base). */
	p->vbase_adrs = (u_int) pmmap + (p->base_adrs & map_mask);

	p->p_qctl = (uint16_t *) (p->vbase_adrs + QEPCTL);
	p->p_qdctl = (uint16_t *) (p->vbase_adrs + QDECCTL);
	p->p_qpctl = (uint16_t *) (p->vbase_adrs + QPOSCTL);
	p->p_qcctl = (uint16_t *) (p->vbase_adrs + QCAPCTL);
	p->p_qposinit = (uint32_t *) (p->vbase_adrs + QPOSINIT);
	p->p_qposmax = (uint32_t *) (p->vbase_adrs + QPOSMAX);
	p->p_qpos = (uint32_t *) (p->vbase_adrs + QPOSCNT);
	p->p_qpilat = (uint32_t *) (p->vbase_adrs + QPOSILAT);
	p->p_qpslat = (uint32_t *) (p->vbase_adrs + QPOSSLAT);

	DPRINT("pmmap           = x%08X\n", (u_int) pmmap);
	DPRINT("vbase_adrs      = x%08X\n", p->vbase_adrs);
	DPRINT("p_qctl          = x%08X\n", (u_int) p->p_qctl);
	DPRINT("p_qdctl         = x%08X\n", (u_int) p->p_qdctl);
	DPRINT("p_qpctl         = x%08X\n", (u_int) p->p_qpctl);
	DPRINT("p_qcctl         = x%08X\n", (u_int) p->p_qcctl);
	DPRINT("p_qposinit      = x%08X\n", (u_int) p->p_qposinit);
	DPRINT("p_qposmax       = x%08X\n", (u_int) p->p_qposmax);
	DPRINT("p_qpos          = x%08X\n", (u_int) p->p_qpos);
	DPRINT("p_qpilat        = x%08X\n", (u_int) p->p_qpilat);
	DPRINT("p_qpslat        = x%08X\n", (u_int) p->p_qpslat);
	
#ifdef notdef /* suggested init reg vals currently */
	QEPCTL:  x15be
	QDECCTL: x0180
	QPOSCTL: x0000
	QCAPCTL: x8000
	QPOSMAX: x00001000
#endif

	/**
	 * Initialize eqep ctrl regs;
	 *  - zero poscnt
	 *  - want poscnt to not wrap around a max val.
	 */
	*p->p_qctl = 0x15be;
	*p->p_qdctl = 0x0180;
	*p->p_qpctl = 0x0000;
	*p->p_qcctl = 0x8000;

	*p->p_qposinit = 0x00000000;
	*p->p_qposmax = 0xFFFFFFFF;

	DPRINT("QEPCTL   = x%04X\n", *p->p_qctl);
	DPRINT("QDECCTL  = x%04X\n", *p->p_qdctl);
	DPRINT("QPOSCTL  = x%04X\n", *p->p_qpctl);
	DPRINT("QCAPCTL  = x%04X\n", *p->p_qcctl);
	DPRINT("QPOSINIT = x%08X\n", *p->p_qposinit);
	DPRINT("QPOSMAX  = x%08X\n", *p->p_qposmax);

	return eqep_ctrl;
}

int eqep_close(EQEP *eqep_ctlr)
{
	if (eqep_ctlr == NULL)
		return -1;
	else {
		(void) free(eqep_ctlr);
		return 0;
	}
}
