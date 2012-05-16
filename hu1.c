/*--- hu1.c ----------------------------------------------------------------*/
/* Hermann Sutter, Florian Thomas */

#include <eos32sys.h>
#include <eos32lib.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include "hu1.h"

#define MAX_LINE     500
#define MAX_COMMANDS  10
#define MAX_ARGUMENTS 19

#define FALSE 0
#define TRUE 1


/* count the finished Programs */
int finishedPrograms = 0;




/* signal handler */
void signal_callback_handler(int signum){ 
}


int main(void){
  char inputStr[MAX_LINE];
  char *token = NULL;       /* read tokens from stream and command */
  char *tmpArg;             /* command could not overwrite by array allocate */
  char *ptr;                /* remove \n from string */
  Commands **cmd;
  struct tbuffer st_cpu;    /* start time */
  struct tbuffer en_cpu;    /* end time */
  int cntCom, cntArg, i, j; /* counter variables */
  int childPid, pid, errno, status, exitValue;

  /* SIGINT init, need for interrupt programm */
  signal(SIGINT, signal_callback_handler);

  while(TRUE){
    /* reset cmd and finishedPrograms counter */
    cmd = NULL;
    finishedPrograms = 0;

    /* read input line, string must be 501 because of last
       null or \n entry, not sure about this point */
    printf("> ");
    if(fgets(inputStr, MAX_LINE+2, stdin) == NULL){
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
      /* check if token is empty */
      token = trimwhitespace(token);
      if(strlen(token) == 0){
        token = strtok(NULL, ";");
        continue;
      }
      token = strtok(NULL, ";");
      cntCom++;
      
    }

    /* if no valid command was entered continue at the next prompt */
    if(cntCom == 0){
      continue;
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
      /* set exitStatus initially to "not finished correctly" */
      cmd[i]->exitStatus = -1;
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
      if( (ptr = strchr(token, '\n')) != NULL){
        *ptr = '\0';
      }
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

    /* fork each process */
    for(i = 0; i < cntCom; i++){
      childPid = fork();

      if(childPid < 0){
        finishedPrograms = finishedPrograms + 1;
        printf("Fork of %s failed", cmd[i]->command);
      }else if(childPid == 0){
        /* exitValue is not 0 if the command can not be executed */
        exitValue = execvp(cmd[i]->command, cmd[i]->args);
        exit(exitValue);
      }else{ /* Parent process */
        cmd[i]->pid = childPid;
      }
    }
    
    /* wait for all children */
    while(TRUE){
      times(&st_cpu);
      pid = wait(&status);
      times(&en_cpu);
      for(i = 0; i < cntCom; i++){
        if(cmd[i]->pid == (int)pid){
          cmd[i]->exitStatus = 0;
          cmd[i]->passedTime = en_cpu.child_user_time - st_cpu.child_user_time;
          break;
        }
      }
      finishedPrograms = finishedPrograms + 1;
      if(finishedPrograms >= cntCom){
        break;
      }
      
    }


    /* only for good look */
    printf("\n");
    

    /* comes to this point by an interrupt with CTRL C */
    if(errno == EINTR){
      /* here are the non terminated processes */
      while((pid = wait(&status)) != -1){
        finishedPrograms = finishedPrograms + 1;
        for(i = 0; i < cntCom; i++){
          if(pid == cmd[i]->pid){
            cmd[i]->exitStatus = -1;
            break;
          }
        } 
      }

      printSummary(cmd, cntCom);
      exit(0);
    }
    

    printSummary(cmd, cntCom);

  }
  return 0; 
}

/* prints the summary */
void printSummary(Commands **cmd, int numberOfCommands){
  int summe, i;
  /* only for good look */
  printf("\n\n");

  summe = 0;
  for(i = 0; i < numberOfCommands; i++){
    if (cmd[i]->exitStatus != 0){
      printf("%s: [execution error]\n", cmd[i]->command);
    }else{
      /*sysconf(_SC_CLK_TCK);*/
      printf("%s: user time = %d\n", cmd[i]->command, (int)(cmd[i]->passedTime));
      summe += cmd[i]->passedTime;
    }
  }
  summe = (int)(summe);
  printf("sum of user times = %d\n", summe);
}

/* deletes whitespace before and after a given string */
/* this function was taken from 
http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way */
char *trimwhitespace(char *str){
  char *end;

  /* Trim leading space */
  while(isspace(*str)) str++;

  if(*str == 0)  /* All spaces? */
    return str;

  /* Trim trailing space */
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  /* Write new null terminator */
  *(end+1) = 0;

  return str;

}


