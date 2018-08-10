#ifndef __INCdebugh
#define __INCdebugh

#define DEBUG 1
#if defined(DEBUG) && DEBUG > 0
#define DPRINT(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, \
     __FILE__, __LINE__, __func__, ##args)
#else
#define DPRINT(fmt, args...) /* Don't do anything in release builds */
#endif

#endif 
