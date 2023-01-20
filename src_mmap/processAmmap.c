#include "./../include/processA_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <bmpfile.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

// Define the size of the shared memory
#define WIDTH 1600
#define HEIGHT 600
#define DEPTH 4

// Define the struct of the shared memory
struct shared
{
    int m[WIDTH][HEIGHT];
};

// Set the color of the circle (0 - 255)
const u_int8_t RED = 0;
const u_int8_t GREEN = 0;
const u_int8_t BLUE = 255;
const u_int8_t ALPHA = 0;

// Delcare circle radius
const int RADIUS = 30;

// Define the semaphores
sem_t *semaphore;
sem_t *semaphore2;

// Character buffer for the log file
char log_buffer[100];

// File descriptor for the log file
int log_fd;

// Function to draw a circle
void draw_my_circle(int radius, int x, int y, bmpfile_t *bmp, rgb_pixel_t color) {
    // Define the center of the circle
    int centerX = x * 20;
    int centerY = y * 20;

    // Loop over the pixels of the circle
    for (int i = centerX - radius; i <= centerX + radius; i++) {
        for (int j = centerY - radius; j <= centerY + radius; j++) {
            if (pow(i - centerX, 2) + pow(j - centerY, 2) <= pow(radius, 2)) {
                // Color the pixel at the specified (x,y) position with the given pixel values
                bmp_set_pixel(bmp, i, j, color);
            }
        }
    }
}

// Function to clear the circle
void clear_circle(int radius, int x, int y, bmpfile_t *bmp) {
    // Define the center of the circle
    int centerX = x * 20;
    int centerY = y * 20;
    // Define the color of the circle
    rgb_pixel_t color = {255, 255, 255, 0}; // White

    // Loop over the pixels of the circle
    for (int i = centerX - radius; i <= centerX + radius; i++) {
        for (int j = centerY - radius; j <= centerY + radius; j++) {
            // If the pixel is inside the circle..
            if (pow(i - centerX, 2) + pow(j - centerY, 2) <= pow(radius, 2)) {
                // Color the pixel at the specified (x,y) position with the given pixel values
                bmp_set_pixel(bmp, i, j, color);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    // Open the log file
    if ((log_fd = open("log/processA.log",O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1)
    {
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
    shm_fd = shm_open("my_shm", O_RDWR | O_CREAT, 0666);

    // Set the size of the shared memory
    ftruncate(shm_fd, sizeof(struct shared));

    // Map the shared memory to the memory space of the process
    shm_ptr = mmap(0, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shm_ptr == MAP_FAILED) {
        perror("Error in mmap");
        exit(1);
    }

    // Use the shared memory data to determine where to draw circles on the image

    //
    
    // Create the bitmap
    bmpfile_t *bmp;
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);
    if (bmp == NULL) {
        printf("Error: unable to create bitmap\n");
        return 1;
    }
    
    // Open the semaphores
    semaphore = sem_open("my_sem1", O_CREAT, 0666, 1);
    semaphore2 = sem_open("my_sem2", O_CREAT, 0666, 1);

    while (1) {
        // Wait for the semaphore
        sem_wait(semaphore);

        // Check the shared memory for the coordinates of the circle
        int x = shm_ptr->m[0][0];
        int y = shm_ptr->m[0][1];

        // Draw the circle on the image
        draw_my_circle(RADIUS, x, y, bmp, (rgb_pixel_t){RED, GREEN, BLUE, ALPHA});

        // Post the semaphore2 to signal that the operation is done
        sem_post(semaphore2);
    }

    // Close the semaphores and unlink the shared memory
    // Close the bitmap
    bmp_destroy(bmp);
    sem_close(semaphore);
    sem_close(semaphore2);
    sem_unlink("my_sem1");
    sem_unlink("my_sem2");
    munmap(shm_ptr, sizeof(struct shared));
    shm_unlink("my_shm");

    return 0;
}

