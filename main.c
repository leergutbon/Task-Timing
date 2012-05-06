#include <stdio.h>
#include <string.h>

/* struct to save command and aruments */
struct commands{
  char command[20];
  char **args;
  int beginTime;
  int endTime;
} Commands;


int main(void){
  char inputStr[500];
  char *token;
  char *arguments;
  /*Commands *cmd;*/

  /* read input line, string must be 501 because of last null entry */
  printf("> ");
  if(fgets(inputStr, 501, stdin) == NULL){
    perror("error: input stream\n");
    return 1;
  }
  if(strlen(inputStr) > 501){
    perror("error: input string to long\n");
    return 1;
  }

  /* split string in tokens and save in struct */
  token = strtok(inputStr, ";");
  while(token != NULL){
    token = strtok(token, "\n");
    if(strlen(token) > 20){
      perror("error: command to long\n");
      return 1;
    }

    

    printf("%s\n", token);
    token = strtok(NULL, ";");
  }

  return 0;
}

