#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0

int signit = 0;

void RunCommand(int, Command * );
void DebugPrintCommand(int, Command * );
void PrintPgm(Pgm * );
void stripwhite(char * );

void sig_handler() {
  signit = 1;
  return;
}

int main(void) {
  Command cmd;
  int parse_result;

  while (TRUE) {
    char * line;
    //printf("Here...............");

    line = readline("> ");
    /* If EOF encountered, exit shell */
    if (!line) {
      break;
    }

    /* Remove leading and trailing whitespace from the line */
    stripwhite(line);
    /* If stripped line not blank */
    if ( * line) {
      add_history(line);
      parse_result = parse(line, & cmd);
      RunCommand(parse_result, & cmd);
    }
  

    //  We call the signal handler for cntrl-c again and again because it misses catching the interrupt sometimes

    signal(SIGINT, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGINT, sig_handler);
    if (signit == 1) {
      continue;  // Bring back prompt after cntrl-c is triggered 
      signit = 0;
    }
    free(line);
  }
  return 0;
}

/* Execute the given command(s).

 * Note: The function currently only prints the command(s).
 * 
 * TODO: 
 * 1. Implement this function so that it executes the given command(s).
 * 2. Remove the debug printing before the final submission.
 */

char * stringbreak(Pgm * p) {

  static char temp[100] = "";
  memset(temp, 0, strlen(temp));
  if (p == NULL) {
    return " ";
  } else {
    char ** pl = p -> pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    while ( * pl) {

      strcat(temp, * pl++);
      strcat(temp, " ");

    }

  }

  temp[strlen(temp) - 1] = '\0';
  return temp;

}

