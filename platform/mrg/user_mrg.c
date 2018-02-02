#include <inttypes.h>
typedef uint64_t iova_t;
typedef unsigned long vaddr_t;

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>

#include <mrg.h>
#include <mrg/blk.h>

#include <bmk-core/core.h>
#include <bmk-core/errno.h>
#include <bmk-core/memalloc.h>
#include <bmk-core/printf.h>
#include <bmk-core/sched.h>
#include <bmk-core/string.h>
#include <bmk-core/mainthread.h>

#include <bmk-rumpuser/rumpuser.h>

#if 0
/* bmk includes are unusable if you link a proper C headers. *SIGH*. */
typedef void (*rump_biodone_fn)(void *, size_t, int);
int rumpuser_open(const char *path, int ruflags, int *fdp);
int rumpuser_close(int fd);
void rumpuser_bio(int fd, int op, void *data, size_t dlen, int64_t doff,
		  rump_biodone_fn biodone, void *bioarg);
#endif

/* Yeah... */
static int next=0;
static struct blkdisk *fildes[256];

int
rumpuser_open(const char *path, int ruflags, int *fdp)
{
	struct blkdisk *blk;

	blk = blk_open(squoze((char *)path));
	if (blk == NULL)
		return -ENOENT;

	if (next >= 256) {
		blk_close(blk);
		return -1;
	}

	fildes[next++] = blk;

	if (fdp)
		*fdp = next-1;
	return 0;
}

int
rumpuser_close(int fd)
{
	blk_close(fildes[fd]);
	fildes[fd] = NULL;
	return 0;
}

struct rumpmrg_bio {
	int evt;
	int ret;
	rump_biodone_fn biodone;
	void *bioarg;
};

unsigned char *datap;

static void _rumpmrg_bio_ast(void *arg)
{
	struct rumpmrg_bio *biop = (struct rumpmrg_bio *)arg;

	//	set_thread("bio_ast");

	rumpuser__hyp.hyp_schedule();
	biop->biodone(biop->bioarg, biop->ret, biop->ret);
	rumpuser__hyp.hyp_unschedule();
	evtfree(biop->evt);
	free(biop);
}

void
rumpuser_bio(int fd, int op, void *data, size_t dlen, int64_t doff,
	rump_biodone_fn biodone, void *bioarg)
{
	uint64_t blkid;
	size_t blkno;
	struct rumpmrg_bio *biop;

	memset(data, 0x55, dlen);

	blkid = doff / 512;
	assert (doff % 512 == 0);
	blkno = (dlen + 511) / 512;

	biop = malloc(sizeof(*biop));
	biop->evt = evtalloc();
	biop->biodone = biodone;
	biop->bioarg = bioarg;

	evtast(biop->evt, _rumpmrg_bio_ast, biop);
	if (op & RUMPUSER_BIO_READ) {
		blk_read(fildes[fd], data, dlen, blkid, blkno, biop->evt, &biop->ret);
		datap = data;
	} else {
		printf("<Write disabled>\n");
		//blk_write(fildes[fd], blkid, blkno, data, dlen, biop->evt, &biop->ret);
	}

}

int
rumpuser_getfileinfo(const char *path, uint64_t *sizep, int *ftp)
{
	struct blkdisk *blk;
	
	blk = blk_open(squoze((char *)path));
	if (blk == NULL)
		return -ENOENT;

	if (sizep)
		*sizep = blk->info.blkno * blk->info.blksz;
	if (ftp)
		*ftp = RUMPUSER_FT_BLK;

	blk_close(blk);
	return 0;
}
