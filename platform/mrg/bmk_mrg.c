#include <sys/cdefs.h>
#include <inttypes.h>

typedef unsigned long vaddr_t;
typedef uint64_t iova_t;

#include <microkernel.h>
#include <mrg.h>

#include <bmk-rumpuser/rumpuser.h>

#include <bmk-core/platform.h>
#include <bmk-core/printf.h>
#include <bmk-core/sched.h>
#include <bmk-core/core.h>
#include <bmk-core/errno.h>
#include <bmk-core/memalloc.h>
#include <bmk-core/sched.h>
#include <bmk-core/string.h>
#include <bmk-core/mainthread.h>
#include <bmk-core/pgalloc.h>
#include <bmk-core/platform.h>

#include "private.h"

#define MAXMEM 800*1024*1024

int
rumprun_platform_rumpuser_init(void)
{
  return 0;
}

unsigned long
bmk_platform_splhigh(void)
{
	extern unsigned __preemption_level;

	preempt_disable();
	return __preemption_level > 1;
}

void
bmk_platform_splx(unsigned long x)
{

	preempt_enable();
}


void __attribute__((noreturn))
bmk_platform_halt(const char *panicstring)
{

	if (panicstring)
		bmk_printf("PANIC: %s\n", panicstring);

	sys_die(-1);
	while(1);
}

void
bmk_platform_cpu_block(bmk_time_t until)
{
	/* TODO: Add time */
	sys_wait();
}

void
bmk_platform_cpu_sched_settls(struct bmk_tcb *next)
{
	sys_tls((void *)next->btcb_tp);
}


static uint64_t x = 0;
bmk_time_t
bmk_platform_cpu_clock_monotonic(void)
{
  /* Ehm. */

  return x++;
}

bmk_time_t
bmk_platform_cpu_clock_epochoffset(void)
{
  return 500 + x * 101;
}

static void
_app_main(void *arg)
{
	bmk_mainthread("{ \"cmdline\" : \"/bin/ls\" } ");
	/* NOTREACHED */
}

void bmk_mrg_start(void)
{
  	extern char _end[];

	bmk_printf_init((void (*)(int))sys_putc, NULL);

	sys_putc('a');
	bmk_sched_init();
	sys_putc('a');
	bmk_core_init(0);
	sys_putc('a');

	brk((void*)(MAXMEM));

	bmk_pgalloc_loadmem(bmk_round_page((unsigned long)_end),
			    MAXMEM);
	sys_putc('b');
	bmk_memsize = MAXMEM - (unsigned long)_end;

	bmk_sched_startmain(_app_main, NULL);
	bmk_platform_halt("unreachable");
}
