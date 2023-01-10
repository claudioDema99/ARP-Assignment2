#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

// Variables to store the PIDs
pid_t pid_procA;
pid_t pid_procB;

// Buffer to store the string to write to the log file
char log_buffer[100];

// File descriptor for the log file
int log_fd;

// Variable to store the value of the write function
int check;

// Function to check the correctness of the operation just done
void CheckCorrectness(int c) 
{
    if(c == -1) 
    {
        close(log_fd);
        perror("Error in writing function");
        exit(1);
    }
}

int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}

// Function to get when a file was last modified
time_t get_last_modified(char *filename)
{
  struct stat attr;
  stat(filename, &attr);

  return attr.st_mtime;
}

int watchdog()
{
  // Array of the log file paths
  char *log_files[2] = {"processA.log", "processB.log"};

  // Array of the PIDs
  pid_t pids[2] = {pid_procA, pid_procB};

  // Flag to check if a file was modified
  int modified;

  // Variable to keep the number of seconds since the last modification
  int counter = 0;

  while (1)
  {
    // Get current time
    time_t current_time = time(NULL);

    // Loop through the log files
    for (int i = 0; i < 2; i++)
    {
      // Get the last modified time of the log file
      time_t last_modified = get_last_modified(log_files[i]);

      // Check if the file was modified in the last 3 seconds
      if (current_time - last_modified > 3)
      {
        modified = 0;
      }
      else
      {
        modified = 1;
        counter = 0;
      }
    }

    if (modified==0)
    {
      counter += 3;
    }

    // If the counter is greater than 60, kill the child processes
    if (counter > 60)
    {
      // Kill all the processes
      kill(pid_procA, SIGKILL);
      kill(pid_procB, SIGKILL);
      
      return 0;
    }

    // Sleep for 2 seconds
    sleep(2);
  }
}

int main() {

  // Open the log file
  if ((log_fd = open("master.log",O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1)
  {
    perror("Error opening log file");
    return 1;
  }
        
  // Get the time when the Master starts its execution
  time_t rawtime;
  struct tm *info;
  time( &rawtime );
  info = localtime( &rawtime );

  // Write into the log file
  sprintf(log_buffer, "<master_process> Master process started: %s\n", asctime(info));
  check = write(log_fd, log_buffer, strlen(log_buffer));
  CheckCorrectness(check);

  // Process A
  char * arg_list_A[] = { "/usr/bin/konsole", "-e", "./bin/processA", NULL };
  pid_procA = spawn("/usr/bin/konsole", arg_list_A);

  // Process B
  char * arg_list_B[] = { "/usr/bin/konsole", "-e", "./bin/processB", NULL };
  pid_procB = spawn("/usr/bin/konsole", arg_list_B);

  // // Create the log files
  int fd_pa = open("processA.log", O_CREAT | O_RDWR, 0666);
  int fd_pb = open("processB.log", O_CREAT | O_RDWR, 0666);
  
  // Check corecctness
  if(fd_pa <0 || fd_pb <0)
  {
    printf("Error opening FILE");
  }
 
  // Close the log files
  close(fd_pa);
  close(fd_pb);

  // Whatchdog funcion call
  watchdog();

  int status;

  // Check PIDs
  waitpid(pid_procA, &status, 0);
  waitpid(pid_procB, &status, 0);

  // Get the time when the Master finishes its execution
  time( &rawtime );
  info = localtime(&rawtime);

  // write into the log file
  sprintf(log_buffer, "<master_process> Master process terminated: %s\n", asctime(info));
  check = write(log_fd, log_buffer, strlen(log_buffer));
  CheckCorrectness(check);
  
  // Close the log file
  close(log_fd);
  
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

