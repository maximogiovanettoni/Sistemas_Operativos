#ifndef INODE_H
#define INODE_H

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#define DATA_INIT_SIZE 1024
#define MAX_DATA_SIZE 16384
#define MAX_PATH 256

enum inode_type { INODE_FILE, INODE_DIR };

typedef struct inode {
	enum inode_type type;
	char path[MAX_PATH];
	char *data;
	size_t data_size;
	size_t data_len;
	nlink_t nlink;
	uid_t uid;
	gid_t gid;
	time_t a_time;
	time_t m_time;
	time_t c_time;
	mode_t file_mode;
} inode_t;


// Devuelve la posicion del inodo libre en el super bloque
int get_free_inode();

// Devuelve la posicion del bloque libre en el super bloque
int search_inode_by_path(const char *path);

//Devuelve el offset de la siguiente línea en el archivo
int read_line(const char *data, char *buffer, off_t offset);

//Devuelve la siguiente entrada de directorio
void get_next_entry(char *data, off_t *offset, char *buffer);

//Devuelve el path del padre
char *get_parent_path(const char path[MAX_PATH]);

//Devuelve el índice del padre en la tabla de inodos
int get_parent(const char path[MAX_PATH]);

//
int add_dentry(int inode_index, const char *path);

//
void remove_inode(const char *path, int inode_index);

//
int remove_parent_dentry(const char *path);

//
int remove_dentry(int inode_index, const char *dentry);

//
int add_parent_dentry(const char *path, enum inode_type type, mode_t mode);

// devuelve la posicion del bloque creado en el super bloque
int create_inode(const char *path, enum inode_type i_type, mode_t file_mode);

int write_inode_data(const char *path, const char *buf, size_t size, off_t offset);

int read_inode_data(const char *path, char *buf, size_t size, off_t offset);

int truncate_inode_data(const char *path, off_t size);

int data_reallocation(char **data, size_t data_size, size_t size_needed);

//
int check_depth(const char *path);

//
int serialize();

//
int deserialize();


#endif