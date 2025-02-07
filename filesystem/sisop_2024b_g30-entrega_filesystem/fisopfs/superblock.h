#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include "inode.h"

#define DEFAULT_FILE_DISK "persistence_file.fisopfs"
#define PATH_FS "sisopG30Group.fisopfs"

// This module contains the superblock structure and its functions

#define NINODES 1024

// inode bitmap values
enum { FREE, USED };

#define MAX_DEPTH 3

#define BLOCK_SIZE 4096

// superblock structure defines
typedef struct superblock {
	inode_t *inodes[NINODES];
	int inode_bitmap[NINODES];
	int inode_number;
} superblock_t;

extern superblock_t superblock;

#endif