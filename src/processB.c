
// Group: CDGG
// Authors:
// Claudio Demaria S5433737
// Gianluca Galvagni S5521188

#include "./../include/processB_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <bmpfile.h>
#include <math.h>
#include <semaphore.h>

#define SHM_SIZE sizeof(struct shared)

// Define the size of the shared memory
const int width = 1600;
const int height = 600;
const int depth = 4;

// Define the struct of the shared memory
//struct shared
//{
  //  int m[1600][600];
//};

// Define the semaphores
sem_t *semaphore;
sem_t *semaphore2;

// Buffer to store the string to write to the log file
char log_buffer[100];
// File descriptor for the log file
int log_fd;

void draw_blue_circle(int radius, int x, int y, uint8_t *img, int img_width, int img_height) {
    // Loop over the pixels of the circle
    for(int i = -radius; i <= radius; i++) {
        for(int j = -radius; j <= radius; j++) {
            // If distance is smaller than the radius, point is within the circle
            if(sqrt(i*i + j*j) < radius) {
                // Check if the point is inside the image boundaries
                if (x + i >= 0 && x + i < img_width && y + j >= 0 && y + j < img_height) {
                    // Color the pixel at the specified (x, y) position blue
                    *(img + (x + i) * img_height + y + j) = 255;
                }
            }
        }
    }
}

void cancel_blue_circle(int radius, int x, int y, uint8_t *img, int img_width, int img_height) {
    // Loop over the pixels of the circle
    for(int i = -radius; i <= radius; i++) {
        for(int j = -radius; j <= radius; j++) {
            // If distance is smaller than the radius, point is within the circle
            if(sqrt(i*i + j*j) < radius) {
                // Check if the point is inside the image boundaries
                if (x + i >= 0 && x + i < img_width && y + j >= 0 && y + j < img_height) {
                    // Color the pixel at the specified (x, y) position white
                    *(img + (x + i) * img_height + y + j) = 255;
                }
            }
        }
    }
}


int main(int argc, char const *argv[])
{
    // Variable declaration
    int shm_fd;
    // Pointer to the shared memory
    struct shared *shm_ptr;
    //sem_t *sem;

    // Open a file descriptor to the shared memory object
    shm_fd = shm_open("my_shm", O_RDONLY, 0);
    if (shm_fd < 0) {
        printf("Error opening shared memory object\n");
        return 1;
    }

    // Truncate the shared memory object to the size of the struct shared
    if (ftruncate(shm_fd, SHM_SIZE) < 0) {
        printf("Error truncating shared memory object\n");
        return 1;
    }

    // Map the shared memory object to the memory space
    shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        printf("Error mapping shared memory object to memory space\n");
        return 1;
    }

    // Open the log file
    log_fd = open("log/processB.log",O_WRONLY|O_APPEND|O_CREAT, 0666)
    if (log_fd == -1){
        // If the file could not be opened, print an error message and exit
        perror("Error opening command file");
        exit(1);
    }

    // Map the shared memory object to the memory space
    shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        printf("Error mapping shared memory object to memory space\n");
        return 1;
    }

    // Create the semaphore to control access to the shared memory
    sem = sem_open("my_sem", O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (sem == SEM_FAILED) {
        printf("Error creating semaphore\n");
        return 1;
    }

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Get the key of the shared memory
    //ShmKEY = ftok(".", 'x');

    // Get the ID of the shared memory
    //ShmID = shmget(ShmKEY, sizeof(struct shared), 0666);
    //if (ShmID < 0) {
        //printf("*** shmget error (server) ***\n");
        //exit(1);
    //}

    // Attach the shared memory to the pointer
    //ShmPTR = (struct shared *) shmat(ShmID, NULL, 0);
    //if ((int) ShmPTR == -1) {
      //  printf("*** shmat error (server) ***\n");
        //exit(1);
    //}

    // Create the bitmap
    bmpfile_t *bmp;
    bmp = bmp_create(width, height, depth);
    if (bmp == NULL) {
        printf("Error: unable to create bitmap\n");
        exit(1);
    }

    // Variable declaration in order to store the coordinates of the circles
    int center_cord = 0;
    int x_cord[600];
    int y_cord[600];
    int y;
    int y_old;
    int x_old;

    // Flag
    int flag ;



    // Open the semaphore 1
    semaphore = sem_open("/mysem", 0);
    if (semaphore == (void*) -1)
    {
        perror("sem_open failure");
        exit(1);
    }

    // Open the semaphore 2
    semaphore2 = sem_open("/mysem2", 0);
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
            sem_wait(sem);




            // Get the coordinates of the circle from the shared memory
            int radius = shm_ptr->radius;
            int x = shm_ptr->x;
            int y = shm_ptr->y;






            // Initialize the coordinates of the circles
            for (int i=0;i<600;i++)
            {
                x_cord[i]=0;
                y_cord[i]=0;
            }
            center_cord = 0;

            int i, j;
            y = 0;
            flag = 0;
            
            // Get the coordinates of the circles
            for (i = 0; i < 1600; i++) {
                
                // If the flag is 1, break the loop
                if (flag == 1) {
                    break;
                }
                for (j = 0; j < 600; j++) {

                    // Get the coordinates of the circles from the shared memory
                    if (ShmPTR->m[i][j] == 1)
                    {
                        x_cord[y] = j;
                        y_cord[y] = i;

                        if (x_cord[y] > x_cord[y-1]) {
                            flag = 1;
                            break;
                        }

                        y++;
                        break;
                    }
                }
            }

            // Update the position of the center
            center_cord = x_cord[y-1] + 30;

            // Write the position of the center in the log file
            sprintf(log_buffer, "<Process_B> Position of center updated: %s\n", asctime(info));
            if (write(log_fd, log_buffer, strlen(log_buffer)) == -1)
            {
                perror("Error writing to log file");
                exit(1);
            }

            // Draw the circles
            mvaddch(floor((int)(center_cord/20)),floor((int)(y_cord[y-1]/20)), '0');

            refresh();

            // Signal the semaphore 1
            sem_post(sem);

            // Cancel the circle with the coordinates of the previous loop
            cancel_blue_circle(30,y_old,x_old,bmp);

            // Draw the circle with the coordinates of the current loop
            draw_blue_circle(30,y_cord[y-1],center_cord,bmp);   

            // Update the (previous) coordinates for the next loop
            y_old = y_cord[y-1];
            x_old = center_cord;           
        }
         
    }

    // Close the semaphores
    sem_close(semaphore);
    sem_close(semaphore2);

    // Detach and remove the shared memory
    //shmdt((void *) ShmPTR);
    //bmp_destroy(bmp);

    // Unmap the shared memory object from the memory space
    if (munmap(shm_ptr, SHM_SIZE) < 0) {
        printf("Error unmapping shared memory object\n");
        return 1;
    }

    // Close the file descriptor for the shared memory object
    if (close(shm_fd) < 0) {
        printf("Error closing file descriptor for shared memory object\n");
        return 1;
    }

    endwin();

    // Close the log file
    close(log_fd);

    return 0;
}
