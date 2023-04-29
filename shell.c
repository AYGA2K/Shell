#include "tubsem.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int fd;
char *myfifo = "./myfifo";
/*
  Function Declarations for builtin shell commands:
 */
void Initsem(Semaphore S, int N) {
  int i;
  char c = 'a';
  pipe(S);
  for (i = 1; i <= N; i++)
    write(S[1], &c, 1);
}
/* P sur le semaphore S, prendre un jeton 'a' */
void P(Semaphore S) {
  char c;
  read(S[0], &c, 1);
}
void V(Semaphore S) {
  char c = 'a';
  write(S[1], &c, 1);
}
int lsh_laister(char **args);

int lsh_supprimer(char **args);
int lsh_calculer(char **args);
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *mult_div[] = {"*", "/"};
char *add_sub[] = {"+", "-"};
int mult_div_positions[10];
int add_sub_positions[10];

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {"laister", "supprimer", "calculer",
                       "cd",      "help",      "exit"};
int (*builtin_func[])(char **) = {&lsh_laister, &lsh_supprimer, &lsh_calculer,
                                  &lsh_cd,      &lsh_help,      &lsh_exit};

int lsh_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

void calc(int num1, int num2, char *op) {
  int res;

  fd = open(myfifo, O_WRONLY);
  if (fd == -1) {
    fprintf(stderr, "Error while openning pipe");
  }

  if (strcmp(op, "+") == 0) {
    res = num1 + num2;
    write(fd, &res, sizeof(res));
  }
  if (strcmp(op, "-") == 0) {
    res = num1 - num2;
    write(fd, &res, sizeof(res));
  }
  if (strcmp(op, "*") == 0) {
    res = num1 * num2;
    write(fd, &res, sizeof(res));
  }
  if (strcmp(op, "/") == 0) {
    res = num1 / num2;
    write(fd, &res, sizeof(res));
  }
  close(fd);
}
int lsh_laister(char **args) {
  if (fork() == 0) {
    // Child process
    if (execvp("ls", args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  }
  return 1;
}
int lsh_supprimer(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"supprimer\"\n");
  }
  if (fork() == 0) {
    // Child process
    if (execvp("rm", args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  }
  return 1;
}
void lsh_seq_calculer(char **args) {
  // Get start time
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int i = 1;
  int mult_div_index = 0;
  int add_sub_index = 0;

  while (args[i] != NULL) {

    for (int j = 0; j < 2; j++) {
      if (strcmp(args[i], mult_div[j]) == 0) {
        for (int k = 0; k < 2; k++) {
          if (strcmp(args[i + 1], add_sub[k]) == 0 ||
              strcmp(args[i + 1], mult_div[k]) == 0) {
            fprintf(stderr, "Please write  a correct sentense \n");
            return;
          }
        }
        mult_div_positions[mult_div_index] = i;
        mult_div_index++;
      }
      if (strcmp(args[i], add_sub[j]) == 0) {
        for (int k = 0; k < 2; k++) {
          if (strcmp(args[i + 1], add_sub[k]) == 0 ||
              strcmp(args[i + 1], mult_div[k]) == 0) {
            fprintf(stderr, "Please write  a correct sentense \n");
            return;
          }
        }
        add_sub_positions[add_sub_index] = i;
        add_sub_index++;
      }
    }
    i++;
  }
  int tmp_pos;
  int num1;
  int num2;
  int res;
  if (mult_div_index > 0) {
    for (int l = 0; l < mult_div_index; l++) {
      tmp_pos = mult_div_positions[l];

      if (strcmp(args[tmp_pos], "*") == 0 || strcmp(args[tmp_pos], "/") == 0) {
        int a = 1;
        while (args[tmp_pos - a] == NULL) {
          a++;
        }
        sscanf(args[tmp_pos - a], "%d", &num1);

        a = 1;
        while (args[tmp_pos + a] == NULL) {
          a++;
        }

        sscanf(args[tmp_pos + a], "%d", &num2);
      }
      if (strcmp(args[tmp_pos], "*") == 0) {
        res = num1 * num2;
      }
      if (strcmp(args[tmp_pos], "/") == 0) {
        res = num1 / num2;
      }

      char tmp[256];

      sprintf(tmp, "%d", res);
      args[tmp_pos] = tmp;
      args[tmp_pos - 1] = NULL;
      args[tmp_pos + 1] = NULL;
    }
  }
  for (int l = 0; l < add_sub_index; l++) {
    tmp_pos = add_sub_positions[l];

    if (strcmp(args[tmp_pos], "+") == 0 || strcmp(args[tmp_pos], "-") == 0) {
      int a = 1;
      while (args[tmp_pos - a] == NULL) {
        a++;
      }
      sscanf(args[tmp_pos - a], "%d", &num1);

      a = 1;
      while (args[tmp_pos + a] == NULL) {
        a++;
      }

      sscanf(args[tmp_pos + a], "%d", &num2);
    }
    char tmp[256];
    if (strcmp(args[tmp_pos], "+") == 0) {
      res = num1 + num2;
    }
    if (strcmp(args[tmp_pos], "-") == 0) {
      res = num1 - num2;
    }

    sprintf(tmp, "%d", res);
    args[tmp_pos] = tmp;
    args[tmp_pos - 1] = NULL;
    args[tmp_pos + 1] = NULL;
  }
  struct timeval end_time;
  gettimeofday(&end_time, NULL);

  // Calculate elapsed time in microseconds
  long elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 +
                      end_time.tv_usec - start_time.tv_usec;

  // Print elapsed time in seconds and microseconds
  printf("Single Process calculation took %ld.%06ld seconds\n",
         elapsed_time / 1000000, elapsed_time % 1000000);

  fprintf(stdout, "Result is : %d \n", res);
}
int lsh_calculer(char **args) {

  if (fork() == 0) {
    lsh_seq_calculer(args);
    exit(0);
  } else {
    wait(NULL);
    Semaphore C;
    // Get start time
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Perform multiprocess calculation here

    // Get end time
    int res = 0;
    int num1;
    int num2;
    int mult_div_index = 0;
    int add_sub_index = 0;
    int i = 1;
    if (args[1] == NULL) {
      fprintf(stderr, "lsh: expected argument to \"calculer\"\n");
    } else {

      while (args[i] != NULL) {
        for (int j = 0; j < 2; j++) {
          if (strcmp(args[i], mult_div[j]) == 0) {
            for (int k = 0; k < 2; k++) {
              if (strcmp(args[i + 1], add_sub[k]) == 0 ||
                  strcmp(args[i + 1], mult_div[k]) == 0) {
                fprintf(stderr, "Please write  a correct sentense \n");
                return 1;
              }
            }
            mult_div_positions[mult_div_index] = i;
            mult_div_index++;
          }
          if (strcmp(args[i], add_sub[j]) == 0) {
            for (int k = 0; k < 2; k++) {
              if (strcmp(args[i + 1], add_sub[k]) == 0 ||
                  strcmp(args[i + 1], mult_div[k]) == 0) {
                fprintf(stderr, "Please write  a correct sentense \n");
                return 1;
              }
            }
            add_sub_positions[add_sub_index] = i;
            add_sub_index++;
          }
        }
        i++;
      }
      int R1;
      int R2;
      int R3;
      int R4;
      if (mult_div_index > 0) {

        for (int l = 0; l < mult_div_index; l++) {
          int tmp_pos = mult_div_positions[l];

          if (strcmp(args[tmp_pos], "*") == 0 ||
              strcmp(args[tmp_pos], "/") == 0) {
            int a = 1;
            while (args[tmp_pos - a] == NULL) {
              a++;
            }
            sscanf(args[tmp_pos - a], "%d", &num1);

            a = 1;
            while (args[tmp_pos + a] == NULL) {
              a++;
            }

            sscanf(args[tmp_pos + a], "%d", &num2);
          }
          if (fork() == 0) {
            calc(num1, num2, args[tmp_pos]);
            exit(0);
          }
          fd = open(myfifo, O_RDONLY);
          read(fd, &res, sizeof(res));

          close(fd);
          char tmp[256];

          sprintf(tmp, "%d", res);
          args[tmp_pos] = tmp;
          args[tmp_pos - 1] = NULL;
          args[tmp_pos + 1] = NULL;
        }
      }
      if (add_sub_index > 0) {

        for (int l = 0; l < add_sub_index; l++) {
          int tmp_pos = add_sub_positions[l];

          if (strcmp(args[tmp_pos], "+") == 0 ||
              strcmp(args[tmp_pos], "-") == 0) {
            int a = 1;
            while (args[tmp_pos - a] == NULL) {
              a++;
            }
            sscanf(args[tmp_pos - a], "%d", &num1);

            a = 1;
            while (args[tmp_pos + a] == NULL) {
              a++;
            }

            sscanf(args[tmp_pos + a], "%d", &num2);
          }
          if (fork() == 0) {
            calc(num1, num2, args[tmp_pos]);
            exit(0);
          }
          fd = open(myfifo, O_RDONLY);
          read(fd, &res, sizeof(res));

          close(fd);
          char tmp[256];

          sprintf(tmp, "%d", res);
          args[tmp_pos] = tmp;
          args[tmp_pos - 1] = NULL;
          args[tmp_pos + 1] = NULL;
        }
      }
    }
    struct timeval end_time;
    gettimeofday(&end_time, NULL);

    // Calculate elapsed time in microseconds
    long elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 +
                        end_time.tv_usec - start_time.tv_usec;

    // Print elapsed time in seconds and microseconds
    printf("Multiprocess calculation took %ld.%06ld seconds\n",
           elapsed_time / 1000000, elapsed_time % 1000000);

    fprintf(stdout, "Result is : %d \n", res);
    return 1;
  }
}

int lsh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_help(char **args) {
  int i;
  printf("Ayoub GAROUAT's shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int lsh_exit(char **args) { return 0; }

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int lsh_launch(char **args) {
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void) {
#define LSH_RL_BUFSIZE 1024
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line) {
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv) {
  // Load config files, if any.
  int R = mknod(myfifo, S_IFIFO | 0666, 0);
  if (R == -1 && errno != EEXIST) {
    perror("Pb de creation du tube nommé");
    exit(2);
  }
  // Run command loop.
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
