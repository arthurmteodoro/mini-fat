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

void print_entry_1(dir_entry_t* entry) {
    if (entry->type == EMPTY_TYPE) {
        printf("Empty node\n");
    } else {
        printf("Name: %s\n", entry->name);
        printf("Type: %d\n", entry->type);
        printf("Size: %d\n", entry->size);
        printf("Creation time: %d/%d/%d - %d:%d:%d\n", entry->create.day, entry->create.month,
               entry->create.year, entry->create.hour, entry->create.minutes, entry->create.seconds);
        printf("Update time: %d/%d/%d - %d:%d:%d\n", entry->update.day, entry->update.month,
               entry->update.year, entry->update.hour, entry->update.minutes, entry->update.seconds);
        printf("First Block: %d\n", entry->first_block);
    }
}

void print_dir_entry_1(dir_entry_t* dir) {
    for(int i = 0; i < 49; i++) {
        printf("Node %d\n", i);
        print_entry_1(&dir[i]);
        printf("----------\n");
    }
}

void print_disk_info_1(info_entry_t info) {
    printf("Total blocks: %d\n", info.total_block);
    printf("Available blocks: %d\n", info.available_blocks);
    printf("Block size: %d\n", info.block_size);
    printf("Blocks per sector: %d\n", info.block_per_sector);
    printf("Sector per fat: %d\n", info.sector_per_fat);
    printf("Dir entry number: %d\n", info.dir_entry_number);
}

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
                         .dir_entry_number = DIRENTRYCOUNT};

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

int create_empty_file(dir_entry_t* dir, dir_entry_t* dir_entry_list, info_entry_t* info, fat_entry_t * fat, const char* name) {
    time_t now;
    struct tm* time_info;
    dir_entry_t new_file;

    int index_in_dir_entry = get_first_empty_dir_entry(dir_entry_list, info->dir_entry_number);
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
    new_file.first_block = index_in_fat_entry+1;

    dir_entry_list[index_in_dir_entry] = new_file;
    fat[index_in_fat_entry] = ENDOFCHAIN;

    // jump fat sectors, info sector and enter this block
    int sector_to_write;
    if (dir == NULL)
        sector_to_write = info->sector_per_fat+1;
    else
        sector_to_write = info->sector_per_fat+1+dir->first_block;
    write_sector(sector_to_write, dir_entry_list);
    write_fat_table(fat, info->sector_per_fat);

    return 0;
}

int create_empty_dir(dir_entry_t* dir, dir_entry_t* dir_entry_list, info_entry_t* info, fat_entry_t * fat, const char* name) {
    time_t now;
    struct tm* time_info;
    dir_entry_t new_file;

    int index_in_dir_entry = get_first_empty_dir_entry(dir_entry_list, info->dir_entry_number);
    if (index_in_dir_entry == -1) return -1;
    int index_in_fat_entry = get_first_empty_fat_entry(fat, info->available_blocks);
    if (index_in_fat_entry == -1) return -1;

    time(&now);
    time_info = localtime(&now);

    strcpy(new_file.name, name);
    new_file.type = DIR_TYPE;
    new_file.size = 0;
    tm_to_date(time_info, &new_file.create);
    tm_to_date(time_info, &new_file.update);
    new_file.first_block = index_in_fat_entry + 1;

    dir_entry_list[index_in_dir_entry] = new_file;
    fat[index_in_fat_entry] = ENDOFCHAIN;

    // jump fat sectors, info sector and enter this block
    if (dir == NULL)
        write_sector((int) info->sector_per_fat+1, dir_entry_list);
    else
        write_sector((int) info->sector_per_fat+1+dir->first_block, dir_entry_list);

    char buffer[SECTOR_SIZE];
    memset(buffer, 0x0, SECTOR_SIZE);
    write_sector(info->sector_per_fat+1+new_file.first_block, buffer);

    write_fat_table(fat, info->sector_per_fat);

    return 0;
}

int search_dir_entry(dir_entry_t* dir_entry, info_entry_t* info, const char* name, dir_descriptor_t* descriptor) {
    for(int i = 0; i < DIRENTRYCOUNT; i++) {
        if (dir_entry[i].type != EMPTY_TYPE) {
            if (strcmp(dir_entry[i].name, name) == 0) {
                memcpy(&descriptor->dir_infos, &dir_entry[i], sizeof(dir_entry_t));
                read_sector((int) info->sector_per_fat+1+dir_entry[i].first_block, descriptor->entry);

                return 0;
            }
        }
    }

    return -1;
}

int search_file_in_dir(dir_entry_t* dir_entry, const char* name, dir_entry_t* file) {
    for(int i = 0; i < DIRENTRYCOUNT; i++) {
        if (dir_entry[i].type != EMPTY_TYPE) {
            if (strcmp(dir_entry[i].name, name) == 0) {
                memcpy(file, &dir_entry[i], sizeof(dir_entry_t));

                return 0;
            }
        }
    }

    return -1;
}

int search_file_index_in_dir(dir_entry_t* dir_entry, const char* name) {
    for(int i = 0; i < DIRENTRYCOUNT; i++) {
        if (dir_entry[i].type != EMPTY_TYPE) {
            if (strcmp(dir_entry[i].name, name) == 0) {
                return i;
            }
        }
    }

    return -1;
}

