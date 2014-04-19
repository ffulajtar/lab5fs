
#ifndef LAB5FS_INODE_H
#define LAB5FS_INODE_H

#include <linux/fs.h>


/* custom lab5fs meta-data inside each VFS inode. */
struct lab5fs_inode_info {
        unsigned long  i_block_num;     /* block containing the inode.               */
       	unsigned long  i_bi_block_num;  /* block containing the inode's data index.  */
};

/* Macro for getting lab5fs inode meta-data from a VFS inode. */
#define LAB5FS_INODE_INFO(ino) ((struct lab5fs_inode_info*)((ino)->u.generic_ip))

/*
 * Given a VFS inode and the inode's block on disk, read the inode's contents
 * into memory. The inode number is supplied inside the VFS inode struct.
 * @return 0 on success, a negative error code on failure.
 */
int lab5fs_inode_read_ino (struct inode *, unsigned long);

/*
 * Clear and release any dynamically allocated memory used by the VFS
 * inode structure
 */
void lab5fs_inode_clear(struct inode *);

#endif /* LAB5FS_INODE_H */
