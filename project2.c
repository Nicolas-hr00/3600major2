#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#define MAX_PATH_LEN 255
#define CMDLINE_LEN 512
#define FALSE !TRUE
#define TRUE 1
#define clear() printf("\033[H\033[J") //\033 - ASCII escape character
//[H - move the cursor to the home position
//[J - erases the screen from the current line down to the bottom of the screen

void init_shell()
{
	clear();
	char* username = getenv("USER"); //find the environment list to find the variable name
	printf("USER is: @%s", username); //searches and finds the current user
	printf("\n");
	sleep(1);
	clear();
}
 void printDirectory()  //this prints the current directory
  {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\n%s:", cwd);	//This prints the path of the user 
  }

int main(int argc, char **argv, char **envp)
{
    int closeShell = FALSE;
    char cmdline[CMDLINE_LEN];
    init_shell();
    while (1)
     {
       printDirectory();
       gets(cmdline);
     }

return 0;
}
