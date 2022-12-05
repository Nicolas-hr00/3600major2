 #include <regex.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include<sys/types.h>
#include<sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include<readline/readline.h>
#include<readline/history.h>
#include <string.h>


// #define MAX_PATH_LEN 255
#define CMDLINE_LEN 512
#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
#define FALSE !TRUE
#define TRUE 1


int take_user_input(char *str) //this function will allow the user to input their response. This is the functiona part of the project
 {
	 char* buffer;	//creating the buffer
	 buffer = readline(""); //this takes the user input
	 if (strlen(buffer) != 0)
	  {
			strcpy(str, buffer);//copies the string into the buffer
			return 0;
		}
 		else
		 {
			 return 1;
		 }
 }
void SigHandler(int sig_num)
 {
   signal(SIGINT, SigHandler);  //for control+c
   signal(SIGTSTP, SigHandler); //control +z
   printf("You cannot terminate using Ctrl+c or Ctrl+z\n");
   fflush(stdout);
 }


 void control(char *cmd)  //Sighandler and control
  {
    signal(SIGINT, SigHandler); //sets the siginit to sig handerler for control ++c
    signal(SIGTSTP, SigHandler);  //same thing for control
  }
 void print_current_directory()  //this prints the current directory
  {
    char buffer[1024];
    getcwd(buffer, sizeof(buffer));

		 if (getcwd(buffer,sizeof(buffer)) == NULL)
		 	 {
				 perror("getcwd() error");
				 return 1;
			 }
			else
			{
				printf("\n\n%s:",buffer);
			}
  }
