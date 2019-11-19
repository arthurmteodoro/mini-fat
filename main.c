#include "minifat.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void print_dir_entry(dir_entry_t* dir) {
    for(int i = 0; i < 49; i++) {
        printf("Node %d\n", i);
        if (dir[i].type == EMPTY_TYPE) {
            printf("Empty node\n");
        } else {
            printf("Name: %s\n", dir[i].name);
            printf("Type: %d\n", dir[i].type);
            printf("Size: %d\n", dir[i].size);
            printf("Creation time: %d/%d/%d - %d:%d:%d\n", dir[i].create.day, dir[i].create.month,
                    dir[i].create.year, dir[i].create.hour, dir[i].create.minutes, dir[i].create.seconds);
            printf("Update time: %d/%d/%d - %d:%d:%d\n", dir[i].update.day, dir[i].update.month,
                   dir[i].update.year, dir[i].update.hour, dir[i].update.minutes, dir[i].update.seconds);
            printf("First Block: %d\n", dir[i].first_block);
        }
        printf("----------\n");
    }
}

int main() {
    setbuf(stdout, NULL);

    fd = open(virtual_disk, O_RDWR);

    /*printf("Formatting disk ..........");
    format(3862528);
    printf("   disk successfully formatted\n");*/

    info_entry_t info;
    fat_entry_t* fat_entry = NULL;
    dir_entry_t* root_entry = NULL;

    printf("Initializing disk ..........");
    init(&info, &fat_entry, &root_entry);
    printf("   disk successfully initialized\n");

    print_dir_entry(root_entry);
    create_empty_file(root_entry, &info, "teste");
    print_dir_entry(root_entry);

    close(fd);

    return 0;
}
