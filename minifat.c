#include "minifat.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void write_block(uint32_t block, void* buffer) {
    int fd = open(virtual_disk, O_RDWR);

    lseek(fd, block*BLOCK_SIZE, SEEK_SET);
    int count_write = write(fd, buffer, 512);

    close(fd);
}

void write_sector(int sector, void* buffer) {
    uint32_t first_block = sector*(SECTOR_SIZE/BLOCK_SIZE);

    for(int i = 0; i < SECTOR_SIZE/BLOCK_SIZE; i++) {
        write_block(first_block+i, buffer+(i*BLOCK_SIZE));
    }
}

int format(int size) {
    int blocks_count = size/BLOCK_SIZE;
    int sector_count = blocks_count/(SECTOR_SIZE/BLOCK_SIZE);
    char buffer[SECTOR_SIZE];

    fat_entry_t* fat_entry = (fat_entry_t*) malloc(sizeof(fat_entry_t) * sector_count);
    for(int i = 0; i < sector_count; i++) fat_entry[i] = UNUSED;

    void* table = fat_entry;

    int qtd_sectors_for_fat_table = (int) ceil(((double)sizeof(fat_entry_t) * sector_count) / SECTOR_SIZE);

    for(int i = 0; i < qtd_sectors_for_fat_table; i++) {
        memset(buffer, 0x0, SECTOR_SIZE);
        memcpy(buffer,table+(i*SECTOR_SIZE), SECTOR_SIZE);

        write_sector(i+1, buffer);
    }

    info_entry_t info = {.total_block = blocks_count,
                         .block_size = BLOCK_SIZE,
                         .block_per_sector = SECTOR_SIZE/BLOCK_SIZE,
                         .sector_per_fat = qtd_sectors_for_fat_table,
                         .root_entry_number = 49};

    memset(buffer, 0x0, SECTOR_SIZE);
    memcpy(buffer, &info, sizeof(info_entry_t));

    write_sector(0, buffer);

    return 0;
}