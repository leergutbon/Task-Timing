typedef struct commands{
  int pid;
  char *command;
  char **args;
  int passedTime;
  int exitStatus;
}Commands;
void signal_callback_handler(int signum);
int main(void);
char *trimwhitespace(char *str);
void printSummary(Commands **cmd, int numberOfCommands);
