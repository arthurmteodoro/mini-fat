#ifndef MINI_FAT_MINIFAT_H
#define MINI_FAT_MINIFAT_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

#define virtual_disk "/dev/sdb"
extern int fd;

#define BLOCK_SIZE 512
#define SECTOR_SIZE 4096
#define DIRENTRYCOUNT (SECTOR_SIZE / sizeof(dir_entry_t))
#define MAXNAME 64
#define MAXPATHLENGTH 1024

#define UNUSED 0x00
#define ENDOFCHAIN 0xFFFFFFFF

#define EMPTY_TYPE 0x00

#define COMMAND_SIZE 5
#define COMMAND_POS 0
#define BLOCK_POS 1
#define READ_BLOCK 1
#define WRITE_BLOCK 2

#define UART_READ_TIMEOUT 5000

typedef uint32_t fat_entry_t;

/**
 * Estrutura para armazenar a data que será usada para informar
 * data relacionada a um arquivo/diretório.
 */
struct date_format {
    unsigned int day:6;
    unsigned int month: 5;
    unsigned int year:13;
    unsigned int hour:6;
    unsigned int minutes:7;
    unsigned int seconds:7;
} __attribute__((packed));
typedef struct date_format date_t;

/**
 * Armazena as informações do sistema de arquivo.
 */
struct info_entry {
    uint32_t total_block;
    uint32_t available_blocks;
    uint32_t block_size;
    uint32_t block_per_sector;
    uint32_t sector_per_fat;
    uint32_t dir_entry_number;
} __attribute__((packed));
typedef struct info_entry info_entry_t;

/**
 * Estrutura que armazena as informações de uma entrada
 * de diretório (arquivo e subdiretórios).
 */
struct dir_entry {
    char name[MAXNAME];
    uid_t uid;
    gid_t gid;
    mode_t mode;
    date_t create;
    date_t update;
    uint32_t size;
    uint32_t first_block;
} __attribute__((packed));
typedef struct dir_entry dir_entry_t;

/**
 * Descritor de um diretório (contém informações e os filhos do diretório).
 */
struct dir_descriptor {
    dir_entry_t dir_infos;
    char entry[SECTOR_SIZE];
} __attribute__((packed));
typedef struct dir_descriptor dir_descriptor_t;

void tm_to_date(struct tm *tm, date_t *date);

int format(int size);

void init(info_entry_t *info, fat_entry_t **fat_entry, dir_entry_t **root_dir);

int create_empty_file(dir_entry_t *dir, dir_entry_t *dir_entry_list,
                      info_entry_t *info, fat_entry_t *fat, const char *name,
                      mode_t mode, uid_t uid, gid_t gid);

int create_empty_dir(dir_entry_t *dir, dir_entry_t *dir_entry_list,
                     info_entry_t *info, fat_entry_t *fat, const char *name,
                     mode_t mode, uid_t uid, gid_t gid);

int search_dir_entry(dir_entry_t *dir_entry, info_entry_t *info, const char *name,
                     dir_descriptor_t *descriptor);

int search_entry_in_dir(dir_entry_t *dir_entry, const char *name,
                        dir_entry_t *entry);

int search_file_in_dir(dir_entry_t *dir_entry, const char *name, dir_entry_t *file);

int write_file(fat_entry_t *fat, const info_entry_t *info, dir_entry_t *dir,
               dir_entry_t *dir_entry_list, dir_entry_t *file,
               int offset, const char *buffer, int size);

int read_file(const fat_entry_t *fat, const info_entry_t *info, dir_entry_t *file,
              int offset, char *buffer,
              unsigned int size);

int delete_file(fat_entry_t *fat, const info_entry_t *info, dir_entry_t *dir,
                dir_entry_t *dir_entry_list,
                dir_entry_t *file);

int delete_dir(fat_entry_t *fat, const info_entry_t *info, dir_entry_t *father_dir,
               dir_entry_t *dir_entry_list,
               dir_entry_t *dir);

int add_entry_in_dir_entry(dir_entry_t *dir, dir_entry_t *dir_entry_list,
                           dir_entry_t *entry, const info_entry_t *info);

int update_entry(dir_entry_t *father_dir, dir_entry_t *dir_entry_list, dir_entry_t *entry, const info_entry_t *info,
                 char *name, uid_t uid, gid_t gid, mode_t mode, date_t* update);

int
resize_file(fat_entry_t *fat, const info_entry_t *info, dir_entry_t *dir, dir_entry_t *dir_entry_list, dir_entry_t *file,
            int new_size);

#endif //MINI_FAT_MINIFAT_H