void RunCommand(int parse_result, Command * cmd) {

  char input[100][10];
  int counter = 0;
  while (cmd -> pgm != NULL) {
    char temp[100] = "";
    strcpy(temp, stringbreak(cmd -> pgm));   // A custom stringbreak definition helps in the input string manipilation for the execution
    strcpy(input[counter++], temp);
    cmd -> pgm = cmd -> pgm -> next;
  }

  // Printing all inputs squentially
  //for (int j = counter - 1; j >= 0; j--) {

    //printf("%s ", input[j]);
    //printf("\n");

  //}

  char * args[64];
  char ** next = args;
  char temp1[20];
  strcpy(temp1, input[0]);
  char * ptr = strtok(temp1, " \n");        // delimiting and splitting strings with space

  while (ptr != NULL) {
    * next++ = ptr;
    ptr = strtok(NULL, " \n");
  }

  * next = NULL;

  //  We call the signal handler for cntrl-c again and again because it misses catching the interrupt sometimes
  signal(SIGINT, sig_handler);
  signal(SIGINT, sig_handler);
  signal(SIGINT, sig_handler);
  signal(SIGINT, sig_handler);
  signal(SIGINT, sig_handler);
  signal(SIGINT, sig_handler);
  
  if (signit == 1) { //flag to check interrupts 
    signit = 0;
    return;
  }
  int num_of_pipes = counter;  //counter increamented after each token of commands inside cmd->pgm  

  //Pipe (single and multiple)

  if (num_of_pipes > 1) {

    int fd[num_of_pipes][2];
    for (int i = 0; i < num_of_pipes; i++) {
      int counter = num_of_pipes;
      char * arg[64];
      char ** next1 = arg;
      char * ptr1 = strtok(input[num_of_pipes - (i + 1)], " \n"); // delimiting with space

      while (ptr1 != NULL) {
        * next1++ = ptr1;
        ptr1 = strtok(NULL, " \n");
      }

      * next1 = NULL;

      if (i != num_of_pipes - 1) {
        if (pipe(fd[i]) < 0) {
          perror("pipe creating was not successfull\n");
          return;
        }
      }
      
      if (fork() == 0) { 
          if (i != num_of_pipes - 1) {
          dup2(fd[i][1], 1);
          close(fd[i][0]);
          close(fd[i][1]);
        }

        if (i != 0) {
          dup2(fd[i - 1][0], 0);
          close(fd[i - 1][1]);
          close(fd[i - 1][0]);
        }
        execvp(arg[0], arg);
        perror("ERROR: Invalid command (pipes)\n");
        exit(1); 
      }
      //parent
      if (i != 0) {
        close(fd[i - 1][0]);
        close(fd[i - 1][1]);
      }

      wait(NULL);
    }
    if (signit == 1) {
      signit = 0;
      return;
    }
  }

  // Commands running in the background

  if (cmd -> background) {
    printf("background process\n");
    int pid = fork();
    if (pid < 0) {
      return;
    }

    if (pid == 0) {
      execvp(args[0], args);
      if (execvp(args[0], args) < 0) {
        printf("ERROR: Invalid command (bg)\n");
        return;
      }
    } else if (!cmd -> background) {
      waitpid(pid, NULL, 0);
    }
  }

  //Commands with re-directional operators

  if (cmd -> rstdin || cmd -> rstdout) {
    //printf("stdin or stdout\n");
    int pid = fork();
    if (pid == 0) {
      if (cmd -> rstdin) {
        //printf("stdin .......\n");
        int fd0 = open(cmd -> rstdin, O_RDONLY);
        dup2(fd0, STDIN_FILENO);
        close(fd0);
      }

      if (cmd -> rstdout) {
        //printf("stdout......\n");

        int fd1 = open(cmd -> rstdout,  O_WRONLY | O_APPEND | O_CREAT);
        dup2(fd1, STDOUT_FILENO);
        close(fd1);
      }
      execvp(args[0], args);
      if (execvp(args[0], args) < 0) {
        printf("ERROR: Invalid command (stdin/out)\n");
        return;
      }

    }
    waitpid(pid, NULL, 0);


  // Single commands with options

  } else {
    if (counter == 1) {

      if (strcmp(args[0], "exit") == 0) {
        printf("quitting from shell\n");
        execlp("exit", "exit", NULL);
        exit(0);

      }

      int pid = fork();
      if (pid == 0) {
        execvp(args[0], args);
        if (execvp(args[0], args) < 0) {
          printf("ERROR: Invalid command (s)\n");
          return;
        }
      }
      waitpid(pid, NULL, 0);

      //waitpid(pid, NULL, 0);

    }
  }

  //DebugPrintCommand(parse_result, cmd);
}

/* 
 * Print a Command structure as returned by parse on stdout. 
 * 
 * Helper function, no need to change. Might be useful to study as inpsiration.
 */
void DebugPrintCommand(int parse_result, Command * cmd) {
  if (parse_result != 1) {
    printf("Parse ERROR\n");
    return;
  }
  printf("------------------------------\n");
  printf("Parse OK\n");
  printf("stdin:      %s\n", cmd -> rstdin ? cmd -> rstdin : "<none>");
  printf("stdout:     %s\n", cmd -> rstdout ? cmd -> rstdout : "<none>");
  printf("background: %s\n", cmd -> background ? "true" : "false");
  printf("Pgms:\n");
  PrintPgm(cmd -> pgm);
  printf("------------------------------\n");
}

/* Print a (linked) list of Pgm:s.
 * 
 * Helper function, no need to change. Might be useful to study as inpsiration.
 */
void PrintPgm(Pgm * p) {

  if (p == NULL) {
    return;
  } else {
    char ** pl = p -> pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    PrintPgm(p -> next);
    printf("            * [ ");
    while ( * pl) {

      printf("%s ", * pl++);
    }
    printf("]\n");

  }

}

/* Strip whitespace from the start and end of a string. 
 *
 * Helper function, no need to change.
 */
void stripwhite(char * string) {
  register int i = 0;

  while (isspace(string[i])) {
    i++;
  }

  if (i) {
    strcpy(string, string + i);
  }

  i = strlen(string) - 1;
  while (i > 0 && isspace(string[i])) {
    i--;
  }

  string[++i] = '\0';
}