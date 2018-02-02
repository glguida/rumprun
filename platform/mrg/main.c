#include <stdio.h>
#include <stdlib.h>

#include "private.h"

static int _sys_write(void *cookie, const char *c, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		sys_putc(*c++);
	}
	return 0;
}

int main(int argc, char **argv)
{
	FILE *syscons;

	syscons = fwopen(NULL, _sys_write);
	stdout = syscons;
	stderr = syscons;
	setvbuf(syscons, NULL, _IONBF, 80);

	fprintf(syscons, "e allora?");
	printf("Funziona!\n");

	bmk_mrg_start();
	return 0;
}
