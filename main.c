#include "minifat.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void print_entry(dir_entry_t* entry) {
    if (entry->mode == EMPTY_TYPE) {
        printf("Empty node\n");
    } else {
        printf("Name: %s\n", entry->name);
        printf("Mode: %o\n",  entry->mode);
        printf("UID: %d\n", entry->uid);
        printf("GID: %d\n", entry->gid);
        printf("Size: %d\n", entry->size);
        printf("Creation time: %d/%d/%d - %d:%d:%d\n", entry->create.day, entry->create.month,
               entry->create.year, entry->create.hour, entry->create.minutes, entry->create.seconds);
        printf("Update time: %d/%d/%d - %d:%d:%d\n", entry->update.day, entry->update.month,
               entry->update.year, entry->update.hour, entry->update.minutes, entry->update.seconds);
        printf("First Block: %d\n", entry->first_block);
    }
}

void print_dir_entry(dir_entry_t* dir) {
    for(int i = 0; i < 49; i++) {
        printf("Node %d\n", i);
        print_entry(&dir[i]);
        printf("----------\n");
    }
}

void print_fat(fat_entry_t* fat, int size) {
    for(int i = 0; i < size; i++) {
        if (fat[i] == ENDOFCHAIN) printf(" ENDOFCHAIN");
        else if (fat[i] == UNUSED) printf(" UNUSED");
        else printf(" %d", fat[i]);

        if (i == 50) break;
    }
}

void print_disk_info(info_entry_t info) {
    printf("Total blocks: %d\n", info.total_block);
    printf("Available blocks: %d\n", info.available_blocks);
    printf("Block size: %d\n", info.block_size);
    printf("Blocks per sector: %d\n", info.block_per_sector);
    printf("Sector per fat: %d\n", info.sector_per_fat);
    printf("Dir entry number: %d\n", info.dir_entry_number);
}

int main() {
    setbuf(stdout, NULL);

    fd = open(virtual_disk, O_RDWR);

    printf("Formatting disk ..........");
    format(3862528);
    printf("   disk successfully formatted\n");

    info_entry_t info;
    fat_entry_t* fat_entry = NULL;
    dir_entry_t* root_entry = NULL;

    printf("Initializing disk ..........");
    init(&info, &fat_entry, &root_entry);
    printf("   disk successfully initialized\n");
    printf("Disk infos: \n");
    print_disk_info(info);

    printf("\n\nClear\n");
    print_dir_entry(root_entry);
    create_empty_file(NULL, root_entry, &info, fat_entry,"teste", S_IFREG | 0644, getuid(), getgid());
    //print_dir_entry(root_entry);

    //create_empty_dir(NULL, root_entry, &info, fat_entry, "dir1");

    //printf("\n\nAfter create file and dir\n");
    //print_dir_entry(root_entry);

    //dir_descriptor_t subdir1;
    //search_dir_entry(root_entry, &info, "dir1", &subdir1);

    //create_empty_file(&subdir1.dir_infos, (dir_entry_t*)subdir1.entry, &info, fat_entry, "teste02");

    //printf("\n\nSubdir: \n");
    //print_dir_entry((dir_entry_t*)subdir1.entry);

    printf("\n\n Entry file teste\n");
    dir_entry_t file;
    search_file_in_dir(root_entry, "teste", &file);
    print_entry(&file);

    char* lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam nunc est, viverra quis metus vel, elementum cursus nisl. Nullam id.";
    char test_text[10000];
    for(int i = 0; i < 10000; i++)
        test_text[i] = (char) i;

    /*for(int i = 0; i < 100; i++) {
        write_file(fat_entry, &info, NULL, root_entry, &file, 0, test_text, 10000);
    }*/

    /*for(int i = 0; i < 100; i++){
        int asd = write_file(fat_entry, &info, NULL, root_entry, &file, 300*i, test_text, 300);
    }*/
    //write_file(fat_entry, &info, NULL, root_entry, &file, 0, test_text, 10000);

    int asd = write_file(fat_entry, &info, NULL, root_entry, &file, 0, test_text, SECTOR_SIZE);
    write_file(fat_entry, &info, NULL, root_entry, &file, SECTOR_SIZE, test_text, SECTOR_SIZE-200);
    write_file(fat_entry, &info, NULL, root_entry, &file, SECTOR_SIZE*2-200, test_text, SECTOR_SIZE);

    //write_file(fat_entry, &info, NULL, root_entry, &file, 0, test_text, SECTOR_SIZE);
    //write_file(fat_entry, &info, NULL, root_entry, &file, SECTOR_SIZE, test_text, 300);

    write_file(fat_entry, &info, NULL, root_entry, &file, 4090, lorem, strlen(lorem));

    search_file_in_dir(root_entry, "teste", &file);
    print_entry(&file);

    char buffer[6000];
    int number_read = read_file(fat_entry, &info, &file, 4090, buffer, strlen(lorem));
    printf("\n\nNumber bytes read: %d\tValue Buffer: \n", number_read);
    for(int i = 0; i < number_read; i++) {
        if (i % 50 == 0) printf("\n");
        printf("%02X ", (unsigned char) buffer[i]);
    }
    buffer[number_read] = '\0';
    printf("\n\n%s\n", buffer);

    printf("\n\nFAT: \n");
    print_fat(fat_entry, info.available_blocks/info.block_per_sector);

    release(&fat_entry, &root_entry);
    close(fd);

    return 0;
}
