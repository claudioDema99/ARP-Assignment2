#include "./../include/processB_utilities.h"
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

// Define struct for shared memory and variables
struct shared
{
    int x;
    int y;
    int m[WIDTH][HEIGHT];
};

size_t SHM_SIZE = sizeof(struct shared);

// Pointer to the shared memory
struct shared *shm_ptr;

// Define the semaphores
sem_t *semaphore;
sem_t *semaphore2;

// Buffer to store the string to write to the log file
char log_buffer[100];
// File descriptor for the log file
int log_fd;

void initialize_shared() {
    shm_ptr = (struct shared *) calloc(1, sizeof(struct shared));
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

    // File descriptor for the shared memory
    int shm_fd;
    // initialize the shared memory
    initialize_shared();

    // Open the shared memory
    shm_fd = shm_open("my_shm", O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error in shm_open");
        exit(1);
    }

    // Set the size of the shared memory
    ftruncate(shm_fd, sizeof(struct shared));

    // Map the shared memory to the memory space of the process
    shm_ptr = mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Error in mmap");
        exit(1);
    }

    // Open the log file
    if ((log_fd = open("log/processBmmap.log",O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1) {
        // If the file could not be opened, print an error message and exit
        perror("Error opening command file");
        exit(1);
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Create the bitmap
    bmpfile_t *bmp;
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);
    if (bmp == NULL) {
        printf("Error: unable to create bitmap\n");
        exit(1);
    }

    // Variable declaration in order to store the coordinates of the circles
    int center_cord = 0;
    int x_cord[600];
    int y_cord[600];
    int cont;
    int y_old;
    int x_old;

    // Flag
    int flag ;

    // Open the semaphore 1
    semaphore = sem_open("/my_sem1", 0);
    if (semaphore == (void*) -1)
    {
        perror("sem_open failure");
        exit(1);
    }

    // Open the semaphore 2
    semaphore2 = sem_open("/my_sem2", 0);
    if (semaphore2 == (void*) -1)
    {
        perror("sem_open failure");
        exit(1);
    }

    // Variable declaration in order to get the time
    time_t rawtime;
    struct tm *info;

    // Infinite loop
    while (TRUE) {

        // Get current time
        time(&rawtime);
        info = localtime(&rawtime);

        // Get the mouse event
        int cmd = getch();
               
        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        else
        {
            mvaddch(LINES/2, COLS/2, '0');
            refresh();

            // Wait for the semaphore 2
            sem_wait(semaphore2);

            // Initialize the coordinates of the circles
            for (int i=0;i<600;i++)
            {
                x_cord[i]=0;
                y_cord[i]=0;
            }
            center_cord = 0;

            int i, j;
            cont = 0;
            flag = 0;
            
            // Get the coordinates of the circles
            for (i = 0; i < 1600; i++) {
                
                // If the flag is 1, break the loop
                if (flag == 1) {
                    break;
                }
                for (j = 0; j < 600; j++) {

                    // Get the coordinates of the circles from the shared memory
                    if (shm_ptr->m[i][j] == 1)
                    {
                        x_cord[cont] = j;
                        y_cord[cont] = i;

                        if (x_cord[cont] > x_cord[cont-1]) {
                            flag = 1;
                            break;
                        }

                        cont++;
                        break;
                    }
                }
            }

            // Update the position of the center
            //center_cord = x_cord[cont-1] + 30;
            center_cord = shm_ptr->y;
            y_cord[cont-1] = shm_ptr->x;

            // Write the position of the center in the log file
            sprintf(log_buffer, "<Process_B> Position of center updated: %d - %d (%s)\n", center_cord, (int)y_cord[cont-1], asctime(info));
            if (write(log_fd, log_buffer, strlen(log_buffer)) == -1)
            {
                perror("Error writing to log file");
                exit(1);
            }

            // Draw the circles
            mvaddch((int)center_cord,(int)y_cord[cont-1], '0');

            refresh();

            // Signal the semaphore 1
            sem_post(semaphore);

            // Cancel the circle with the coordinates of the previous loop
            cancel_blue_circle(30,y_old,x_old,bmp);

            // Draw the circle with the coordinates of the current loop
            draw_blue_circle(30,y_cord[cont-1],center_cord,bmp);   

            // Update the (previous) coordinates for the next loop
            y_old = y_cord[cont-1];
            x_old = center_cord;           
        }
         
    }

    // Close the semaphores
    sem_close(semaphore);
    sem_close(semaphore2);

    // Unmap the shared memory from the memory space of the process
    munmap(shm_ptr, sizeof(struct shared));

    // Close the shared memory
    close(shm_fd);

    endwin();

    close(log_fd);

    return 0;
}


