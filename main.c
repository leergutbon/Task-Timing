/*--- main.c ---*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE     500
#define MAX_COMMANDS  10


/* struct to save command and aruments */
typedef struct commands{
  char *command;
  char **args;
  int beginTime;
  int endTime;
}Commands;



Commands **newCommandEntry(int numCmd, Commands **cmd);

int main(void){
  char inputStr[MAX_LINE];
  char *token = NULL;
  char *tmpArg;
  char **tmpArgArray;
  Commands **cmd;
  Commands **tmpCmd;
  int cntCom, cnt1, cnt2;

  /* read input line, string must be 501 because of last
     null or \n entry, not sure about this point */
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
  cntCom = 0;
  while(token != NULL){
    /*token = strtok(token, "\n");*/

    /* limited to 10 commands */
    if(cntCom < MAX_COMMANDS){
      tmpCmd = (Commands **)malloc(sizeof(Commands)*(cntCom+1));
      cnt1 = 0;
      while(cnt1 < (sizeof(cmd)/sizeof(cmd[0]))){
        tmpCmd[cnt1] = cmd[cnt1];
        cnt1++;
      }
      cmd = tmpCmd;
      cmd[cntCom] = (Commands *)malloc(sizeof(Commands *));
      cmd[cntCom]->command = token;
      cntCom++;
    }else{
      perror("error: more than 10 commands\n");
      return 1;
    }

    token = strtok(NULL, ";");
  }
  for(cnt1=0; cnt1<cntCom; cnt1++){
    tmpArg = cmd[cnt1]->command;
    /* get command without arguments */
    token = strtok(tmpArg, " ");
    cmd[cnt1]->command = token;
    /* create new argument array and copy from old */
    token = strtok(NULL, " ");
    cntCom = 0;
    while(token != NULL){
      tmpArgArray = (char **)malloc(sizeof(char *)*(cntCom+1));
      if(cntCom == 0){
        cmd[cnt1]->args = tmpArgArray;
        cmd[cnt1]->args[cntCom] = token;
      }else{
        for(cnt2=0; cnt2<cntCom; cnt2++){
          tmpArgArray[cnt2] = cmd[cnt1]->args[cnt2];
        }
        tmpArgArray[cntCom] = token;
        cmd[cnt1]->args = tmpArgArray;
      }
      /*printf("%s\n", cmd[cnt1]->args[cntCom]);*/
      cntCom++;
      token = strtok(NULL, " ");
    }
    printf("%s\n", cmd[cnt1]->command);
  }

  return 0;
}

