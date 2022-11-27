#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include<readline/readline.h>
#include<readline/history.h>
// #define MAX_PATH_LEN 255
#define CMDLINE_LEN 512
#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
#define FALSE !TRUE
#define TRUE 1


int take_user_input(char *str) //this function will allow the user to input their response
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


 void parseSpace(char* string, char** parsed)	//this function will parce the command words
 {
 	for (int i = 0; i < MAXLIST; i++)
	{
 		parsed[i] = strsep(&string, " ");
 		if (parsed[i] == NULL)
 			break;
 		if (strlen(parsed[i]) == 0)
 			i--;
 	}
 }

 int cmdhandeler(char** parsed)
 {
 	int cmds = 2, switchArg = 0; //for this finctionlity we only need cd commands
 	char* allcmds[cmds];//gets all the commands
 	char* username;


 	allcmds[0] = "cd";	//this allows the terminal to recognize cd
	allcmds[1] = "ls";

 	for (int i = 0; i < cmds; i++) {
 		if (strcmp(parsed[0], allcmds[i]) == 0)  //coppies strings of the parsed arguments with all the commnds and if equal 0 switches argumenst + 1
		 {
 			switchArg = i + 1;
 			break;
 	 	 }
 	}

 	switch (switchArg)
	{
 	case 1:	//jumps of the case 1 change directoy if true and value of 1 being given
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
	 } else {
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

 	if (cmdhandeler(parsed))
 	 {
 		return 0;
	 }
 	else
	 {
 		return 1 + piped;
	 }
 }

 int main()
 {
 	char userInput[MAXCOM], *parsedArgs[MAXLIST];
 	char* parsedArgsPiped[MAXLIST];
 	int execFlag = 0;

 	while (1)
	{
 		print_current_directory(); //prints the current shell line
 		// takes input
 		if (take_user_input(userInput))
 			continue; //process

 		execFlag = processString(userInput, parsedArgs);	// execflag returns zero if there is no command
 	 			//returns 1 if it is a simple command
 		// or it is a builtin command,
 		// 1 if it is a simple command

 		// execute
 		if (execFlag == 1)
		 {
 			execArgs(parsedArgs);
		 }

 	}
 	return 0;
 }
