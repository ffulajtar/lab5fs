#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include "lab5fs.h"
#include "lab5fs_super.h"
#include "lab5fs_inode.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sourav Chakraborty");


static struct super_block* lab5fs_get_sb(struct file_system_type *fs_type,
				   int flags,
				   const char *dev_name,
				   void *data)
{
	return get_sb_bdev(fs_type, flags, dev_name, data, lab5fs_fill_super);
}

static void lab5fs_kill_sb(struct super_block *sb)
{
	printk("Unmounting lab5fs\n");
	kill_block_super(sb);
}

struct file_system_type lab5fs_fs_type = {
	.owner = THIS_MODULE,
	.name = "lab5fs",
	.fs_flags = FS_REQUIRES_DEV,
	.get_sb = lab5fs_get_sb,
	.kill_sb = lab5fs_kill_sb
};

static int __init init_lab5fs(void)
{
	int r;

	printk("Initializing module lab5fs\n");
	r = register_filesystem(&lab5fs_fs_type);
	if(r) {
		printk("Error registering lab5fs: %d\n", r);
	}

	return r;
}

static void __exit exit_lab5fs(void)
{
	unregister_filesystem(&lab5fs_fs_type);
	printk("Cleaning up module lab5fs\n");
}

module_init(init_lab5fs);
module_exit(exit_lab5fs);

