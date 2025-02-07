#include "inode.h"
#include "superblock.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

int
data_reallocation(char **data, size_t data_size, size_t size_needed)
{
	if (data_size == 0) {
		data_size = DATA_INIT_SIZE;
	}
	if (!data || *data == NULL) {
		printf("[debug] Data is NULL or uninitialized\n");
		return data_size;
	}
	printf("[debug] Necesita: %ld\n", size_needed);
	printf("[debug] Tiene: %ld\n", data_size);
	if (data_size >= size_needed) {
		// no hace falta realocar
		printf("[debug] Data size is less than size needed\n");
		return data_size;
	}

	if (size_needed > MAX_DATA_SIZE) {
		printf("[debug] Exceeds maximum file size: %d\n", MAX_DATA_SIZE);
		return data_size;
	}

	size_t new_size = data_size;

	while (new_size < size_needed) {
		new_size *= 2;
	}

	char *new_data = realloc(*data, new_size);
	if (!new_data) {
		printf("[debug] Data reallocation failed!\n");
		return data_size;
	}

	*data = new_data;

	return new_size;
}

int
search_inode_by_path(const char *path)
{
	for (int i = 0; i < NINODES; i++) {
		if (superblock.inode_bitmap[i] == FREE) {
			continue;
		}

		if (strcmp(superblock.inodes[i]->path, path) == 0) {
			printf("[debug] Path: %s found!\n", path);
			return i;
		}
	}
	printf("[debug] Path: %s not found\n", path);
	return -ENOENT;
}

int
get_free_inode()
{
	int i = 0;
	while (i < NINODES && superblock.inode_bitmap[i] != FREE) {
		i++;
	}
	return i;
}


// returns offset for next line
int
read_line(const char *data, char *buffer, off_t offset)
{
	int i = 0;
	printf("entro al while (read Line) \n");
	while (data[offset] != '\n' && data[offset] != '\0') {
		buffer[i] = data[offset];
		i++;
		offset++;
	}
	printf("salio del while\n");
	if (data[offset] == '\0' && i == 0) {
		return 0;
	}
	buffer[i++] = '\0';
	printf("buffer: %s\n", buffer);
	return i;
}

void
get_next_entry(char *data, off_t *offset, char *buffer)
{
	off_t leer = read_line(data, buffer, *offset);
	*offset += leer;
}

char *
get_parent_path(const char path[MAX_PATH])
{
	char *parent = malloc(sizeof(char) * (strlen(path) + 1));
	strcpy(parent, path);
	char *leading_slash = strrchr(parent, '/');
	if (leading_slash == parent) {
		leading_slash++;
	}
	*leading_slash = '\0';
	return parent;
}

// Returns the parent inode index in inode table
int
get_parent(const char path[MAX_PATH])
{
	char *parent_path = get_parent_path(path);
	int parent_index = search_inode_by_path(parent_path);
	free(parent_path);
	return parent_index;
}

void
remove_inode(const char *path, int inode_index)
{
	if (inode_index < 0) {
		printf("[debug] Inode not found when trying to remove inode\n");
		return;
	}

	inode_t *inode = superblock.inodes[inode_index];
	if (inode->type == INODE_DIR && inode->nlink > 2) {
		printf("[debug] Directory Inode has more than one hard link\n");
		return;
	}

	free(inode->data);
	free(inode);
	superblock.inode_number--;
	superblock.inodes[inode_index] = NULL;
	superblock.inode_bitmap[inode_index] = FREE;
}

int
add_dentry(int inode_index, const char *dentry)
{
	inode_t *inode_dir = superblock.inodes[inode_index];
	if (!inode_dir || !dentry) {
		return -EINVAL;
	}

	int len_dentry = strlen(dentry);

	int current_len = inode_dir->data_len;

	int current_size = inode_dir->data_size;

	int size_needed = current_len + len_dentry + 1;

	int new_size = current_size;

	if (size_needed > current_size) {
		new_size = data_reallocation(&(inode_dir->data), current_size, size_needed );
		if (new_size == current_size) {
			return -ENOMEM;
		}
	}

	strcat(inode_dir->data, dentry);
	strcat(inode_dir->data, "\n");

	inode_dir->data_size = new_size;
	inode_dir->data_len = size_needed;

	inode_dir->nlink++;

	return 0;
}

