/*
 * Jason Kepler
 * Process Manager (PMan)
 */

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>        //isdigit()
#include "linked_list.h"
#include "pman_helper.h"


pid_t valid_pid_format(char *str_pid);
int print_formatted_stat(char *file_stream);

/*
 * Constants and global variables
 */
#define USLEEP_TIME 100000
#define MAX_INPUT_SIZE 50
#define MAX_FILE_STREAM 2048
#define ITEMS_IN_PROC_STAT 52
#define COMM_INDEX 1
#define STATE_INDEX 2
#define UTIME_INDEX 13
#define STIME_INDEX 14
#define RSS_INDEX 23
#define VOL_CTEXT_INDEX 53
#define NONVOL_CTEXT_INDEX 54

Node* head = NULL;


/*
 * Starts given process running in background
 */
void bg(char **cmd) {
    pid_t pid;

    pid = fork();
    if (pid == 0) { //child process
      execvp(cmd[1], &cmd[1]);
      printf("ERROR: execvp failed\n");
      exit(-1);
    }
    else if (pid > 0) { //parent process
      char *real_path = realpath(cmd[1], NULL);
      head = add_new_node(head, pid, real_path);
      usleep(USLEEP_TIME);
    }
    else {
      printf("ERROR: fork failed\n");
    }
}


/*
 * Prints pid and path for each process, then number of processes
 */
void bg_list(char **cmd) {
  // printf("\n"); // whitespace
  print_nodes(head);
  printf("Total background jobs: \t%d\n", get_length(head));
}


/*
 * Kills the given process if it exists
 */
void bg_kill(char * str_pid) {

  pid_t pid = valid_pid_format(str_pid);

  if(pid >= 0 && pid_exists(head, pid)) {
    if (kill(pid, SIGKILL) == 0) {
      head = delete_node(head, pid);
      printf("killed process %d\n", pid);
    }
    else {
      printf("ERROR: failed to kill %d\n", pid);
    }
  }
  else {
    printf("ERROR: not a valid pid\n");
  }
}


/*
 * Stops the given process
 */
void bg_stop(char * str_pid) {

  pid_t pid = valid_pid_format(str_pid);

  if(pid >= 0) {
    if(pid_exists(head, pid)) {
      if (kill(pid, SIGSTOP) == 0) {
        printf("stopped process %d\n", pid);
      }
      else {
        printf("ERROR: failed to stop %d\n", pid);
      }
    }
    else {
      printf("ERROR: %d is not a bg process\n", pid);
    }
  }
  else {
    printf("ERROR: not a valid pid\n");
  }
}


/*
 * Re-starts the given process
 */
void bg_start(char * str_pid) {

  pid_t pid = valid_pid_format(str_pid);

  if(pid >= 0) {
    if(pid_exists(head, pid)) {
      if (kill(pid, SIGCONT) == 0) {
        printf("started process %d\n", pid);
      }
      else {
        printf("ERROR: failed to start %d\n", pid);
      }
    }
    else {
      printf("ERROR: %d is not a bg process\n", pid);
    }
  }
  else {
    printf("ERROR: not a valid pid\n");
  }
}


/*
 * Prints information about given process by accessing information from
 * proc/[pid]/stat and proc/[pid]/status in the PCB
 */
void pstat(char * str_pid) {

  pid_t pid = valid_pid_format(str_pid);

  if(pid >= 0) {
    if(pid_exists(head, pid)) {
      FILE *fp;
      char stat_file_stream[MAX_FILE_STREAM];
      char status_file_stream[55][1024];
      char stat_path[128];
      char status_path[128];

      // get the psudo file path for stat and status
      sprintf(stat_path, "/proc/%d/stat", pid);
      sprintf(status_path, "/proc/%d/status", pid);

      fp = fopen(stat_path, "r");
      if(fp != NULL) {
        fgets(stat_file_stream, sizeof(stat_file_stream) - 1, fp);
        fclose(fp);

        print_formatted_stat(stat_file_stream);
      }
      else {
        printf("ERROR: couldn't read stat\n");
      }

      fp = fopen(status_path, "r");
      if(fp != NULL) {
        int index = 0;
        int stream_size = sizeof(status_file_stream) - 1;
        while(fgets(status_file_stream[index], stream_size, fp) != NULL) {
          index++;
        }
        fclose(fp);

        printf("%s", status_file_stream[VOL_CTEXT_INDEX]);      // string ends with \n
        printf("%s", status_file_stream[NONVOL_CTEXT_INDEX]);   // string ends with \n
      }
      else {
        printf("ERROR: couldn't read status\n");
      }
    }
    else {
      printf("ERROR: %d is not a bg process\n", pid);
    }
  }
  else {
    printf("ERROR: not a valid pid\n");
  }
}