int write_file(fat_entry_t * fat, const info_entry_t* info, dir_entry_t* dir, dir_entry_t* dir_entry_list, dir_entry_t* file, int offset, char* buffer, int size) {
    char sector_buffer[SECTOR_SIZE];

    if (file == NULL) return -1;
    if (offset > file->size) return -1;
    if (offset < 0) return -1;

    // get the number of empty bytes in sector and a number of used bytes
    int used_bytes_last_block = file->size;
    int number_of_blocks = 1;
    while(used_bytes_last_block > SECTOR_SIZE) {
        used_bytes_last_block -= SECTOR_SIZE;
        number_of_blocks++;
    }
    int empty_bytes_in_last_block = SECTOR_SIZE - used_bytes_last_block;

    // get the last sector
    int sector_number = -1;
    for(int i = 0; i < number_of_blocks; i++) {
        if (sector_number == -1) sector_number = file->first_block;
        else sector_number = fat[sector_number-1];
    }

    // if the buffer fits in the last sector
    if (empty_bytes_in_last_block >= size) {
        // get offset in buffer
        int real_offset = offset;
        if (number_of_blocks > 1) real_offset = offset - (number_of_blocks-1)*SECTOR_SIZE;

        // read a sector, write in end and save sector
        read_sector((int) info->sector_per_fat+1+sector_number, sector_buffer);
        memcpy(&sector_buffer[real_offset], buffer, size);
        write_sector((int) info->sector_per_fat+1+sector_number, sector_buffer);

    // if the buffer does not fit in the last sector and the size is smaller than one sector
    } else if ((empty_bytes_in_last_block < size) && (size <= SECTOR_SIZE)) {
        // get new block
        int new_fat_entry_file = get_first_empty_fat_entry(fat, info->available_blocks);
        if (new_fat_entry_file == -1) return -1;

        // calculate offset in buffer
        int real_offset = offset;
        if (number_of_blocks > 1) real_offset = offset - (number_of_blocks-1)*SECTOR_SIZE;

        // read a sector, write and save sector
        read_sector((int) info->sector_per_fat+1+sector_number, sector_buffer);
        memcpy(&sector_buffer[real_offset], buffer, empty_bytes_in_last_block);
        write_sector((int) info->sector_per_fat+1+sector_number, sector_buffer);

        // update fat table
        fat[sector_number-1] = new_fat_entry_file;
        fat[new_fat_entry_file] = ENDOFCHAIN;

        // write buffer in sector
        memset(sector_buffer, 0x0, SECTOR_SIZE);
        memcpy(sector_buffer, &buffer[empty_bytes_in_last_block], size-empty_bytes_in_last_block);
        write_sector((int) info->sector_per_fat+1+new_fat_entry_file, sector_buffer);

        // write fat table
        write_fat_table(fat, info->sector_per_fat);

    // if the buffer does not fit in the last sector and the size is greater than one sector
    } else if ((empty_bytes_in_last_block < size) && (size > SECTOR_SIZE)){
        int real_offset = offset;
        int buffer_offset = 0;
        int remain_write = size;

        // get offset
        if (number_of_blocks > 1) real_offset = offset - (number_of_blocks-1)*SECTOR_SIZE;

        // write first block
        read_sector((int) info->sector_per_fat+1+sector_number, sector_buffer);
        memcpy(&sector_buffer[real_offset], buffer, empty_bytes_in_last_block);
        write_sector((int) info->sector_per_fat+1+sector_number, sector_buffer);

        // calculate buffer offset and remain bytes to write
        buffer_offset += empty_bytes_in_last_block;
        remain_write -= empty_bytes_in_last_block;

        // calculate the number of blocks required
        int number_of_request_blocks = (int) ceil((double)(size - empty_bytes_in_last_block)/SECTOR_SIZE);
        for(int i = 0; i < number_of_request_blocks; i++) {
            // get new fat entry
            int new_fat_entry_file = get_first_empty_fat_entry(fat, info->available_blocks);

            // update fat table
            fat[sector_number-1] = new_fat_entry_file;
            fat[new_fat_entry_file] = ENDOFCHAIN;

            // clear sector
            memset(sector_buffer, 0x0, SECTOR_SIZE);

            // if is last block, write only remain bytes
            if(i == number_of_request_blocks-1) {
                memcpy(sector_buffer, &buffer[buffer_offset], remain_write);
                remain_write -= remain_write;
                buffer_offset += remain_write;
            }
            else {
                memcpy(sector_buffer, &buffer[buffer_offset], SECTOR_SIZE);
                remain_write -= SECTOR_SIZE;
                buffer_offset += SECTOR_SIZE;
            }

            // write sector
            write_sector((int) info->sector_per_fat+1+new_fat_entry_file, sector_buffer);
            sector_number = new_fat_entry_file+1;
        }

        // update fat entry
        write_fat_table(fat, info->sector_per_fat);
    }

    // update file in dir entry
    time_t now;
    struct tm* time_info;
    time(&now);
    time_info = localtime(&now);

    // update file size and update time
    file->size += size;
    tm_to_date(time_info, &file->update);
    int file_index = search_file_index_in_dir(dir_entry_list, file->name);
    memcpy(&dir_entry_list[file_index], file, sizeof(dir_entry_t));

    // update dir entry list
    int sector_to_write;
    if (dir == NULL)
        sector_to_write = info->sector_per_fat+1;
    else
        sector_to_write = info->sector_per_fat+1+dir->first_block;
    write_sector(sector_to_write, dir_entry_list);

    return 0;
}
