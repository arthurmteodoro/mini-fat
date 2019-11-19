#include "minifat.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

int fd;

void tm_to_date(struct tm * tm, date_t* date) {
    date->day = tm->tm_mday;
    date->month = tm->tm_mon;
    date->year = 1900 + tm->tm_year;
    date->hour = tm->tm_hour;
    date->minutes = tm->tm_min;
    date->seconds = tm->tm_sec;
}

void write_block(uint32_t block, void* buffer) {
    //int fd = open(virtual_disk, O_RDWR);

    lseek(fd, block*BLOCK_SIZE, SEEK_SET);
    int count_write = write(fd, buffer, BLOCK_SIZE);

    //close(fd);
}

void read_block(uint32_t block, void* buffer) {
    //int fd = open(virtual_disk, O_RDWR);

    lseek(fd, block*BLOCK_SIZE, SEEK_SET);
    int read_counter = read(fd, buffer, BLOCK_SIZE);

    //close(fd);
}

void write_sector(int sector, void* buffer) {
    uint32_t first_block = sector*(SECTOR_SIZE/BLOCK_SIZE);

    for(int i = 0; i < SECTOR_SIZE/BLOCK_SIZE; i++) {
        write_block(first_block+i, buffer+(i*BLOCK_SIZE));
    }
}

void read_sector(int sector, void* buffer) {
    uint32_t first_block = sector*(SECTOR_SIZE/BLOCK_SIZE);

    for(int i = 0; i < SECTOR_SIZE/BLOCK_SIZE; i++) {
        read_block(first_block+i, buffer+(i*BLOCK_SIZE));
    }
}

void write_fat_table(void* fat, int size) {
    char buffer[SECTOR_SIZE];

    for(int i = 0; i < size; i++) {
        memcpy(buffer, fat+(i*SECTOR_SIZE), SECTOR_SIZE);
        write_sector(i+1, buffer);
    }
}

int format(int size) {
    int blocks_count = size;// /BLOCK_SIZE;
    int sector_count = blocks_count/(SECTOR_SIZE/BLOCK_SIZE);
    char buffer[SECTOR_SIZE];

    //fat_entry_t* fat_entry = (fat_entry_t*) malloc(sizeof(fat_entry_t) * sector_count);
    //memset(fat_entry, UNUSED, sizeof(fat_entry_t) * sector_count);

    //void* table = fat_entry;

    int qtd_sectors_for_fat_table = (int) ceil(((double)sizeof(fat_entry_t) * sector_count) / SECTOR_SIZE);

    // write fat entry
    memset(buffer, UNUSED, SECTOR_SIZE);
    for(int i = 0; i < qtd_sectors_for_fat_table; i++) {
        write_sector(i+1, buffer);
    }

   // free(fat_entry);

    // write fat infos
    int blocks_per_sector = SECTOR_SIZE/BLOCK_SIZE;
    info_entry_t info = {.total_block = blocks_count,
                         .available_blocks = blocks_count - (blocks_per_sector) - (blocks_per_sector*qtd_sectors_for_fat_table) - (blocks_per_sector),
                         .block_size = BLOCK_SIZE,
                         .block_per_sector = blocks_per_sector,
                         .sector_per_fat = qtd_sectors_for_fat_table,
                         .root_entry_number = SECTOR_SIZE/sizeof(dir_entry_t)};

    memset(buffer, 0x0, SECTOR_SIZE);
    memcpy(buffer, &info, sizeof(info_entry_t));
    write_sector(0, buffer);

    // write root dir entry
    memset(buffer, 0x0, SECTOR_SIZE);
    write_sector(qtd_sectors_for_fat_table+1, buffer);

    return 0;
}

void init(info_entry_t* info, fat_entry_t** fat_entry, dir_entry_t** root_dir) {
    char buffer[SECTOR_SIZE];

    // read fat info
    read_block(0, buffer);
    memcpy(info, buffer, sizeof(info_entry_t));

    // malloc and read fat entry
    *fat_entry = (fat_entry_t*) malloc(sizeof(fat_entry_t) * (info->available_blocks/info->block_per_sector));
    for(uint32_t i = 0; i < info->sector_per_fat; i++) {
        read_sector(i+1, buffer);
        memcpy((void*)*fat_entry+(i*SECTOR_SIZE), buffer, SECTOR_SIZE);
    }

    // malloc and read root dir entry
    *root_dir = (dir_entry_t*) malloc(SECTOR_SIZE);
    read_sector((int) info->sector_per_fat+1, *root_dir);
}

void release(fat_entry_t** fat_entry, dir_entry_t** root_dir) {
    free(*fat_entry);
    free(*root_dir);

    *fat_entry = NULL;
    *root_dir = NULL;
}

int get_first_empty_dir_entry(const dir_entry_t* dir_entry, int size) {
    for(int i = 0; i < size; i++) {
        if (dir_entry[i].type == EMPTY_TYPE) return i;
    }

    return -1;
}

int get_first_empty_fat_entry(const fat_entry_t* fat, int size) {
    for(int i = 0; i < size; i++) {
        if (fat[i] == UNUSED) return i;
    }

    return -1;
}

int create_empty_file(dir_entry_t* dir_entry, info_entry_t* info, fat_entry_t * fat, const char* name) {
    time_t now;
    struct tm* time_info;
    dir_entry_t new_file;

    int index_in_dir_entry = get_first_empty_dir_entry(dir_entry, info->root_entry_number);
    if (index_in_dir_entry == -1) return -1;
    int index_in_fat_entry = get_first_empty_fat_entry(fat, info->available_blocks);
    if (index_in_fat_entry == -1) return -1;

    time(&now);
    time_info = localtime(&now);

    strcpy(new_file.name, name);
    new_file.type = FILE_TYPE;
    new_file.size = 0;
    tm_to_date(time_info, &new_file.create);
    tm_to_date(time_info, &new_file.update);
    new_file.first_block = index_in_fat_entry;

    dir_entry[index_in_dir_entry] = new_file;
    fat[index_in_fat_entry] = ENDOFCHAIN;

    write_sector((int) info->sector_per_fat+1, dir_entry);
    write_fat_table(fat, info->sector_per_fat);

    return 0;
}
