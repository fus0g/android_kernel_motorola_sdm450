#ifndef _LINUX_ALI_DBGFB_H
#define _LINUX_ALI_DBGFB_H
#include <linux/init.h>
/*
 * Bring-up aid for ali (no UART): paint a white band into the bootloader's
 * continuous-splash framebuffer at boot checkpoints, then pause ~1.5s so the
 * progress is visible on the panel. Band k sits at line ~k*70.
 */
#ifdef CONFIG_ALI_DBGFB
void __init ali_dbgfb_early(int k);	/* before vmalloc: uses early_ioremap */
void ali_dbgfb(int k);			/* after mm_init: uses ioremap */
#else
static inline void ali_dbgfb_early(int k) {}
static inline void ali_dbgfb(int k) {}
#endif
#endif
