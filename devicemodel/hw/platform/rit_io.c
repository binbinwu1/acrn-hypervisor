#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "inout.h"
#include <string.h>

#include "dm.h"
#include "vmmapi.h"
#include "mevent.h"

#define RIT_NAME		"rit_io"

struct rit_agent {
	struct inout_port io_ctl;
	struct inout_port io_0x83;
};

static struct rit_agent ra_inst;
static char* dirname;

static int rit_io_0x83_handler(struct vmctx *ctx, int vcpu, int in, int port,
		  int bytes, uint32_t *eax, void *arg)
{
	printf("RIT_IO: vcpu=%d, in=%d, port=0x%x, bytes=%d, eax=0x%x\r\n", vcpu, in, port, bytes, *eax);
	return 0;
}

static int rit_io_ctl_handler(struct vmctx *ctx, int vcpu, int in, int port,
		  int bytes, uint32_t *eax, void *arg)
{
	char path[80];
	FILE *fp_cfg = NULL;
	FILE *fp_dump = NULL;
	uint32_t addr;
	uint32_t len;
	uint32_t cfg_len;
	int i = 0;

	printf("RIT_IO: vcpu=%d, in=%d, port=0x%x, bytes=%d, eax=0x%x\r\n", vcpu, in, port, bytes, *eax);

	if (*eax) {
		sprintf(path, "%s/rit_cfg.bin", dirname);
		fp_cfg = fopen(path, "r");
		if (fp_cfg == NULL) {
			printf("RIT_IO: failed to open %s\r\n", path);
			return -1;
		}

		fseek(fp_cfg, 0, SEEK_END);
		cfg_len = ftell(fp_cfg);
		if (cfg_len == 0) {
			printf("RIT_IO: invalid cfg file\r\n");

			return -1;
		}

		fseek(fp_cfg, 0, SEEK_SET);
		while(cfg_len) {
			sprintf(path, "%s/rit_err_%d.dump", dirname, i);
			fp_dump = fopen(path, "w");

			if (fp_dump == NULL) {
				printf("RIT_IO: failed to open %s\r\n", path);
				goto ERROR;
			}

			printf("RIT_IO: create %s for error dump\r\n", path);

			/* get rit config info for error dump */
			if (fread(&addr, 1, 4, fp_cfg) != 4) {
				printf("RIT_IO: failed to read rit cfg file\r\n");
				goto ERROR;
			}
			if (fread(&len, 1, 4, fp_cfg) != 4) {
				printf("RIT_IO: failed to read rit cfg file\r\n");
				goto ERROR;
			}

			if (fwrite((void*)((uint64_t)ctx->baseaddr + addr), 1, len, fp_dump) != len) {
				printf("RIT_IO: failed to write error dump file\r\n");
				goto ERROR;
			}

			fclose(fp_dump);
			fp_dump = NULL;
			cfg_len -= 8;
			i++;
		}
		printf("RIT_IO:  error dump done!\r\n");
	} else {
		printf("RIT_IO:  test pass!\r\n");
	}

	fclose(fp_cfg);
	fp_cfg = NULL;
//	free(dirname);

	return 0;
ERROR:
	if (fp_dump != NULL) {
		fclose(fp_dump);
	}
	if (fp_cfg != NULL) {
		fclose(fp_cfg);
	}
	printf("RIT_IO: fail\r\n");
	return -1;
}

int rit_agent_init(struct vmctx *ctx, const char* dir)
{
	struct inout_port *io;

	dirname = strdup(dir);

	io = &ra_inst.io_ctl;
	io->name = "rit_io_ctl";
	io->port = 0xf8;
	io->size = 2;
	io->flags = IOPORT_F_INOUT;
	io->handler = rit_io_ctl_handler;
	assert(register_inout(io) == 0);

	io = &ra_inst.io_0x83;
	io->name = "rit_io_x83";
	io->port = 0x83;
	io->size = 1;
	io->flags = IOPORT_F_INOUT;
	io->handler = rit_io_0x83_handler;
	assert(register_inout(io) == 0);

	return 0;
}