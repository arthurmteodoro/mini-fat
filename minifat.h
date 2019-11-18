#ifndef MINI_FAT_MINIFAT_H
#define MINI_FAT_MINIFAT_H

#include <stdint.h>
#include <time.h>

#define virtual_disk "/dev/sdb"

#define BLOCK_SIZE 512
#define SECTOR_SIZE 4096
#define DIRENTRYCOUNT (SECTOR_SIZE / sizeof(dir_entry_t))
#define MAXNAME 64
#define MAXPATHLENGTH 1024

#define UNUSED 0x00
#define ENDOFCHAIN 0xFFFFFFFF

typedef uint8_t byte_t;

typedef uint32_t fat_entry_t;

struct date_format {
    unsigned int day:5;
    unsigned int month: 4;
    unsigned int year:12;
    unsigned int hour:5;
    unsigned int minutes:6;
    unsigned int seconds:6;
} __attribute__((packed));
typedef struct date_format date_t;

struct info_entry {
    uint32_t total_block;
    uint32_t block_size;
    uint32_t block_per_sector;
    uint32_t sector_per_fat;
    uint32_t root_entry_number;
} __attribute__((packed));
typedef struct info_entry info_entry_t;

struct dir_entry {
    char name[MAXNAME];
    byte_t type;
    date_t create;
    date_t update;
    uint32_t size;
    uint32_t first_block;
} __attribute__((packed));
typedef struct dir_entry dir_entry_t;

int format(int size);
info_entry_t init();

#endif //MINI_FAT_MINIFAT_H
