 #include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#define clear() printf("\033[H\033[J")

void parentHandler();

void setParentSignals();

void init_shell()
{
	clear();
	char* username = getenv("USER");
	printf("\n\n\nUSER is: @%s", username);
	printf("\n");
	sleep(1);
	clear();
}


#define MAX 512 //user's input is less than 512 bytes
int hist_count = 0; //global variable for MyHistory function
int alias_count = 0;  //global variable for MyAlias function
pid_t ppid; //gloabal parent id
pid_t cpid; //global child id

//============================================================
typedef struct History{		//typedef struct history
	struct History *next;
	char *com;
}History;

typedef struct List{	//typedef struct List
    	History *top;
	unsigned int count;
}List;

bool isEmpty(List *list){
if(list->count == 0)
  return true;	//return true if empty, else false
else
      return false;
}
//====================================================
void InteractiveMode();
void BatchMode(char *file);

int ParseCommands(char *userInput); //e.g., "ls -a -l; who; date;" is converted to "ls -al" "who" "date"
int ParseArgs(char *full_line, char *args[]); //e.g., "ls -a -l" is converted to "ls" "-a" "-l"
void ExecuteCommands(char *command, char *full_line);

void MyCD(char *dir_input, int arg_count);
void MyExit(char *str);
void MyPath(char *args[], int arg_count);
void MyHistory(char *args[], int arg_count);

void CommandRedirect(char *args[], char *first_command, int arg_count, char *full_line);
void PipeCommands(char *args[], char *first_command, int arg_count);
void signalHandle();
void MyAlias(char *args[], int arg_count);
void io_redirect(char *command, char *full_line);

int ParseArgs(char *full_line, char *args[]); //e.g., "ls -a -l" is converted to "ls" "-a" "-l"
void ExecuteCommands(char *command, char *full_line);



char CURRENT_DIRECTORY[MAX]; //current directory
char *COMMANDS[MAX]; //commands to be executed
char *MYHISTORY[MAX]; //shell command history
char *MYPATH; //my PATH variable
char *MYALIAS[MAX]; //alias variable
const char *ORIG_PATH_VAR; //The original PATH contents
char *prompt;

int exitCmd = 0;//Functions seem to treat this as a global variable -DM


int main(int argc, char *argv[]){
  // signalHandle();

  init_shell();
    //error checking on user's input
         if (!(argc < 3)) {
                 fprintf(stderr, "Error: Too many parameters\n");
                 fprintf(stderr, "Usage: './output [filepath]'\n");
                 exit(0);//No memory needs to be cleared
         }
         signalHandle();//no sirve


   //initialize your shell's enviroment
   MYPATH = (char*) malloc(1024);
         memset(MYPATH, '\0', sizeof(MYPATH));
         ORIG_PATH_VAR = getenv("PATH"); // needs to include <stdlib.h>
         // signalHandle(); //el mejor hasta ahora

   //save the original PATH, which is recovered on exit
         strcpy(MYPATH, ORIG_PATH_VAR);
   //make my own PATH, namely MYPATH
         setenv("MYPATH", MYPATH, 1);
         // signalHandle(); no sirve
         if(argc == 1) InteractiveMode();

     else if(argc == 2) BatchMode(argv[1]);
               //gets the parent id and sets it to ppid
   ppid = getpid();

   //handles the signal (Ctrl + C)
   signal(SIGINT, signalHandle);
     //handles the signal (Ctrl + Z)
   signal(SIGTSTP, signalHandle);

   //free all variables initialized by malloc()
         free(MYPATH);

       return 0;
}

void BatchMode(char *file){

       FILE *filePtr = fopen(file, "r");
   if(filePtr == 0) {
               fprintf(stderr, "BATCH FILE UNAVAILABLE!\n");
               MyExit(0);
   }

   char *batchCmd = (char *)malloc(MAX);
   memset(batchCmd, '\0', sizeof(batchCmd));


   //reads and stores command
   while(fgets(batchCmd, MAX, filePtr)){
       batchCmd[strcspn(batchCmd, "\n")] = 0;
       printf("Reading input: %s\n", batchCmd);

       
       int commandCount = ParseCommands(batchCmd);//parses batchCmd

      
       for(int i=0; i< commandCount; i++){
           char *tempVal = strdup(COMMANDS[i]); 
           tempVal = strtok(tempVal, " "); //retrieves commands to execute
           ExecuteCommands(tempVal, COMMANDS[i]);
                       free(tempVal);//frees tempVal
       }
   }
       free(batchCmd);
       fclose(filePtr);
}

