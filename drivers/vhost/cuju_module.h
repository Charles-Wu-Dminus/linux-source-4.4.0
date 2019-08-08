#ifndef _CUJU_MODULE_TEST_H
#define _CUJU_MODULE_TEST_H

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <stdbool.h>
#include <linux/mutex.h>
#include <net/sock.h>
#include "vhost.h"

#define CUJU_VHOST_MAX_KICK 50
#define CUJU_VHOST_MAX_BUFFER 20

struct cuju_vhost_kick {
	struct msghdr *msg;
	int len;
};
struct cuju_vhost_buffer {
	struct cuju_vhost_kick *kick;
	/*
	 * knum : number of kick
	 */
	int knum;
};
struct cuju_vhost_set_epoch_flush {
	/*
	 * epo : set epoch - If have buffer, set_epoch++.
	 * flu : set flush - If have epoch, set_flush++ and set_epoch--.
	 */
	int epo;
	int flu;
	spinlock_t lock;
};

struct cuju_vhost_entry {

	struct cuju_vhost_buffer *buf;

	/*
	 * Buffer the buffer number.
	 */
	int bnum;

	/*
	 * Flush the buffer number.
	 */
	int fbnum;

	/*
	 * Flush the kick number.
	 */
	int fknum;

	struct cuju_vhost_set_epoch_flush sef;
	void *sock;
	int ftmode;
};

extern struct cuju_vhost_entry cuju_vhost;

int cuju_vhost_set_ftmode(int flag);
void cuju_vhost_set_flush(int arg);
int cuju_vhost_snapshot(int flag);

void cuju_vhost_set_epoch(void);
void cuju_vhost_add_buffer(struct msghdr *msg, int len);

#endif
