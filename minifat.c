//
// Created by arthur on 13/11/2019.
//

#include "minifat.h"
#include <stdio.h>

int main() {
    printf("Size of fir entry: %ld\n", BLOCK_SIZE/sizeof(struct dir_entry));

    return 0;
}