int
remove_dentry(int inode_index, const char *dentry)
{
	inode_t *parent = superblock.inodes[inode_index];

	if (!parent) {
		printf("[debug] Parent inode not found when trying to remove "
		       "dentry\n");
		return -EEXIST;
	}
	if (parent->type != INODE_DIR) {
		printf("[debug] Parent inode is not a directory\n");
		return -ENOTDIR;
	}

	int len_data = parent->data_len;
	if (len_data == 0) {
		printf("[debug] Parent directory is empty\n");
		return -ENOENT;
	}

	char *new_data = calloc(len_data + 1, sizeof(char));
	if (!new_data) {
		printf("[debug] Memory allocation failed\n");
		return -ENOMEM;
	}

	off_t offset = 0;

	while (offset < len_data) {
		char buffer[MAX_PATH + 1];
		get_next_entry(parent->data, &offset, buffer);
		printf("[debug] buffer: %sTermina buffer\n", buffer);
		
		if (strcmp(buffer, dentry) != 0) {
			strcat(new_data, buffer);
			strcat(new_data, "\n");
		} else {
			printf("[debug] dentry a remover en buffer: %s\n", buffer);
		}
	}

	printf("[debug] new_data after removing dentry %s: %s\n", dentry, new_data);

	int new_data_len = parent->data_len - strlen(dentry) - 1;

	memcpy(parent->data, new_data, new_data_len);

	parent->data[new_data_len] = '\0';

	parent->data_len = new_data_len;
	parent->nlink--;
	free(new_data);

	return 0;
}

int
remove_parent_dentry(const char *path)
{
	int inode_index = search_inode_by_path(path);
	if (inode_index < 0) {
		printf("[debug] Path: %s not found when trying to remove dentry\n",
		       path);
		return inode_index;
	}

	inode_t *inode = superblock.inodes[inode_index];
	if (inode->data[0] != '\0' && inode->type == INODE_DIR) {
		printf("[debug] Cannot remove non-empty directory %s\n", path);
		return -ENOTEMPTY;
	}

	int parent_index = get_parent(path);
	if (parent_index < 0) {
		printf("[debug] Parent not found when attempting to remove "
		       "dentry\n");
		return -ENOENT;
	}
	printf("[debug] Parent index: %d\n", parent_index);
	char *dentry = malloc(sizeof(char) * MAX_PATH);
	strcpy(dentry, path);
	char *dentry_to_remove = basename(dentry);
	
	remove_dentry(parent_index, dentry_to_remove);
	free(dentry);
	remove_inode(path, inode_index);

	return 0;
}


int
add_parent_dentry(const char *path, enum inode_type type, mode_t mode)
{
	char *parent_path = get_parent_path(path);

	printf("[debug] Parent path: %s\n", parent_path);
	int parent_inode_number = search_inode_by_path(parent_path);
	free(parent_path); 

	if (parent_inode_number < 0) {
		printf("[debug] Parent directory not found\n");
		return parent_inode_number;
	}

	int inode_number = search_inode_by_path(path);
	if (inode_number >= 0) {
		printf("[debug] Directory already exists\n");
		return -EEXIST;
	}

	int inode = create_inode(path, type, mode);

	if (inode < 0) {
		printf("[debug] inode could't be created\n");
		return inode;
	}

	inode_t *parent_inode = superblock.inodes[parent_inode_number];

	if (parent_inode->type != INODE_DIR) {
		printf("[debug] Parent is not a directory\n");
		return -ENOTDIR;
	}

	char *dentry = strrchr(path, '/');
	dentry++;

	if (add_dentry(parent_inode_number, dentry) < 0) {
		printf("[debug] Error adding dentry to parent\n");
		return -ENOSPC;
	}

	return 0;
}

int write_inode_data(const char *path, const char *buf, size_t size, off_t offset) { 

    int inode_number = search_inode_by_path(path);
    if (inode_number < 0) {
        printf("[debug] NO ENCONTRÓ INODO: %s write\n", path);
        return inode_number;
    }

    inode_t *inode = superblock.inodes[inode_number];

    if (inode->type != INODE_FILE) {
        printf("[debug] Path is not a file\n");
        return -EISDIR;
    }

    size_t current_size = inode->data_size;
    size_t size_needed = offset + size;

	printf("[debug] offset: %ld\n", offset);
	printf("[debug] size: %ld\n", size);
	printf("[debug] current_size: %ld\n", current_size);

    if (size_needed > current_size) {
        size_t new_size = data_reallocation(&(inode->data), current_size, size_needed);
        if (new_size == current_size) {
            printf("[debug] Error reallocating data\n");
            return -ENOMEM;
        }
        inode->data_size = new_size;
    }

    memcpy(inode->data + offset, buf, size);

	inode->data[offset + size] = '\0';

    if (offset + size > inode->data_len) {
        inode->data_len = offset + size;
    }

    inode->m_time = time(NULL);

    return size;
}

