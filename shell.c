#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128
#define MAX_WORD_LENGTH 256

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;
int fd;


char* get_env_var(char* env_var_string) {
  char* new_env_var_string = env_var_string + 1;
  char* env_var_value = getenv(new_env_var_string);
  return env_var_value;
}

void sighandler(signum) {
  signal(SIGINT, sighandler);
  signal(SIGALRM, sighandler);
  dup2(fd, STDOUT_FILENO);
  return;
}

int main() {
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
  
    // Stores the tokenized command line input.
    char cwd[256]; 

    char empty_array[MAX_COMMAND_LINE_ARGS] = {'\0'};
    char **env = environ;
    int env_idx = 0;
    int i;
    for (i = 0; i < sizeof(env)/sizeof(env[0]); i++) {
      if (env[i] != NULL) {
        env_idx++;
      }
    }
    	
    while (true) {
      signal(SIGINT, sighandler);
      getcwd(cwd, sizeof(cwd));
        do{ 
            // Print the shell prompt.
            printf("%s%s", cwd, prompt);
            fflush(stdout);

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")
        
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
 
        }while(command_line[0] == 0x0A);  // while just ENTER pressed

      
        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

        // TODO:
			  // 0. Modify the prompt to print the current working directory
        // 1. Tokenize the command line input (split it on whitespace)
        char *arguments[MAX_COMMAND_LINE_ARGS];
        int in_word = 1;
        char word[MAX_WORD_LENGTH] = {'\0'};
        char empty_word[MAX_WORD_LENGTH] = {'\0'};
        char* token;
        int i;
        int word_idx = 0;
        int arguments_idx = 0;
        for (i = 0; i <= strlen(command_line); i++){
          token = &command_line[i];
          if (in_word){
            if (strncmp(token, "\n", 1) != 0 && strncmp(token, " ", 1) != 0){
              word[word_idx] = *token;
              word_idx++;
            }
            else if (strncmp(token, " ", 1) == 0){
              arguments[arguments_idx] = malloc(MAX_WORD_LENGTH * sizeof(char));
              strcpy(arguments[arguments_idx], word);
              arguments_idx++;
              memcpy(word, empty_word, MAX_WORD_LENGTH);
              in_word = 0;
              word_idx = 0;
            }
            else if (strncmp(token, "\n", 1) == 0) {
              arguments[arguments_idx] = malloc(MAX_WORD_LENGTH * sizeof(char));
              strcpy(arguments[arguments_idx], word);
              break;
            }
          }
          else { 
            if (strncmp(token, "\n", 1) != 0 && strncmp(token, " ", 1) != 0){ 
              in_word = 1;
              word[word_idx] = *token;
              word_idx++;
            }
            else if (strncmp(token, "\n", 1) == 0) {
              break;
            }
          }
        }
        
        // 2. Implement Built-In Commands

        // pwd
        if (strcmp(arguments[0], "pwd") == 0){ 
          printf("%s\n", cwd);
        }

        // cd
        else if (strcmp(arguments[0], "cd") == 0){
          if (strncmp(arguments[1], "$", 1) == 0){
              char* env_var = get_env_var(arguments[i]);
              strcpy(arguments[1], env_var);
            } 
          chdir(arguments[1]);
        }

        // echo
        else if (strcmp(arguments[0], "echo") == 0){ 
          int i;
          for (i = 1; i < sizeof(arguments)/sizeof(arguments[0]); i++){
            if (arguments[i] == NULL){
              break;
            }
            if (strncmp(arguments[i], "$", 1) == 0){
              char* env_var = get_env_var(arguments[i]);
              printf("%s ", env_var); 
            }
            else{
              printf("%s ", arguments[i]);
            }
          }
          printf("\n");
        }

        // exit
        else if (strcmp(arguments[0], "exit") == 0){ 
          exit(0);
        }

        // env
        else if (strcmp(arguments[0], "env") == 0){ 
          int i;
          if (arguments[1] != NULL) {
            printf("%s", arguments[1]);
            for (i = 1; i < sizeof(arguments)/sizeof(arguments[0]); i++){
              if (arguments[i] == NULL){
                break;
              }
              if (strncmp(arguments[i], "$", 1) == 0){
                char* env_var = get_env_var(arguments[i]);
                printf("%s ", env_var); 
              }
              else{
                char* env_var = getenv(arguments[i]);
                printf("%s ", env_var);
              }
            }
          }
          else {
            for (i = 0; i < sizeof(env); i++){
              printf("%s\n", env[i]);
            }
          }
          printf("\n");
        }

        // setenv
        else if (strcmp(arguments[0], "setenv") == 0){ 
          int i;
          for (i = 1; i < sizeof(arguments)/sizeof(arguments[0]); i++){
            if (arguments[i] == NULL){
              break;
            }
            printf("%s\n", arguments[i]);
            int contains_equals = 0;
            char empty_array2[MAX_WORD_LENGTH] = {'\0'}; 
            char set_env_var[MAX_WORD_LENGTH] = {'\0'};
            int env_var_idx = 0;
            char* name_and_value[2];
            char env_token;
            int j;
            char* arg = arguments[i];
            for (j = 0; j <= strlen(arguments[i]); j++){
              env_token = arg[j];
              if (env_token != '\n' && env_token != ' ' && env_token != '=' && env_token != '\0'){
                set_env_var[env_var_idx] = env_token;
                env_var_idx++;
              }
              else if (env_token == '='){
                name_and_value[0] = malloc(MAX_WORD_LENGTH * sizeof(char));
                strcpy(name_and_value[0], set_env_var);
                memcpy(set_env_var, empty_array2, MAX_WORD_LENGTH);
                contains_equals = 1;
                env_var_idx = 0;
              }
              else if (env_token == '\n'|| env_token == ' ' || env_token == '\0') {
                name_and_value[1] = malloc(MAX_WORD_LENGTH * sizeof(char));
                strcpy(name_and_value[1], set_env_var);
                break;
              }
            }
            
            char* name;
            char* value;

            if (strncmp(name_and_value[0], "$", 1) == 0){
              name = get_env_var(name_and_value[0]);
            }
            else{
              name = name_and_value[0];
            }
            if (strncmp(name_and_value[1], "$", 1) == 0){
              value = get_env_var(name_and_value[1]);
            }
            else {
              value = name_and_value[1];
            }
            setenv(name, value, 1);
            env[env_idx] = arguments[i];
            env_idx++;
          }
        }

        // 3. Create a child process which will execute the command line input
        else {
          int i;
          int background = 0;
          for (i = 0; i < sizeof(arguments)/sizeof(arguments[0]); i++) {
            if (arguments[i] == NULL && strncmp(arguments[i-1], "&", 1) == 0) {
              arguments[i-1] = '\0';
              background = 1;
              break;
            }
            else if (arguments[i] == NULL) {
              break;
            }
          }

          pid_t child_pid = fork();
          if (background) {
            fd = open("outfile", O_WRONLY | O_CREAT);
            if (child_pid == 0) {
              dup2(fd, STDOUT_FILENO);
              close(fd);
              int status = execvp(arguments[0], arguments);
              if (status == -1) {
                perror("Error");
                exit(1);
              }
              exit(0);
            }
          }
          else {
            if (child_pid == 0) {
              fd = open("outfile", O_WRONLY | O_CREAT);
              signal(SIGALRM, sighandler);
              alarm(10);
              close(fd);
              int status = execvp(arguments[0], arguments);
              if (status == -1) {
                perror("Error");
                exit(1);
              }
                exit(0);
            }
            else {
              wait(NULL);
            }
          }
        }
  
        // empty arguments array
        memcpy(arguments, empty_array, MAX_COMMAND_LINE_ARGS);

        // 4. The parent process should wait for the child to complete unless its a background process
      
      
        // Hints (put these into Google):
        // man fork
        // man execvp
        // man wait
        // man strtok
        // man environ
        // man signals
        
        // Extra Credit
        // man dup2
        // man open
        // man pipes
    }
    // This should never be reached.
    return -1;
}