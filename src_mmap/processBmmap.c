#include "./../include/processB_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <bmpfile.h>
#include <math.h>
#include <semaphore.h>

// Declaration variables for bmp file
const int width = 1600;
const int height = 600;
const int depth = 4;

// Define struct for shared memory and variables
struct shared {
    int m[1600][600];
};

//const char * sem_fn = { "my_sem"};
sem_t *semaphore;
sem_t *semaphore2;

// Buffer to store the string to write to the log file
char log_buffer[100];

// File descriptor for the log file
int log_fd;

// Variable to store the value of the write function
int check;

// Function to check the correctness of the operation just done
void CheckCorrectness(int c) {
    if(c == -1) {
        close(log_fd);
        perror("Error in writing function");
        exit(1);
    }
}

// Function to draw a blue circle
void draw_blue_circle(int radius,int x,int y, bmpfile_t *bmp) {

    rgb_pixel_t pixel = {255, 0, 0, 0};
    for(int i = -radius; i <= radius; i++) {
    for(int j = -radius; j <= radius; j++) {
      // If distance is smaller, point is within the circle
      if(sqrt(i*i + j*j) < radius) {
          /*
          * Color the pixel at the specified (x,y) position
          * with the given pixel values
          */
          bmp_set_pixel(bmp, x*20 + i, y*20 + j, pixel);
      }
    }
  }
}

// Function to cancel the blue circle
void cancel_blue_circle(int radius,int x,int y, bmpfile_t *bmp) {

    rgb_pixel_t pixel = {255, 255, 255, 0};
    for(int i = -radius; i <= radius; i++) {
    for(int j = -radius; j <= radius; j++) {
      // If distance is smaller, point is within the circle
      if(sqrt(i*i + j*j) < radius) {
          /*
          * Color the pixel at the specified (x,y) position
          * with the given pixel values
          */
          bmp_set_pixel(bmp,  x*20+i,y*20+  j, pixel);
      }
    }
  }
}

int main(int argc, char const *argv[]) {
    // Open the log file
    if ((log_fd = open("processB.log",O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1) {
        // If the file could not be opened, print an error message and exit
        perror("Error opening command file");
        exit(1);
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // File descriptor for the shared memory
    int shm_fd;

    // Pointer to the struct of shared memory
    struct shared *shm_ptr;

    // Open the shared memory
    shm_fd = shm_open("my_shm", O_RDWR, 0666);

    // Map the shared memory to the memory space of the process
    shm_ptr = mmap(0, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shm_ptr == MAP_FAILED) {
        perror("Error in mmap");
        exit(1);
    }

    // Use the shared memory data to determine where to draw circles on the image

    // ...

    // Unmap the shared memory from the memory space of the process
    munmap(shm_ptr, sizeof(struct shared));

    // Close the shared memory
    close(shm_fd);

    return 0;
}


