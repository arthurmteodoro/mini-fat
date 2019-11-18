#include "minifat.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void create_disk(const char* path, int size) {
    int fd = open(path, O_RDWR | O_CREAT);

    char buffer[512];
    for(int i = 0; i < 512; i++)
        buffer[i] = (char) 0xFF;

    for(int i = 0; i < size/512; i++) {
        write(fd, buffer, 512);
    }

    close(fd);
}

int main() {
    /*if (access(virtual_disk, F_OK) == -1) {
        create_disk(virtual_disk, (int)100E6);
    }*/

    format(3862528);
    printf("Size of fir entry: %ld\n", SECTOR_SIZE/sizeof(struct dir_entry));
    init();

    return 0;
}
