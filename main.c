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
#define MAX_ARGUMENTS 19


/* struct to save command and aruments */
typedef struct commands{
  int pid;
  char *command;
  char **args;
  clock_t st_time;
  clock_t en_time;
  struct tms st_cpu;
  struct tms en_cpu;
}Commands;


int main(void){
  char inputStr[MAX_LINE];
  char *token = NULL;
  char *tmpArg;
  Commands **cmd;
  int cntCom, cntArg, i, j, childPid, pid, errno, status, exitValue;
  int pipefd[2];
  pipe(pipefd);

  while(1){
    /* set cmd to NULL for next loop pass */
    cmd = NULL;
    /* read input line, string must be 501 because of last
       null or \n entry, not sure about this point */
    printf("> ");
    if(fgets(inputStr, MAX_LINE+1, stdin) == NULL){
      perror("error: input stream\n");
      return 1;
    }
    if(strlen(inputStr) > MAX_LINE+1){
      perror("error: input string to long\n");
      return 1;
    }

    /* create command array */
    tmpArg = (char *)malloc(sizeof(char) * strlen(inputStr));
    strcpy(tmpArg, inputStr);
    token = strtok(tmpArg, ";");
    cntCom = 0;
    while(token != NULL){
      cntCom++;
      token = strtok(NULL, ";");
    }
    cmd = (Commands **)malloc(sizeof(Commands *));
    if(cmd == NULL){
      perror("error: out of memory\n");
      return 1;
    }
    for(i=0; i<cntCom; i++){
      cmd[i] = (Commands *)malloc(sizeof(Commands));
      if(cmd[i] == NULL){
        perror("error: out of memory\n");
        return 1;
      }
    }

    /* check for max commands, max commands are 10 */
    if(cntCom > MAX_COMMANDS+1){
      perror("error: to much commands\n");
      return 1;
    }

    /* split string in tokens and save in struct */
    strcpy(tmpArg, inputStr);
    token = strtok(inputStr, ";");
    for(i=0; i<cntCom; i++){
      cmd[i]->command = token;
      token = strtok(NULL, ";");
    }

    /* saves arguments in char array for each command */
    for(i=0; i<cntCom; i++){
      /* command can't be over written */
      tmpArg = (char *) malloc(sizeof(char) * strlen(cmd[i]->command));
      strcpy(tmpArg, cmd[i]->command);
      /* count arguments */
      cntArg = 0;
      token = strtok(tmpArg, " ");
      while(token != NULL){
        cntArg++;
        token = strtok(NULL, " ");
      }
      if((cntArg-1) > 20){
        perror("error: to much arguments\n");
        return 1;
      }
      /* create arguments array */
      cmd[i]->args = (char **)malloc(sizeof(char *));
      if(cmd[i]->args == NULL){
        perror("error: out of memory\n");
        return 1;
      }
      for(j=0; j<cntArg+1; j++){
        cmd[i]->args[j] = (char *)malloc(sizeof(char));
        if(cmd[i]->args == NULL){
          perror("error: out of memory\n");
          return 1;
        }
      }

      strcpy(tmpArg, cmd[i]->command);
      token = strtok(tmpArg, " ");
      cmd[i]->command = token;
      /* first element MUST be the command */
      for(j=0; j<cntArg; j++){
        cmd[i]->args[j] = token;
        token = strtok(NULL, " ");
      }
      /* last element MUST be a NULL */
      cmd[i]->args[cntArg] = NULL;

      if(strlen(cmd[i]->command) > 21){
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
        /*close(pipefd[0]);*/    /* close reading end in the child */
        /*dup2(pipefd[1], 1);*/  /* send stdout to the pipe */
        /*close(pipefd[1]);*/
        /* begin time measure */
        /*cmd[i]->st_time = times(&cmd[i]->st_cpu);*/
        /* exitValue is not 0 if the command can not be executed */
        exitValue = execvp(cmd[i]->command, cmd[i]->args);
        exit(exitValue);
      }else{ /* Parent process */
        cmd[i]->pid = childPid;
        /*printf("Process pid %d\n", getpid());
        printf("Process pid %d\n", cmd[i]->pid);*/
      }
    }
    
    /* wait for all children */
    while((pid = waitpid(-1, &status,0))){
      /*for(i=0; i<cntCom; i++){*/
        /* end time measure */
        /*if(cmd[i]->pid == (int)pid){
          cmd[i]->en_time = times(&cmd[i]->en_cpu);
printf("start: %d\t", (int)(cmd[i]->st_cpu.tms_utime));
printf("end: %d\n", (int)(cmd[i]->en_cpu.tms_utime));
          break;
        }
      }*/
      if(errno == ECHILD){
        break;
      }
      /*printf("Exit status of %d was %d \n", (int)pid, WEXITSTATUS(status));*/
    }

    for(i = 0; i < cntCom; i++){
      /*printf("%s\tPID:%d\tTime:%d\n",cmd[i]->command,  cmd[i]->pid, (int)(cmd[i]->en_cpu.tms_cutime - cmd[i]->st_cpu.tms_cutime));*/
    }
  }

  return 0;
}

