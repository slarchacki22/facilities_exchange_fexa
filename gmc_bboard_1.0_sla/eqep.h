#ifndef __INCeqeph
#define __INCeqeph

#include <stdint.h>
#include "mc_api.h"

#define EQEP0_BASE 0x48300180
#define EQEP1_BASE 0x48302180
#define EQEP2_BASE 0x48304180

/**
 * Register offsets; note the width of each one
 * when allocating ptrs.
 */
#define QPOSCNT    0x0000 /* (r/w) width=4 */
#define QPOSINIT   0x0004 /* (r/w) width=4 */
#define QPOSMAX    0x0008 /* (r/w) width=4 */
#define QPOSCMP    0x000C /* (r/w) width=4 */
#define QPOSILAT   0x0010 /* (r/w) width=4 */
#define QPOSSLAT   0x0014 /* (r/w) width=4 */
#define QPOSLAT    0x0018 /* (r/w) width=4 */
#define QDECCTL    0x0028 /* (r/w) width=2 */
#define QEPCTL     0x002A /* (r/w) width=2 */
#define QCAPCTL    0x002C /* (r/w) width=2 */
#define QPOSCTL    0x002E /* (r/w) width=2 */

/* QEPCTL reg bits */
#define FREESOFT1  (0x0001 << 15)
#define FREESOFT0  (0x0001 << 14)
#define PCRM1      (0x0001 << 13)
#define PCRM0      (0x0001 << 12)
#define SEI1       (0x0001 << 11)
#define SEI0       (0x0001 << 10)
#define IEI1       (0x0001 << 9)
#define IEI0       (0x0001 << 8)
#define SWI        (0x0001 << 7)
#define SEL        (0x0001 << 6)
#define IEL1       (0x0001 << 5)
#define IEL0       (0x0001 << 4)
#define PHEN       (0x0001 << 3)
#define QCLM       (0x0001 << 2)
#define UTE        (0x0001 << 1)
#define WDE        (0x0001 << 0)

typedef struct {
	MOTOR m_id;
	int fdm;
	u_int base_adrs;
	u_int vbase_adrs;
	uint16_t *p_qctl;
	uint16_t *p_qdctl;
	uint16_t *p_qpctl;
	uint16_t *p_qcctl;
	uint32_t *p_qposinit;
	uint32_t *p_qposmax;
	uint32_t *p_qpos;
	uint32_t *p_qpilat;
	uint32_t *p_qpslat;
        uint32_t *p_qpslats;
} EQEP;


EQEP *eqep_open(MOTOR m_id);
int eqep_close(EQEP *eqep_ctlr);

#endif /* __INCeqeph */
