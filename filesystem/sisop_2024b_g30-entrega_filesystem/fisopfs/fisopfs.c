#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "inode.h"
#include "superblock.h"

char *filedisk = DEFAULT_FILE_DISK;

superblock_t superblock = {};

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] ENTRÓ fisopfs_getattr - path: %s\n", path);

	int inode_number = search_inode_by_path(path);
	if (inode_number < 0) {
		printf("[debug] NO ENCONTRÓ INODO: %s fisopfs_getattr\n", path);
		return -ENOENT;
	}

	inode_t *inode = superblock.inodes[inode_number];

	st->st_ino = inode_number;
	st->st_nlink = inode->nlink;
	st->st_uid = inode->uid;
	st->st_gid = inode->gid;
	st->st_atime = inode->a_time;
	st->st_mtime = inode->m_time;
	st->st_ctime = inode->c_time;

	st->st_blksize = BLOCK_SIZE;
	int n_blocks = inode->data_size / BLOCK_SIZE;
	int block_fraction = inode->data_size % BLOCK_SIZE;
	st->st_blocks = n_blocks + (block_fraction > 0 ? 1 : 0);

	if (inode->type == INODE_DIR) {
		st->st_mode = __S_IFDIR | 0755;
	} else {
		st->st_mode = __S_IFREG | 0644;
		st->st_size = inode->data_len;
	}
	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] ENTRÓ fisopfs_readdir - path: %s\n", path);

	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);


	int inode_number = search_inode_by_path(path);

	if (inode_number < 0) {
		printf("[debug] Directory not found.");
		return -ENOENT;
	}

	inode_t *inode = superblock.inodes[inode_number];

	if (inode->type != INODE_DIR) {
		printf("[debug] Path is not a directory");
		return -ENOTDIR;
	}

	inode->a_time = time(NULL);

	int len_data = strlen(inode->data);
	while (offset < len_data) {
		char buffer_name[MAX_PATH];
		get_next_entry(inode->data, &offset, buffer_name);
		filler(buffer, buffer_name, NULL, 0);
	}

	return 0;
}


static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] ENTRÓ fisopfs_mkdir - path: %s\n", path);
	int success = add_parent_dentry(path, INODE_DIR, mode);

	if (success < 0) {
		printf("[debug] dentry could't be added");
		return success;
	}

	return 0;
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] ENTRÓ fisopfs_read - path: %s, offset: %lu, size: "
	       "%lu\n",
	       path,
	       offset,
	       size);


	int cant_read = read_inode_data(path, buffer, size, offset);

	return cant_read;
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] ENTRÓ fisopfs_rmdir - path: %s\n", path);

	int success = remove_parent_dentry(path);

	if (success < 0) {
		printf("[debug] dentry could't be removed.");
		return success;
	}

	return 0;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] ENTRÓ fisopfs_create - path: %s\n", path);

	int success = add_parent_dentry(path, INODE_FILE, mode);
	if (success < 0) {
		printf("[debug] parent dentry could't be added");
		return success;
	}


	return 0;
}


static int
fisopfs_unlink(const char *path)
{
	printf("[debug] ENTRÓ fisopfs_unlink - path: %s\n", path);
	int success = remove_parent_dentry(path);
	if (success < 0) {
		printf("[debug] dentry could't be removed.");
		return success;
	}

	return 0;
}

static int
fisopfs_write(const char *path,
              const char *buf,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] ENTRÓ fisopfs_write - path: %s\n", path);

	int bytes_written = write_inode_data(path, buf, size, offset);
	if (bytes_written < 0) {
		printf("[debug] Error writing data to inode");
		return bytes_written;
	}

	return bytes_written;
}

static int
fisopfs_utimens(const char *path, const struct timespec ts[2])
{
	printf("[debug] ENTRÓ utimens - path: %s\n", path);

	int inode_number = search_inode_by_path(path);
	if (inode_number < 0) {
		printf("[debug] NO ENCONTRÓ INODO: %s utimens\n", path);
		return inode_number;
	}

	inode_t *inode = superblock.inodes[inode_number];

	inode->a_time = ts[0].tv_sec;
	inode->m_time = ts[1].tv_sec;

	return 0;
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] ENTRÓ truncate\n");

	int error = truncate_inode_data(path, size);
	if (error < 0) {
		printf("[debug] Error truncating inode data\n");
	}
	return error;
}

static void
fisopfs_destroy(void *private_data)
{
	printf("[debug] ENTRÓ destroy\n");
	int error = serialize();
	if (error < 0) {
		printf("[debug] Error serializing filesystem\n");
	}
}


static int
fisopfs_open(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] ENTRÓ fisopfs_open - path: %s\n", path);

	int inode_number = search_inode_by_path(path);
	if (inode_number < 0) {
		printf("[debug] NO ENCONTRÓ INODO: %s open\n", path);
		return -ENOENT;
	}

	inode_t *inode = superblock.inodes[inode_number];

	if (inode->type != INODE_FILE) {
		printf("[debug] Path is not a file");
		return -EISDIR;
	}

	return 0;
}


static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] ENTRÓ fisopfs_fflush - path: %s\n", path);
	int error = serialize();
	if (error < 0) {
		printf("[debug] Error serializing filesystem\n");
	}
	return 0;
}

// Funciones Arriba
static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");

	int error = deserialize();
	if (error < 0) {
		printf("[debug] Error deserializing filesystem\n");
		int error = create_inode("/", INODE_DIR, 0777);
		if (error < 0) {
			printf("[debug] root directory could not be "
			       "created.\n");
			return NULL;
		}
		printf("[debug] Filesystem created!\n");
		return NULL;
	}
	printf("[debug] Filesystem loaded!\n");
	return NULL;
}


static struct fuse_operations operations = { .init = fisopfs_init,
	                                     .getattr = fisopfs_getattr,
	                                     .readdir = fisopfs_readdir,
	                                     .read = fisopfs_read,
	                                     .mkdir = fisopfs_mkdir,
	                                     .rmdir = fisopfs_rmdir,
	                                     .create = fisopfs_create,
	                                     .unlink = fisopfs_unlink,
	                                     .write = fisopfs_write,
	                                     .utimens = fisopfs_utimens,
	                                     .truncate = fisopfs_truncate,
	                                     .destroy = fisopfs_destroy,
	                                     .flush = fisopfs_flush

};


int
main(int argc, char *argv[])
{
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--filedisk") == 0) {
			filedisk = argv[i + 1];

			// We remove the argument so that fuse doesn't use our
			// argument or name as folder.
			// Equivalent to a pop.
			for (int j = i; j < argc - 1; j++) {
				argv[j] = argv[j + 2];
			}

			argc = argc - 2;
			break;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}
