#ifndef MINI_FAT_MINIFAT_H
#define MINI_FAT_MINIFAT_H

#include <stdint.h>
#include <time.h>

#define BLOCK_SIZE 4096
#define DIRENTRYCOUNT (BLOCK_SIZE - sizeof(dir_entry_t))
#define MAXNAME 64
#define MAXPATHLENGTH 1024

typedef uint8_t byte_t;

struct dir_entry {
    char name[MAXNAME];
    byte_t type;
    time_t create_time;
    time_t modified_time;
    uint32_t size;
    uint32_t first_block;
} __attribute__((packed));
typedef struct dir_entry dir_entry_t;

#endif //MINI_FAT_MINIFAT_H
