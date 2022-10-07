#include <stdio.h>
#include <unistd.h> //fork
#include <sys/wait.h> //wait
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h> // for history
#include <string.h>
#include <sys/types.h> // for open file
#include <sys/stat.h> // for open file
#include <fcntl.h> // for open file


int input_row = 0;

/* cut the input string */
char **get_input(char *input)
{
	char **command = malloc(20*sizeof(char*));
	if (command == NULL)
	{
		printf("malloc failed");
		exit(-1);
	}
	char *separator = " "; //cut the string by the blank
	char *parsed;
	int index = 0;

	parsed = strtok(input,separator);
	while(parsed != NULL) // still can be cut
	{
		command[index++] = parsed;
		parsed = strtok(NULL,separator);
	}
	command[index] = NULL; // the last one is NULL
	input_row = index;
	return command;
}

int main(int argc, char **argv)
{
	char *input; //get the input
	char **command; //get the command (already be cut)
	char buf[100]; //to print pwd or to change PATH

	pid_t pid; // for external commands

	while(1)
	{
		input = readline("promp$ "); //read a line from the terminal

		if (strlen(input)) // input != 0
		{
			add_history(input);
		}

		command = get_input(input);

		/* for bg exec */
		int bgflag = 0;
		if (!strcmp(command[input_row-1],"&"))
		{
			bgflag = 1;
			command[input_row-1] = NULL;
			input_row--;
		}

		/* parsed the command and work */
		if (strcmp(command[0],"cd") == 0) //command cd
		{

			if (chdir(command[1]) < 0) //error
			{
				printf("cd error\n");
			}
		}
		else if (strcmp(command[0],"pwd") == 0) //command pwd
		{
			/* output file */
			int flag = 0, index = 0;
			FILE *fp;
			for (index = 0; index < input_row; index++)
			{
				if (!strcmp(command[index],">"))
				{
					flag = 1; //have output file
					break;
				}
			}
			if (flag == 1)
			{
				fp = fopen(command[index+1],"w");
				fprintf(fp,"%s\n",getcwd(buf,sizeof(buf)));
				fclose(fp);
			}
			else
			{
				/* work */
				printf("%s\n",getcwd(buf,sizeof(buf)));
			}
			strcpy(buf,"");
		}
		else if (strcmp(command[0],"export") == 0) //command export
		{
			//export PATH=$PATH:home:/123
			strncpy(buf,command[1]+10,strlen(command[1])-10);//get input value of PATH
			setenv("PATH",buf,1); //set value
			printf("PATH: %s\n",getenv("PATH"));
			strcpy(buf,"");
			
		}
		else if (strcmp(command[0],"echo") == 0) //command echo
		{
			/* output file */
			int flag = 0, index = 0;
			FILE *fp;
			for (index = 0; index < input_row; index++)
			{
				if (!strcmp(command[index],">"))
				{
					flag = 1; //have output file
					break;
				}
			}
			if (flag == 1)
			{
				fp = fopen(command[index+1],"w");
				for (int i = 1; i < index; i++)
				{
					fprintf(fp,"%s ",command[i]);
				}
				fprintf(fp,"\n");
				fclose(fp);
			}
			else
			{

				for (int i = 1; command[i] != NULL; i++)
				{
					printf("%s ",command[i]);
				}
				printf("\n");
			 }
		}
		else if (!strcmp(command[0],"exit"))
		{
			return 0;
		}
		else //external command
		{
			pid = fork();
			if (pid < 0)
			{
				printf("fork error\n");
				return 0;
			}
			else if (pid == 0) //child process
			{
				int flag = 0, index = 0,fd;
				for (index = 0; index < input_row; index++)
				{
					if (!strcmp(command[index],">"))
					{
						flag = 1; //have output file
						break;
					}
				}
				if (flag == 1)
				{
					fd = open(command[index+1], O_WRONLY|O_CREAT|O_TRUNC,777);
					dup2(fd,STDOUT_FILENO); //change output
					for (; index < input_row; index++)
					{
						command[index] = NULL;
					}

				}
				/* external command */
				if (execvp(command[0],command) < 0) //error
				{
					printf("external command error\n");
					return 0;
				}
				if (fd != STDOUT_FILENO) // change to standard output
				{
					close(fd);
				}

			}
			else //parent process (shell)
			{
				if (!bgflag) // not backgroung execution
				{
					wait(NULL);
				}  
			}

		}

	}	
	return 0;
}
