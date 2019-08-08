#include "cuju_module.h"

struct cuju_vhost_entry cuju_vhost;
EXPORT_SYMBOL_GPL(cuju_vhost);

int cuju_vhost_set_ftmode(int flag)
{
	cuju_vhost.ftmode = flag;
	return cuju_vhost.ftmode;
}
EXPORT_SYMBOL_GPL(cuju_vhost_set_ftmode);

void cuju_vhost_set_epoch(void)
{
	/*
	 * new cuju_vhost_buffer add
	 */
	cuju_vhost.bnum = ((cuju_vhost.bnum + 1) % CUJU_VHOST_MAX_BUFFER);

	spin_lock_irq(&cuju_vhost.sef.lock);
	cuju_vhost.sef.epo++;
	spin_unlock_irq(&cuju_vhost.sef.lock);

}
EXPORT_SYMBOL_GPL(cuju_vhost_set_epoch);

static int cuju_vhost_flush(void)
{
	struct socket *sock;
	int err, i;
	int knum, fbnum;

	sock = cuju_vhost.sock;

	if (!sock)
		printk("--- cuju no socket\n");

	fbnum = cuju_vhost.fbnum;
	knum = cuju_vhost.buf[fbnum].knum;

	for (i = 0; i < knum; i++) {
		err = sock->ops->sendmsg(sock,
			 cuju_vhost.buf[fbnum].kick[i].msg,
			 cuju_vhost.buf[fbnum].kick[i].len);

		if (err != cuju_vhost.buf[fbnum].kick[i].len) {
			printk("--- cuju vhost  err = %d\n", err);
		}

		cuju_vhost.buf[fbnum].kick[i].len = 0;
		kfree(cuju_vhost.buf[fbnum].kick[i].msg);
	}

	cuju_vhost.buf[fbnum].knum = 0;

	fbnum = ((fbnum + 1) % CUJU_VHOST_MAX_BUFFER);
	cuju_vhost.fbnum = fbnum;

	spin_lock_irq(&cuju_vhost.sef.lock);
	cuju_vhost.sef.flu--;
	spin_unlock_irq(&cuju_vhost.sef.lock);

	return i;
}

static int INIT_CUJU_VHOST_FT(void)
{
	int i, j;

	/*
	 * cuju_vhost_buffer init
	 */
	cuju_vhost.buf = kmalloc(
		(sizeof(struct cuju_vhost_buffer) * CUJU_VHOST_MAX_BUFFER),
		GFP_KERNEL);

	if (!cuju_vhost.buf)
		printk("--- cuju_vhost.buf NULL\n");

	for(i = 0; i < CUJU_VHOST_MAX_BUFFER; i++) {
		/*
		 * cuju_vhost_kick init
		 */
		cuju_vhost.buf[i].kick = kmalloc(
			(sizeof(struct cuju_vhost_kick) * CUJU_VHOST_MAX_KICK),
			GFP_KERNEL);

		if (!cuju_vhost.buf[i].kick)
			printk("--- cuju_vhost.buf[%d].kick NULL\n", i);

		for(j = 0; j < CUJU_VHOST_MAX_KICK; j++) {
			cuju_vhost.buf[i].kick[j].msg = NULL;
			cuju_vhost.buf[i].kick[j].len = 0;
		}
		cuju_vhost.buf[i].knum = 0;
	}

	/*
	 * cuju_vhost_entry init
	 */
	cuju_vhost.bnum = 0;
	cuju_vhost.fbnum = 0;
	cuju_vhost.fknum = 0;

	cuju_vhost.sef.epo = 0;
	cuju_vhost.sef.flu = 0;
	spin_lock_init(&cuju_vhost.sef.lock);

	cuju_vhost.sock = NULL;
	cuju_vhost.ftmode = 0;

	return 0;
}

static void RELEASE_CUJU_VHOST_FT(void)
{
	int i, j;

	for(i = 0; i < CUJU_VHOST_MAX_BUFFER; i++) {
		for(j = 0; j < CUJU_VHOST_MAX_KICK; j++) {
			if (cuju_vhost.buf[i].kick[j].msg)
				kfree(cuju_vhost.buf[i].kick[j].msg);
			cuju_vhost.buf[i].kick[j].len = 0;
		}
		cuju_vhost.buf[i].knum = 0;
		kfree(cuju_vhost.buf[i].kick);
	}
	kfree(cuju_vhost.buf);
}

int cuju_vhost_snapshot(int arg)
{
	int ret, n;

	spin_lock_irq(&cuju_vhost.sef.lock);
	ret = cuju_vhost.sef.epo;
	if (cuju_vhost.sef.flu) {
		spin_unlock_irq(&cuju_vhost.sef.lock);
		n = cuju_vhost_flush();
		printk("flush success = %d\n", n);
	} else
		spin_unlock_irq(&cuju_vhost.sef.lock);

	return ret;
}
EXPORT_SYMBOL_GPL(cuju_vhost_snapshot);

void cuju_vhost_set_flush(int arg)
{
	spin_lock_irq(&cuju_vhost.sef.lock);
	cuju_vhost.sef.epo--;
	cuju_vhost.sef.flu++;
	spin_unlock_irq(&cuju_vhost.sef.lock);
}
EXPORT_SYMBOL_GPL(cuju_vhost_set_flush);

void cuju_vhost_add_buffer(struct msghdr *msg, int len)
{
	int bnum, knum;

	bnum = cuju_vhost.bnum;
	knum = cuju_vhost.buf[bnum].knum;

	cuju_vhost.buf[bnum].kick[knum].msg = kmalloc(sizeof(struct msghdr),
						      GFP_KERNEL);
	if (!cuju_vhost.buf[bnum].kick[knum].msg)
		printk("--- cuju vhost no buffer msg!!\n");

	memcpy(cuju_vhost.buf[bnum].kick[knum].msg,
		msg,
		sizeof(struct msghdr));

	cuju_vhost.buf[bnum].kick[knum].len = len;
	cuju_vhost.buf[bnum].knum++;
}
EXPORT_SYMBOL_GPL(cuju_vhost_add_buffer);

static int cuju_module_init(void)
{
        printk("cuju_module init\n");
	return INIT_CUJU_VHOST_FT();
}

static void cuju_module_exit(void)
{
	RELEASE_CUJU_VHOST_FT();
        printk("cuju_module exit\n");
}

module_init(cuju_module_init);
module_exit(cuju_module_exit);

MODULE_VERSION("1.0");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("WU_CI_JYUN");
MODULE_DESCRIPTION("vhost-net support cuju ft-mode");
