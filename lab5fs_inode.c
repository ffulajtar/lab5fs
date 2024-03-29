#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/types.h>
#include "lab5fs.h"
#include "lab5fs_super.h"
#include "lab5fs_inode.h"

/* inode operations go here*/
struct inode_operations inode_ops;

/* file operations go here*/ 
struct file_operations file_ops;

/* address operations go here*/
struct address_space_operations address_ops;

/*Read inode data from a block on disk and fill out a VFS inode*/
int lab5fs_inode_read_ino(struct inode *ino, unsigned long block_num){

        int err = -ENOMEM;
        struct super_block *sb = ino->i_sb;
        struct buffer_head *ibh = NULL;
        struct lab5fs_inode *lab5fs_ino = NULL;
        unsigned long bi_block_num = 0;
        struct lab5fs_inode_info *inode_meta = NULL;


        printk("Reading inode %ld\n", ino->i_ino);

        /* read the inode's block from disk. */
        if (!(ibh = sb_bread(sb,block_num))) {
                printk("Unable to read inode block %lu.\n", block_num);
                goto ret_err;
        }
        lab5fs_ino = (struct lab5fs_inode *)((char *)(ibh->b_data));

        bi_block_num = le32_to_cpu(lab5fs_ino->i_data_index_block_num);

        printk("Inode %ld, block_index_num=%lu\n",
                             ino->i_ino, bi_block_num);

        /* initialize the inode's meta data. */
        inode_meta = kmalloc(sizeof(struct lab5fs_inode_info),GFP_KERNEL);
        if (inode_meta==NULL) {
                printk("Not enough memory to allocate inode meta struct.\n");
                goto ret_err;
        }
        inode_meta->i_block_num = block_num;
        inode_meta->i_bi_block_num = bi_block_num;

	/* fill out VFS inode*/ 
        ino->i_mode = le16_to_cpu(lab5fs_ino->i_mode);
        ino->i_nlink = le16_to_cpu(lab5fs_ino->i_link_count);
        ino->i_size = le32_to_cpu(lab5fs_ino->i_size);
        ino->i_blksize = LAB5FS_BLOCK_SIZE;
        ino->i_blkbits = 10;
        ino->i_blocks = le32_to_cpu(lab5fs_ino->i_num_blocks);
        ino->i_uid = le32_to_cpu(lab5fs_ino->i_uid);
        ino->i_gid = le32_to_cpu(lab5fs_ino->i_gid);
        ino->i_atime.tv_sec = le32_to_cpu(lab5fs_ino->i_atime);
        ino->i_mtime.tv_sec = le32_to_cpu(lab5fs_ino->i_mtime);
        ino->i_ctime.tv_sec = le32_to_cpu(lab5fs_ino->i_ctime);
        ino->u.generic_ip = inode_meta;

        /* set the inode operations structs  */
        /* TODO: Implement actual operations */
        ino->i_op = &inode_ops;
        ino->i_fop = &file_ops;
        ino->i_mapping->a_ops = &address_ops;

        printk(    "Inode %ld: i_mode=%o, i_nlink=%d, "
                   "i_uid=%d, i_gid=%d\n",
                   ino->i_ino, ino->i_mode, ino->i_nlink,
                   ino->i_uid, ino->i_gid);
	return 0;

ret_err:
	if (ibh)
                brelse(ibh);
        return err;	
}

/*Free memory used by VFS inode object*/
void lab5fs_inode_clear(struct inode *ino){
	struct lab5fs_inode_info *inode_info = LAB5FS_INODE_INFO(ino);
	kfree(inode_info);
	ino->u.generic_ip = NULL;
}