/*
 * Kills all processes, then frees allocated memory in list
 */
void bg_quit() {
  kill_all_nodes(head, SIGKILL);
  free_all_nodes(head);
  printf("Bye Bye \n");
  exit(0);
}


pid_t valid_pid_format(char *str_pid) {
  
  if(str_pid == NULL) {
    return -1;
  }

  for(int i = 0; i < strlen(str_pid); i++) {
    if(!isdigit(str_pid[i])) {
      return -1;
    }
  }
  return atoi(str_pid);
}


/*
 * Removes exited and terminated processes from list
 */
void remove_terminated_jobs() {
  int pid;
  int status;

  while(true) {
    pid = waitpid(-1, &status, WNOHANG);
    if(pid > 0) {
      if (WIFEXITED(status)) {
        printf("process %d exited\n", pid);
        head = delete_node(head, pid);   // remove programs that exit (ls, pwd, etc)
      }
      if (WIFSIGNALED(status)) {
        printf("process %d terminated\n", pid);
        head = delete_node(head, pid);
      }
    }
    else {
      break;  //no changes
    }
  }
}


/*
 * Prints comm, state, utime, stime and rss given the file stream
 * from proc/[pid]/stat
 * Returns: 0 if successful, -1 otherwise
 */
int print_formatted_stat(char *file_stream) {
  char * stat_list[ITEMS_IN_PROC_STAT];
  char delims[3] = " \n";
  if(tokenize(file_stream, stat_list, delims, ITEMS_IN_PROC_STAT) < 0) {
    return -1;
  }

  float ticks_per_second = sysconf(_SC_CLK_TCK);
  float utime = atof(stat_list[UTIME_INDEX]) / ticks_per_second;
  float stime = atof(stat_list[STIME_INDEX]) / ticks_per_second;

  // printf("\n");   // whitespace
  printf("comm: %s\n", stat_list[COMM_INDEX]);
  printf("state: %s\n", stat_list[STATE_INDEX]);
  printf("utime: %f seconds\n", utime);
  printf("stime: %f seconds\n", stime);
  printf("rss: %s\n", stat_list[RSS_INDEX]);

  return 0;
}

/*
 * PMan: Prompts the user for input and executes given commands
 */
int main() {
    char user_input_str[50];
    while (true) {
      printf("Pman: > ");
      fgets(user_input_str, 50, stdin);

      char * lst[MAX_INPUT_SIZE];
      char delims[3] = " \n";
      if(tokenize(user_input_str, lst, delims, MAX_INPUT_SIZE) < 0) {
        continue;
      }

      remove_terminated_jobs();

      if (strcmp("bg",lst[0]) == 0) {
        bg(lst);
      } else if (strcmp("bglist",lst[0]) == 0) {
        bg_list(lst);
      } else if (strcmp("bgkill",lst[0]) == 0) {
        bg_kill(lst[1]);
      } else if (strcmp("bgstop",lst[0]) == 0) {
        bg_stop(lst[1]);
      } else if (strcmp("bgstart",lst[0]) == 0) {
        bg_start(lst[1]);
      } else if (strcmp("pstat",lst[0]) == 0) {
        pstat(lst[1]);
      } else if (strcmp("q",lst[0]) == 0) {
        bg_quit();
      } else {
        printf("Invalid input\n");
      }

      remove_terminated_jobs();
    }

  return 0;
}