int
read_inode_data(const char *path, char *buf, size_t size, off_t offset)
{
	int inode_number = search_inode_by_path(path);
	if (inode_number < 0) {
		printf("[debug] NO ENCONTRÓ INODO: %s read\n", path);
		return inode_number;
	}

	inode_t *inode = superblock.inodes[inode_number];

    if (inode->data_len == 0) {
        return 0;
    }

	if (inode->type != INODE_FILE) {
		printf("[debug] Path is not a file\n");
		return -EISDIR;
	}

	if (offset + size > inode->data_len) {
		size = inode->data_len - offset;
	}

	size = size > 0 ? size : 0;

	memcpy(buf, inode->data + offset, size);

	inode->a_time = time(NULL);

	return size;
}

int
truncate_inode_data(const char *path, off_t size)
{
	int inode_number = search_inode_by_path(path);
	if (inode_number < 0) {
		printf("[debug] NO ENCONTRÓ INODO: %s truncate\n", path);
		return inode_number;
	}

	inode_t *inode = superblock.inodes[inode_number];

	if (inode->type != INODE_FILE) {
		printf("[debug] Path is not a file\n");
		return -EISDIR;
	}

	if (size > MAX_DATA_SIZE * sizeof(char)) {
		printf("[debug] Exceeds maximum file size: %d\n", MAX_DATA_SIZE);
		return -EFBIG;
	}

	size_t current_size = inode->data_size;

	if (size > current_size) {
		size_t new_size = data_reallocation(&(inode->data), inode->data_size, size);
		if (new_size == current_size){
			printf("[debug] Error reallocating data\n");
			return -ENOMEM;
		}
		inode->data_size = new_size;
		inode->m_time = time(NULL);
		return 0;
	}

	if (size == 0) {
		free(inode->data);
		char *new_data = calloc(DATA_INIT_SIZE, sizeof(char));
		if (!new_data) {
			printf("[debug] Error allocating memory for new data\n");
			return -ENOMEM;
		}
		inode->data = new_data;
		inode->data_size = DATA_INIT_SIZE;
		inode->data_len = 0;
		inode->m_time = time(NULL);
		return 0;
	}

	if (inode->data_len > size) {
		inode->data[size] = '\0';
		inode->m_time = time(NULL);
		return 0;
	}

	return 0;
}


int
check_depth(const char *path)
{
	int depth = 0;
	while (*path != '\0') {
		if (*path == '/') {
			depth++;
		}
		path++;
	}
	if (depth > MAX_DEPTH) {
		return -ENAMETOOLONG;
	}
	return 0;
}


int
create_inode(const char *path, enum inode_type i_type, mode_t file_mode)
{
	if (i_type == INODE_DIR && check_depth(path) < 0) {
		printf("[debug] Max directory depth exceeded");
		return -ENOSPC;
	}

	if (!path) {
		printf("[debug] Path is NULL\n");
		return -EINVAL;
	}

	if (strlen(path) > MAX_PATH) {
		printf("[debug] Path is too long\n");
		return -ENAMETOOLONG;
	}

	int i = get_free_inode();

	if (i == NINODES) {
		printf("[debug] No free inodes available\n");
		return -ENOSPC;
	}

	inode_t *new_inode = malloc(sizeof(inode_t));
	if (!new_inode) {
		printf("[debug] Memory allocation for new inode failed\n");
		return -ENOMEM;
	}

	strncpy(new_inode->path, path, sizeof(new_inode->path) - 1);

	printf("[debug] Path de creacion: %s\n", new_inode->path);

	char *data = malloc(sizeof(char) * DATA_INIT_SIZE);
	if (!data) {
		printf("[debug] Memory allocation for data failed\n");
		return -ENOMEM;
	}
	new_inode->data_size = sizeof(char) * DATA_INIT_SIZE;

	data[0] = '\0';
	new_inode->data_len = 0;
	new_inode->data = data;
	
	new_inode->type = i_type;
	new_inode->nlink = 1;

	if (i_type == INODE_DIR) {
		new_inode->nlink++;  // Directory inode initializes with 2 hard links (. and ..)
	}
	new_inode->file_mode = file_mode;

	time_t current_time = time(NULL);
	new_inode->a_time = current_time;
	new_inode->m_time = current_time;
	new_inode->c_time = current_time;

	new_inode->gid = getgid();
	new_inode->uid = getuid();

	superblock.inodes[i] = new_inode;
	superblock.inode_number++;
	superblock.inode_bitmap[i] = USED;
	return i;
}