int ParseCommands(char *str){

       int i = 0;

       char *token = strtok(str, ";"); //breaks str into a series of tokens using ;

       while(token != NULL){
               //error checking for possible bad user inputs
               //Removes Spaces at beginning
               while (token[0] == ' ') {
                       int size = strlen(token);
                       for (int j=0; j<size; j++) {
                               token[j] = token[j+1];
                       }
               }

               //If after, removing all whitespaces we're left with a NULL char,
               //then the command is empty and will be ignored
               if (token[0] == '\0') {
                       token = strtok(NULL, ";");
                       continue;
               }

               //Removes all but one whitespace in between args
               for (int j=0; j<strlen(token); j++) {
                       //fprintf(stderr,"Token Edit: %s\n", token);
                       if (token[j] == ' ' && token[j+1] == ' ') {
                               int size = strlen(token);
                               for (int k=j; k<size; k++)
                                       token[k] = token[k+1];
                               j--;
                       }
               }

       //save the current token into COMMANDS[]
       COMMANDS[i] = token;
       i++;
       //move to the next token
       token = strtok(NULL, ";");
       }

       return i;
}

void ExecuteCommands(char *command, char *full_line){

       char *args[MAX]; //hold arguments

       MYHISTORY[hist_count%20] = strdup(full_line); //array of commands
       hist_count++;

   //save backup full_line
   char *backup_line = strdup(full_line);

   if (strcmp(command, "alias") == 0 && strchr(full_line, '=') != NULL) {
               //break full_line into a series of tokens by the delimiter space (or " ")
               char *token = strchr(full_line, ' ');
               while (token[0] == ' ') {
                       int size = strlen(token);
                       for (int j=0; j<size; j++) {
                               token[j] = token[j+1];
                       }
               }
               MYALIAS[alias_count] = strdup(token);
               alias_count++;
       }
       else {
               //parse full_line to get arguments and save them to args[] array
               int arg_count = ParseArgs(full_line, args);

               //restores full_line
       strcpy(full_line, backup_line);
       free(backup_line);

               //check if built-in function is called
               if(strcmp(command, "cd") == 0)
                       MyCD(args[0], arg_count);
               else if(strcmp(command, "exit") == 0)
                       MyExit(command);
               else if(strcmp(command, "path") == 0)
                       MyPath(args, arg_count);
               else if(strcmp(command, "myhistory") == 0)
                       MyHistory(args, arg_count);
               else if(strcmp(command, "alias") == 0){}
                       //MyAlias(args, arg_count);
               else
                       CommandRedirect(args, command, arg_count, full_line);
               //free memory used in ParsedArgs() function
               for(int i=0; i<arg_count-1; i++){
                       if(args[i] != NULL){
                               free(args[i]);
                               args[i] = NULL;
                       }
               }
       }
}

int ParseArgs(char *full_line, char *args[]){
       int count = 0;

   //break full_line into a series of tokens by the delimiter space (or " ")
       char *token = strtok(full_line, " ");
       //skip over to the first argument
       token = strtok(NULL, " ");

   while(token != NULL){
       //copy the current argument to args[] array
       args[count] = strdup(token);
       count++;
       //move to the next token (or argument)
       token = strtok(NULL, " ");
   }

   return count + 1;
}

void CommandRedirect(char *args[], char *first_command, int arg_count, char *full_line){
       pid_t pid;
       int status;

       //if full_line contains pipelining and redirection, error displayed
       if (strchr(full_line, '|') != NULL && (strchr(full_line, '<') != NULL || strchr(full_line, '>') != NULL)) {

           fprintf(stderr,"Command cannot contain both pipelining and redirection\n");
       }
       //if full_line contains "<" or ">", then io_redirect() is called
       else if (strchr(full_line, '<') != NULL || strchr(full_line, '>') != NULL) {
               io_redirect(first_command, full_line);
       }
       //if full_line contains "|", then PipeCommands() is called
       else if (strchr(full_line, '|') != NULL) {
               PipeCommands(args, first_command, arg_count);
       }
       else {//else excute the current command
               //set the new cmd[] array so that cmd[0] hold the actual command
               //cmd[1] - cmd[arg_count] hold the actual arguments
               //cmd[arg_count+1] hold the "NULL"
               char *cmd[arg_count + 1];
               cmd[0] = first_command;
               for (int i=1; i<arg_count; i++)
                       cmd[i] = args[i-1];
               cmd[arg_count] = '\0';

               pid = fork();
               if(pid == 0) {
                       execvp(*cmd, cmd);
                       fprintf(stderr,"%s: command not found\n", *cmd);
                       MyExit(0);//Ensures child exits after executing command
               }
               else wait(&status);
       }
}

