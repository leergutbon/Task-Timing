/*--- main.c ---*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE     500
#define MAX_COMMANDS  10


/* struct to save command and aruments */
typedef struct commands{
  int pid;
  char *command;
  /*char *args;*/
  char **argsArray;
  int beginTime;
  int endTime;
}Commands;


int main(void){
  char inputStr[MAX_LINE];
  char *token = NULL;
  char *tmpArg;
  char **tmpArgArray;
  Commands **cmd = NULL;
  Commands **tmpCmd;
  int cntCom, cnt1, cntArg, cnt2, i, childPid, pid, errno, status, exitValue;
  int pipes[2];
  char writebuf[sizeof(int)]={0};
  char readbuf[sizeof(int)]={0};

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

  /* init pipe */
  if(pipe(pipes)<0){
    perror("Pipe allocation failed");
    return 1;
  }

  /* split string in tokens and save in struct */
  token = strtok(inputStr, ";");
  cntCom = 0;
  while(token != NULL){
    /* limited to 10 commands */
    if(cntCom < MAX_COMMANDS){
      tmpCmd = (Commands **)malloc(sizeof(Commands)*(cntCom+1));
      cnt1 = 0;
      while(cnt1 < cntCom+1){
        if(cmd != NULL){
          tmpCmd[cnt1] = cmd[cnt1];
        }else{
          cmd = tmpCmd;
        }
        cnt1++;
      }
      cmd = tmpCmd;
      cmd[cntCom] = (Commands *)malloc(sizeof(Commands));
      cmd[cntCom]->command = token;
      cntCom++;
    }else{
      perror("error: more than 10 commands\n");
      return 1;
    }
    token = strtok(NULL, ";");
  }
  /* saves arguments in char array for each command */
  for(cnt1=0; cnt1<cntCom; cnt1++){
    if(cnt1 > 20){
      perror("error: to much arguments\n");
      return 1;
    }
    tmpArg = cmd[cnt1]->command;
    /* get command without arguments */
    token = strtok(tmpArg, " ");
    if(token != NULL){
      cmd[cnt1]->command = token;
      /*printf("%s\t",cmd[cnt1]->command);*/
      /* create new argument array and copy from old */
      token = strtok(NULL, " ");
      /*cmd[cnt1]->args = token;*/
      cntArg = 0;
      while(token != NULL){
        tmpArgArray = (char **)malloc(sizeof(char *)*(cntArg+1));
        if(cntArg == 0){
          cmd[cnt1]->argsArray = tmpArgArray;
          cmd[cnt1]->argsArray[cntArg] = token;
          /*printf("%s\n",cmd[cnt1]->argsArray[cntArg]);*/
        }else{
          for(cnt2=0; cnt2<cntArg; cnt2++){
            tmpArgArray[cnt2] = cmd[cnt1]->argsArray[cnt2];
          }
          tmpArgArray[cntArg] = token;
          cmd[cnt1]->argsArray = tmpArgArray;
        }
        cntArg++;
        token = strtok(NULL, " ");
      }
    }

    if(strlen(cmd[cnt1]->command) > 21){
      perror("error: command to long");
      return 1;
    }
  }

  for(i = 0; i < cntCom; i++){
    printf("%s\n", cmd[i]->command);
    childPid = fork();

    if(childPid < 0){
      printf("Fork of %s failed", cmd[i]->command);
    }else if(childPid == 0){
      sprintf(writebuf, "%d", getpid());
      /* exitValue is not 0 if the command can not be executed */
      exitValue = execvp(cmd[i]->command, cmd[i]->argsArray);
      exit(exitValue);
    }else{
      cmd[i]->pid = childPid;
      /* Parent process */
      printf("Process pid %d\n", getpid());
      printf("Process pid %d\n", cmd[i]->pid);
    }
  }
  
  /* wait for all children */
  while((pid = waitpid(-1, &status,0))){
    if(errno == ECHILD){
      break;
    }
    /*printf("Exit status of %d was %d \n", (int)pid, WEXITSTATUS(status));*/
  }

  for(i = 0; i < cntCom; i++){
    
    printf("%s : PID: %d\n",cmd[i]->command,  cmd[i]->pid);
  }

  return 0;
}