int
write_in_file(void *data, size_t size, size_t count, FILE *fp) {
    size_t written = fwrite(data, size, count, fp);
    if (written != count) {
        printf("Error writing to file");
        return -EIO;
    }
    return 0;
}


int serialize() {
    FILE *fp = fopen(PATH_FS, "wb");
    if (fp == NULL) {
        printf("[debug] Error opening file\n");
        return -EIO;
    }

    printf("[debug] Serializing superblock\n");
    if (write_in_file(&superblock.inode_bitmap, sizeof(int), NINODES, fp) != 0) {
        printf("[debug] Error writing inode_bitmap");
		fclose(fp);
        return -EIO;
    }

    for (int i = 0; i < NINODES; i++) {
        if (superblock.inode_bitmap[i] == FREE) continue;

        inode_t *inode = superblock.inodes[i];
        if (write_in_file(inode, sizeof(inode_t), 1, fp) != 0) {
			printf("[debug] Error writing inode\n");
            fclose(fp);
            return -EIO;
        }

		if (write_in_file(&(inode->data_size), sizeof(size_t), 1, fp) != 0) {
            printf("[debug] Error writing inode size");
			fclose(fp);
            return -EIO;
        }

        if (write_in_file(&(inode->data_len), sizeof(size_t), 1, fp) != 0) {
            printf("[debug] Error writing inode content length");
			fclose(fp);
            return -EIO;
        }

        if (inode->data_len > 0 && write_in_file(inode->data, inode->data_len, 1, fp) != 0) {
            printf("[debug] Error writing inode data");
			fclose(fp);
            return -EIO;
        }
    }

    fclose(fp);
    printf("[debug] Serialization completed\n");
    return 0;
}

int deserialize() {
    FILE *fp = fopen(PATH_FS, "rb");
    if (fp == NULL) {
        printf("[debug] Error opening file\n");
        return -ENOENT;
    }

    printf("[debug] Deserializing superblock\n");
    if (fread(&superblock.inode_bitmap, sizeof(int), NINODES, fp) != NINODES) {
        fclose(fp);
        printf("[debug] Error reading inode_bitmap");
        return -EIO;
    }


    for (int i = 0; i < NINODES; i++) {
        if (superblock.inode_bitmap[i] == FREE) continue;

		printf("[debug] Inode %d\n", i);

        superblock.inodes[i] = malloc(sizeof(inode_t));
        if (!superblock.inodes[i]) {
            printf("[debug] Memory allocation failed");
            fclose(fp);
            return -ENOMEM;
        }
		

        inode_t *inode = superblock.inodes[i];

        if (fread(inode, sizeof(inode_t) , 1, fp) != 1) {
            printf("[debug] Error reading inode");
            fclose(fp);
            return -EIO;
        }

		printf("Path: %s\n", inode->path);
		printf("Type: %d\n", inode->type);
		
		if (fread(&(inode->data_size), sizeof(size_t), 1, fp) != 1) {
            printf("[debug] Error reading inode size");
            fclose(fp);
            return -EIO;
        }

		printf("Data size: %ld\n", inode->data_size);

        if (fread(&(inode->data_len), sizeof(size_t), 1, fp) != 1) {
            printf("[debug] Error reading inode size");
            fclose(fp);
            return -EIO;
        }

		printf("Data len: %ld\n", inode->data_len);

        if (inode->data_len > 0) {
            inode->data = malloc(inode->data_size);
            if (!inode->data) {
                printf("[debug] Memory allocation failed for inode data");
                fclose(fp);
                return -ENOMEM;
            }

            if (fread(inode->data, inode->data_len, 1, fp) != 1) {
                printf("[debug] Error reading inode data");
                fclose(fp);
                return -EIO;
            }
        } else {
            inode->data = malloc(DATA_INIT_SIZE*sizeof(char));
			inode->data[0] = '\0';
        }
    }

    fclose(fp);
    printf("[debug] Deserialization completed\n");
    return 0;
}

