#include <linux/init.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <asm/cacheflush.h>
#include <asm/memory.h>
#include <asm/early_ioremap.h>
#include <linux/ali_dbgfb.h>

#define FB_PHYS		0x90001000UL
#define LINE_BYTES	(1080UL * 3)
#define BAND_LINES	40UL
#define BAND_BYTES	(BAND_LINES * LINE_BYTES)
#define BAND_STRIDE	(70UL * LINE_BYTES)
#define MAP_BYTES	(8UL << 20)
#define PAUSE_SEC_X10	15	/* 1.5 s */

static void __iomem *fb_map;

static void ali_dbgfb_pause(void)
{
	u64 f, t0, t;

	asm volatile("mrs %0, cntfrq_el0" : "=r"(f));
	asm volatile("mrs %0, cntvct_el0" : "=r"(t0));
	f = f / 10 * PAUSE_SEC_X10;
	do {
		asm volatile("mrs %0, cntvct_el0" : "=r"(t));
	} while (t - t0 < f);
}

static void fill(void __iomem *p)
{
	unsigned long i;

	for (i = 0; i < BAND_BYTES; i += 8)
		writeq_relaxed(~0ULL, p + i);
	wmb();
}

void __init ali_dbgfb_early(int k)
{
	void __iomem *p = early_ioremap(FB_PHYS + k * BAND_STRIDE, BAND_BYTES);

	if (!p)
		return;
	fill(p);
	early_iounmap(p, BAND_BYTES);
	ali_dbgfb_pause();
}

void ali_dbgfb(int k)
{
	unsigned long phys = FB_PHYS + k * BAND_STRIDE;

	if (pfn_valid(__phys_to_pfn(phys))) {
		/* Splash region is normal RAM here: ioremap() would refuse it.
		 * Use the linear map and push the lines out to the display. */
		void *p = __va(phys);

		memset(p, 0xff, BAND_BYTES);
		__flush_dcache_area(p, BAND_BYTES);
	} else {
		if (!fb_map)
			fb_map = ioremap(FB_PHYS, MAP_BYTES);
		if (!fb_map)
			return;
		fill(fb_map + k * BAND_STRIDE);
	}
	ali_dbgfb_pause();
}
