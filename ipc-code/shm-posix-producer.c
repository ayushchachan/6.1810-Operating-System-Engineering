#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

int main(void) {
    const int SIZE = 4096;
    const char *name = "OS";
    const char *empty_msg = "freeeee";
    const char *filled_msg = "OSisFUN";
    const int SLOT_SIZE = 8;
    const int NUM_SLOT = SIZE / SLOT_SIZE;
    const int MAX_EXCHANGE = 1000;

    int fd;
    char *ptr;

    fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        fprintf(stderr, "shm_open failed: %s", strerror(errno));
        exit(1);
    }

    if (ftruncate(fd, SIZE) == -1) {
        fprintf(stderr, "ftruncate error: %s", strerror(errno));
        exit(1);
    }

    ptr = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s", strerror(errno));
        exit(1);
    }
    // write to shared memory using sprintf
    for (int i = 0; i < NUM_SLOT; i++) {
        memcpy(ptr + i * SLOT_SIZE, empty_msg, SLOT_SIZE);
    }

    int free_slot_index = 0;
    int num_exchange = 0;
    while (num_exchange < MAX_EXCHANGE) {
        if (strcmp(ptr[free_slot_index * SLOT_SIZE], empty_msg) == 0) {
            memcpy(ptr + free_slot_index * SLOT_SIZE, filled_msg, SLOT_SIZE);
            free_slot_index = (free_slot_index % NUM_SLOT);
        } 
    }
    
}