#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include "lab5fs.h"
#include "lab5fs_super.h"
#include "lab5fs_inode.h"


/*MACRO for accessing the superblock info pointer*/
#define LAB5FS_SB_INFO(sb) ((struct lab5fs_sb_info*)((sb)->s_fs_info))

/*function prototypes for super block operations*/
void lab5fs_read_inode (struct inode *);
void lab5fs_clear_inode (struct inode *);
void lab5fs_put_super (struct super_block *);

/*Note: still need to actually implement these functions*/
struct super_operations lab5fs_super_ops ={
	read_inode: lab5fs_read_inode,
	clear_inode: lab5fs_clear_inode,
	put_super: lab5fs_put_super,
};

/* Store custom metadata about filesystem*/
struct lab5fs_sb_info {
	/*lab5fs super block*/
	struct buffer_head *s_sbh;
        struct lab5fs_super_block *s_lab5fs_sb;

	/*lab5fs block bitmap*/
        struct buffer_head *s_block_bitmap_bh;
        struct lab5fs_bitmap *s_lab5fs_block_bitmap;

	/*lab5fs inode bitmap*/
        struct buffer_head *s_inode_bitmap_bh;
        struct lab5fs_bitmap *s_lab5fs_inode_bitmap;

	/*lab5fs inode table*/
	struct buffer_head *s_inode_table_bh;
	struct lab5fs_inode_table *s_lab5fs_inode_table;
};


/*Locate the block number of an inode given its inode number*/
unsigned long lab5fs_find_block_num(struct inode *ino)
{
        unsigned long ino_num = ino->i_ino;
        unsigned long block_num = 0;
        struct lab5fs_inode_table *inode_table;

        if (ino_num > LAB5FS_MAX_INODE_COUNT) {
             printk("inode number '%lu' is out of range\n", ino_num);
             return 0;
        }

        inode_table = LAB5FS_SB_INFO(ino->i_sb)->s_lab5fs_inode_table;
        block_num = le32_to_cpu(inode_table->inodes[ino_num]);
        printk("inode number '%lu' is on block %lu\n", ino_num, block_num);

        return block_num;
}


/* Fill in vfs superblock from lab5fs image*/
int lab5fs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct buffer_head *bh,*bb_bh,*ib_bh,*it_bh;
	struct lab5fs_super_block *disk_sb;
	struct lab5fs_bitmap *disk_block_bitmap;
	struct lab5fs_bitmap *disk_inode_bitmap;
	struct lab5fs_inode_table *disk_inode_table;
	struct inode *inode;
	struct lab5fs_sb_info *metadata;
	printk("Mounting lab5fs\n");

	/*init buffer heads and read data from disk*/
	sb_set_blocksize(sb, LAB5FS_BLOCK_SIZE);
	bh = sb_bread(sb, 0);
	disk_sb = (struct lab5fs_super_block*)bh->b_data;
	printk("magic: %0x, free: %0x\n", disk_sb->s_magic, disk_sb->s_free_blocks_count);

	if(!(bb_bh = sb_bread(sb, LAB5FS_BLOCK_BITMAP_NUM))){
		printk("Unable to read block bitmap\n");
	}
	disk_block_bitmap = (struct lab5fs_bitmap*)bb_bh->b_data;
	
	if(!(ib_bh = sb_bread(sb, LAB5FS_INODE_BITMAP_NUM))){
		printk("Unable to read inode bitmap");
	}
	disk_inode_bitmap = (struct lab5fs_bitmap*) ib_bh->b_data;
	
	if(!(it_bh = sb_bread(sb, LAB5FS_INODE_TABLE_NUM))){
		printk("Unable to read inode table");
	}
	disk_inode_table = (struct lab5fs_inode_table*) it_bh->b_data;

	/* set up superblock meta data*/
	metadata = kmalloc(sizeof(struct lab5fs_sb_info), GFP_KERNEL);
	if(metadata == NULL)
	{
		printk("Not enough memory to allocate super block struct.\n");
	}
	metadata->s_sbh = bh;
	metadata->s_lab5fs_sb = disk_sb;
	metadata->s_block_bitmap_bh = bb_bh;
	metadata->s_lab5fs_block_bitmap = disk_block_bitmap;
	metadata->s_inode_bitmap_bh = ib_bh;
	metadata->s_lab5fs_inode_bitmap = disk_inode_bitmap;	
	metadata->s_inode_table_bh = it_bh;
	metadata->s_lab5fs_inode_table = disk_inode_table;
	
	/*fill vfs super block*/
	sb->s_maxbytes = LAB5FS_MAX_SIZE;
	sb->s_blocksize = LAB5FS_BLOCK_SIZE;
	sb->s_blocksize_bits = LAB5FS_BITS;
	sb->s_magic = LAB5FS_SUPER_MAGIC;
	sb->s_op = &lab5fs_super_ops;
	sb->s_fs_info = metadata;

	/*load root inode*/
	inode = iget(sb,0);
	sb->s_root = d_alloc_root(inode);

	return 0;
}

void lab5fs_read_inode (struct inode *ino)
{
        unsigned long block_num = 0;

        /* find the inode's block number. */
        block_num = lab5fs_find_block_num(ino);
        if (block_num == 0)
		printk("Error reading inode\n");
        lab5fs_inode_read_ino(ino, block_num); /*function defined in lab5fs_inode.c*/
}

/*Free bufferheads and release memory*/
void lab5fs_put_super(struct super_block *sb){
	struct lab5fs_sb_info *sb_info = LAB5FS_SB_INFO(sb);
	printk("Releasing VFS super block\n");
	brelse(sb_info->s_sbh);
	brelse(sb_info->s_block_bitmap_bh);
	brelse(sb_info->s_inode_bitmap_bh);
	brelse(sb_info->s_inode_table_bh);
	kfree(sb_info);	
	sb->s_fs_info = NULL;
}


/*Release an inode and clear memory used by inode*/
void lab5fs_clear_inode (struct inode * ino){
	printk("Releasing inode #%ld from VFS\n",ino->i_ino);
	lab5fs_inode_clear(ino); /*function defined in lab5fs_inode.c*/
}
