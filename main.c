/*--- hu1.c ----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
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
  clock_t st_time;
  clock_t en_time;
  struct tms st_cpu;
  struct tms en_cpu;
}Commands;


int main(void){
  char inputStr[MAX_LINE];
  char *token = NULL;
  char *tmpArg;
  char **tmpArgArray;
  Commands **cmd;
  Commands **tmpCmd;
  int cntCom, cnt1, cntArg, cnt2, i, childPid, pid, errno, status, exitValue;
  int pipefd[2];
  pipe(pipefd);

  while(1){
    /* set cmd to NULL for next loop pass */
    cmd = NULL;
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
        /* create new argument array and copy from old */
        token = strtok(NULL, " ");
        cntArg = 0;
        /* every argument gets his array entry */
        while(token != NULL){
          tmpArgArray = (char **)malloc(sizeof(char *)*(cntArg+1));
          /* first element is null */
          if(cntArg == 0){
            cmd[cnt1]->argsArray = tmpArgArray;
            cmd[cnt1]->argsArray[cntArg] = token;
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
      /*printf("%s\n", cmd[i]->command);*/
      childPid = fork();

      if(childPid < 0){
        printf("Fork of %s failed", cmd[i]->command);
      }else if(childPid == 0){
        close(pipefd[0]);    /* close reading end in the child */

        dup2(pipefd[1], 1);  /* send stdout to the pipe */

        close(pipefd[1]);
        /* begin time measure */
        cmd[i]->st_time = times(&cmd[i]->st_cpu);

        /* exitValue is not 0 if the command can not be executed */
        exitValue = execvp(cmd[i]->command, cmd[i]->argsArray);
        exit(exitValue);
      }else{ /* Parent process */
        cmd[i]->pid = childPid;
        /*printf("Process pid %d\n", getpid());
        printf("Process pid %d\n", cmd[i]->pid);*/
      }
    }
    
    /* wait for all children */
    while((pid = waitpid(-1, &status,0))){
      for(i=0; i<cntCom; i++){
        /* end time measure */
        if(cmd[i]->pid == (int)pid){
          cmd[i]->en_time = times(&cmd[i]->en_cpu);
        }
      }
      if(errno == ECHILD){
        break;
      }
      /*printf("Exit status of %d was %d \n", (int)pid, WEXITSTATUS(status));*/
    }

    for(i = 0; i < cntCom; i++){
      printf("%s\tPID: %d\tTime: %f\n",cmd[i]->command,  cmd[i]->pid, (double)(cmd[i]->en_time - cmd[i]->st_time));
    }
  }

  return 0;
}