void InteractiveMode(){

       int status = 0;

   //get custom prompt
   prompt = (char*)malloc(MAX);
   printf("Enter custom prompt: ");
   fgets(prompt, MAX, stdin);

   //remove newline from prompt
   if (prompt[strlen(prompt)-1] == '\n') {
       prompt[strlen(prompt)-1] = '\0';
   }

       while(exitCmd = 0){
               char *str = (char*)malloc(MAX);

               printf("%s> ", prompt);
               fgets(str, MAX, stdin);

               //error checking for empty commandline
               if (strlen(str) == 1) {
                       continue;
               }

               //remove newline from str
               if (str[strlen(str)-1] == '\n') {
                       str[strlen(str)-1] = '\0';
               }

               //parse commands
               int cmd_num = ParseCommands(str);//this function can be better designed

               //execute commands that are saved in COMMANDS[] array
               for(int i=0; i < cmd_num; i++){
                       char *temp = strdup(COMMANDS[i]);
                       temp = strtok(temp, " ");
                       ExecuteCommands(temp, COMMANDS[i]);
                       //free temp
                       free(temp);
               }

               //ctrl-d kill

               free(str);

               // if exit was selected
               if(exitCmd) {
                   free(prompt);
                   MyExit(0);
               }
       }
}

void MyCD(char *dir_input, int arg_count){
  char currentDirectory[256];
  char tempcurrentdirectory[255];
  char HomeDirectory[255];

  getcwd(currentDirectory, sizeof(currentDirectory));

  printf("Current directory: %s\n", currentDirectory);
  if (chdir(dir_input) == -1) //change the current working directory
   {
     // if (dir_input == "cd")
     //  {
     //    strcpy(arg_count,"\\home\\");
     //    strcat(arg_count, tempcurrentdirectory);
     //    printf("%s",tempcurrentdirectory);
     //    chdir(tempcurrentdirectory);
     //    printf("New workking directory: %s\n", getcwd(tempcurrentdirectory,sizeof(tempcurrentdirectory)));
     //
     //  }
     perror("chdir");//prints error if it occurs
     return;
   }
   printf("New workking directory: %s\n", getcwd(tempcurrentdirectory,sizeof(tempcurrentdirectory)));

   return;
}

void MyExit(char *str){
    ParseCommands(str);
    int cmd_num = ParseCommands(str);
    if (cmd_num <=1){
        return;
    }
    else{
        for(int i = 0; i <= cmd_num; i++){
            char *tempVal = strdup(COMMANDS[i]); 
           tempVal = strtok(tempVal, " "); //retrieves commands to execute
           ExecuteCommands(tempVal, COMMANDS[i]);
        }
    }
	exitCmd = 1;
	exit(EXIT_SUCCESS);
	return;
}

void MyPath(char *args[], int arg_count){


}

void MyHistory(char *args[], int arg_count){
}

void PipeCommands(char *args[], char *first_command, int arg_count){


  for (int i =0; i < arg_count; i++)
   {
     first_command[i] = strsep(args, "|");

     if (first_command[i] == NULL)
      {
        break;
      }
   }
   if (first_command[1] == NULL)

      return 0;
   else
       {
         return 1;
       }



  int pipefd[2];
  pid_t p1,p2;

   if (pipe(pipefd) < 0 )
    {
    printf("\nPipe could not be initialized");
    return;
    }
   p1 = fork();
   if ( p1 < 0)
     {
       printf("\nCould not fork" );
       return;
     }
     if (p1==0)
      {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close (pipefd[1]);

        if (execvp(first_command[0], first_command) < 0 )
         {
           printf("\nCould not execute command");
           exit(0);
         }
      }
      else
       {
         p2 = fork();

         if (p2< 0)
          {
            printf("\Could not fork");
            return;
          }

      if ( p2==0)
       {
         close(pipefd[1]);
         dup2(pipefd[0], STDIN_FILENO);
         close (pipefd[0]);
         if (execvp(first_command[0], arg_count)< 0)
           {
             printf("\nCould not execute command 2..");
             exit(0);
           }
       }
       else
        {
          wait(NULL);
          wait(NULL);
        }
      }

 // return 0;
}


void parentHandler()
{
    printf("\n");
}
void signalHandle(){

  // signal(SIGTTOU, SIG_IGN); se pude borrar
  // signal(SIGHUP, parentHandler);
  signal(SIGINT, parentHandler);
  signal(SIGQUIT, parentHandler);
  // signal(SIGTSTP, parentHandler); //works for control z
  signal(SIGKILL, parentHandler);
  signal(SIGQUIT, parentHandler);
  signal(SIGCONT, parentHandler);
  signal(SIGPIPE, parentHandler);
  signal(SIGTERM, parentHandler);
}


void io_redirect(char *command, char* full_line) {

}