void execargpiped(char** parsed, char** parsedpipe)
 {
   int pipefd[2];
   pid_t p1,p2;

   if (pipe(pipefd) < 0 ) //the pipe won't be able to initialize if the pipe is less than 0
    {
        printf("\n Pipecan't be initialized");
        return 0;
    }
    if (p1==0 ) //piepe rading
     {
       close (pipefd[0]); //needs to be closed first
       dup2(pipefd[1], STDOUT_FILENO);
       close(pipefd[1]);
     }
     if(execvp(parsed[0], parsed) < 0)
      {
        printf("\nCould't execute command 1");
        exit(0);
      }
        //parent executing
      else
       {
         p2 =fork();

         if(p2<0)
          {
            printf("\nProblem at forking");
            return 0;
          }
       }
      if (p2 == 0)
       {
          close(pipefd[1]);
          dup2(pipefd[0], STDIN_FILENO);
          close(pipefd[0]);
            if ( execvp(parsedpipe[0],parsedpipe) < 0)
             {
               printf("\Could not execute command 2"); //in case it fails it exits ;
               exit(0);
             }
       }
        else
         {
           wait(NULL);
         }
 }

 void parseSpace(char* string, char** parsed)	//this function will parce the command words
 {  //it also allows the shell to not have a core dumped
 	for (int i = 0; i < MAXLIST; i++)
	{
 		parsed[i] = strsep(&string, " ");
 		if (parsed[i] == NULL)
 			break;
 		if (strlen(parsed[i]) == 0)
 			i--;
 	}
 }

 int cd(char** parsed) //allow the user to access onto the shell comands like cd, ls, exit,
 {
 	int cmds = 3, switchArg = 0; //for this finctionlity we only need cd commands
 	char* allcmds[cmds];//gets all the commands
 	char* username;

  allcmds[0] = "exit";
 	allcmds[1] = "cd";	//this allows the terminal to recognize cd
	// allcmds[1] = "ls";

 	for (int i = 0; i < cmds; i++) {
 		if (strcmp(parsed[0], allcmds[i]) == 0)  //coppies strings of the parsed arguments with all the commnds and if equal 0 switches argumenst + 1
		 {
 			switchArg = i + 1;
 			break;
 	 	 }
 	}
 	switch (switchArg)
	{	//jumps of the case 1 change directoy if true and value of 1 being given
  case 1:	//jumps of the case 1 change directoy if true and value of 1 being given
	 {
 		// chdir(parsed[2]);
    exit(0);
 		return 0;
	}
  case 2:	//jumps of the case 2 change directoy if true and value of 1 being given
   {
	    chdir(parsed[1]);
    return 1;
  }
 	default:
 		break;
 	}

 	return 0;
 }
 void execArgs(char** parsed) //the printing files will be passes here and will print all the current files of the directory
 {
	 pid_t pid = fork();

	 if (pid == -1)  //this is the child
	 {
		 printf("\nFailed forking child..");
		 return;
	 }
	 else if (pid == 0) //this is the parent process
	 {
		 if (execvp(parsed[0], parsed) < 0) {
			 printf("\nCould not execute command..");
		 }

		 exit(0);
	 }
      else {
		 // waiting for child to terminate
		 wait(NULL);
		 return;
	 }
 }


 int processString(char* str, char** parsed)
 {
 	char* strpiped[2];
 	int piped = 0;

 	// piped = parsePipe(str, strpiped);
 	if (piped)
	{
 		parseSpace(strpiped[0], parsed);
 		parseSpace(strpiped[1], parseSpace);
 	}
	else
	{
 		parseSpace(str, parsed);
 	}

 	if (cd(parsed))
 	 {
 		return 0;
	 }
 	else
	 {
 		return 1 + piped;
	 }
 }
 void command(char* commands, bool isBatch)
 {
  //  char* tok; //Tokens
  //  regex_t regex; //Regex
  //
 	// regex_t io_regex;								// regex match for char < or > in command
 	// char *io_compare = "[><]";
  //
 	// regex_t exit_regex;							// regex to match for exit in command
 	// char *exit_compare = "exit";
  //
  // regex_t re_regex;							// regex to match for redirect command
 	// char* re_compare = "|";
  //
  //  //If in batch mode read out the commnads
  //  if(isBatch == true)
  //  printf("Executing command: %s\n",commands);
  //
  //  //Run through the commands
  //  tok = strtok(commands, ";");//grab first command
  //  while(tok != NULL)
  //  {
 	// 	//
 	// 	//	if users command conatins '>' or '<' call my_io(command) to handle io redirection
 	// 	//
 	// 	regcomp(&io_regex, io_compare, 0);	// compile regex to comare word to ">" or "<"
  //    regcomp(&exit_regex, exit_compare, 0);	// compile regex to comare word to "exit"
  //    regcomp(&regex, "path", 0);//Compile Regex
  //    regcomp(&io_regex, io_compare, 0);	// compile regex to comare word to ">" or "<"
  //    regcomp(&re_regex, re_compare, 0);	// compile regex to comare word to "|"
  //
 	// 	if((regexec(&io_regex, tok,0,NULL,0)) == 0){
 	// 		my_io(tok);		// if users input contains < or >
 	// 	}
  //
 	// 	//
 	// 	// if users command contains "exit" call my_exit(command) to handle exiting program
 	// 	//
 	// 	else if((regexec(&exit_regex, tok,0,NULL,0)) == 0){
 	// 		my_exit(tok);		// if users input contains exit
 	// 	}
  //
  //    //Call Path Command
  //    else if((regexec(&regex, tok,0,NULL,0)) == 0)//Compare Regex to word 'PATH'
  //    {
  //      path(tok);
  //    }
  //
  //
 	// 		//
 	// 		//	if users command conatins '>' or '<' call my_io(command) to handle io redirection
  //      else if((regexec(&io_regex, tok,0,NULL,0)) == 0){
 	// 		//
 	// 			my_io(tok);		// if users input contains < or >
 	// 		}
  //
 	// 		//
 	// 		// if users command contains "exit" call my_exit(command) to handle exiting program
 	// 		//
 	// 		else if((regexec(&exit_regex, tok,0,NULL,0)) == 0){
 	// 			my_exit(tok);		// if users input contains exit
 	// 		}
  //
  //      else if((regexec(&re_regex, tok,0,NULL,0)) == 0)
  //      {
  //        tok = strtok(NULL, ";"); //Call next command
  //        continue;
  //        //printf("Found pipe in: %s\n", tok);
  //        //pipef(tok);
  //      }
  //
  //      else //If command does not exsist
  //        printf("Command \"%s\" does not exsist\n", tok);
  //        //printf("Executing command: %s\n",commands);
  //
  //    tok = strtok(NULL, ";"); //Call next command
  //  }
 }

 int main()
 {
 	char userInput[MAXCOM], *parsedArgs[MAXLIST];
 	char* parsedArgsPiped[MAXLIST];
 	int execFlag = 0;
  char* file;
  char input[512];
  char comands[515];


 	while (1)
	{
    file = strtok(input," ");
 		print_current_directory(); //prints the current shell line
 		// takes input
 		if (take_user_input(userInput))
 			continue; //process

 		execFlag = processString(userInput, parsedArgs);	// execflag returns zero if there is no command
   // if(take_user_input(userInput) == ' ' ||take_user_input(userInput) == ';')
   //   {
   //     printf("Argument valid continue:");
   //   }

 		// execute
 		if (execFlag == 1)
		 {
 			execArgs(parsedArgs);
		 }
     if(execFlag == 2)
      {
        print_current_directory(); //prints the current shell line
        execargpiped(parsedArgs, parsedArgsPiped);
         // takes input
        execFlag = processString(userInput, parsedArgs);
      }
 	}
 	return 0;
 }
