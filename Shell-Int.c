//libraries needed for input, processing, storing, forking, separating words.
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<readline/readline.h>
#include<readline/history.h>

//in a easy way to follow the functions//
int takeComand(char* str);						   //taking user input
void Message();									   //messaging in which directory we are 
void execV(char** command);						   // exeecv system call for forking a child and in wait and execute mode
void execVPiped(char** command, char** parsedpipe);// functions for piping commands
int	lineInterpreter(char** parsed);				   //COMMAND LINE INTERPRETER |via SWITCH STATEMENTS
int gettingPipe(char* arg, char** argpiped);	   //function for finding/parsing/getting the pipe
void getCommand(char* str, char** Cparsed);		   //getting command and separating it from the argument before " "character
int processingCommands(char* str, char** parsed, char** parsedpipe);	//processing the strings which are typed by the user
void makeFolder(char* argumentsE);				   //making new folder in directory in which we want fcn

#define LETTERS 1024 //numbers of letters maximum 
#define COMMANDS 100 //number of comamnds maximum
#define clear() printf("\033[H\033[J") //clearing 

int main()
{
	char inputS[LETTERS];			//declaring input, with the size defined 
	char* parsedArgs[COMMANDS];		//declaring the arguments needed 
	char* argsPiped[COMMANDS];		//declaring pipe

	int exFlag = 0;
	printf("\nType a Command in This Shell Interpreter:"); //printing statements like menu, but far away of such form
	printf("\nFor Example:");
	printf("\nls, ls -l, ls -a, cd + " " + argument, etc.");
	printf("\nor you can even make a folder in the directory using mkdir + nameOfFolder. ");

	while (1)
	{
						//print the shell line
		while (1)
		{
			Message(); //printing message about the directory we are using every time
			break;
		}
						//take input
		if (takeComand(inputS))
			continue;
						//process it
		exFlag = processingCommands(inputS, parsedArgs, argsPiped); //execution flag used 
																	//returns zero if there is no command
		if (exFlag == 1)
		{
			execV(parsedArgs); //1 for a simple comand, like - ls, exit
		}
		if (exFlag == 2)		//2 with inclusion of pipe, like - (ls -l, cd somedirectory, ls -l -a, etc)
		{
			execVPiped(parsedArgs, argsPiped);
		}
	}
	return 0;
}

//bodies of the functions needed
int takeComand(char* str)		   //taking user input
{
	char* buffer;						//declaring buffer to hold it 
	buffer = readline("\nShell> ");		//the reading is from the Shell interpeter line	
	if (strlen(buffer) != 0)			//since it is not empty 
	{
		add_history(buffer);			//adding getting the input for the command(included history library)
		strcpy(str, buffer);			//copying the contents of the buffer
		return 0;						//returning 
	}
	else 
	{		
		return 1;						//else statement that something got wrong
	}
}

void Message()	//function for message which says in which directory we are 
{														//ready command from the library 
	char cwd[1024];										//declaring range of the array needed (1024 chars enough)
	getcwd(cwd, sizeof(cwd));							//getting it 	
	printf("\nThe directory in we are is: %s", cwd);	//print statement which tells us about the directory 
}

//body of next function is following presentation 3, whith customizations
void execV(char** command)		//function where shell must create a child process (via the fork() system call) followed by the next functinon
{
	pid_t pid = fork();			// Forking a child
	if (pid == -1)				//if statement where the child process is not created
	{
		printf("\nFailed forking child.."); //printing statement 
		return;
	}
	else if (pid == 0)			//another if statement with inside if in it 
	{
		if (execvp(command[0], command) < 0) //same procedure, but only parent existing and child cannot execute
		{
			printf("\nCould not execute command..");	//printing statement, the command is invalid
		}
		exit(0);
	}
	else 
	{					// waiting for child to terminate
		wait(NULL);		//waiting function for do so 
		return;			//return statement
	}
}

//combined function from lecture 3 + stackoverflow help - customized for this task
//maybe the most important function in this task
//creating ordinary pipe - unidirectional 
//for releationship here using zero for read-end(0) and one(1) for write end 
void execVPiped(char** command, char** parsedpipe)	//pipe commands with 2 arguments - typed in argument and the pipe itself
{	
	int pipeRW[2];	    //declaring pipe with size 
	pid_t p1, p2;		//declaring 2 processes, later used for child processes 

	if (pipe(pipeRW) < 0)	//there are nothing to put into the pipe 
	{
		printf("\nPipe could not be initialized");	//printing statement
		return;
	}

	p1 = fork();		//forking a child
	if (p1 < 0)			//if there is no such does not fork
	{
		printf("\nCould not fork"); //printing statement 
		return;
	}

	if (p1 == 0) // Child 1 executing and it write at the write end
	{
		close(pipeRW[0]);				//piping it
		dup2(pipeRW[1], STDOUT_FILENO); //dublicating it
		close(pipeRW[1]);				//closing the pipe

		if (execvp(command[0], command) < 0)//if there is no command it won't invoke any process to start
		{	
			printf("\nCould not execute command 1.."); //printing  statement
			exit(0);
		}
	}
	else				//parent is still executing but trying to invoke second child 
	{

		p2 = fork();	//forking second child
		if (p2 < 0)		//if there is no such argument for it
		{
			printf("\nCould not fork");	//did not invoking, then nothing happens
			return;
		}
		if (p2 == 0)	//when the second child is executing
		{
			close(pipeRW[1]);	//the same procedure as the child 1
			dup2(pipeRW[0], STDIN_FILENO);
			close(pipeRW[0]);	

			if (execvp(parsedpipe[0], parsedpipe) < 0)
			{
				printf("\nCould not execute command 2..");
				exit(0);
			}
		}
		else //there are 2 children processes and the parent execuring after children has already executed
		{
			// parent executing, waiting for two children
			wait(NULL);
			wait(NULL);
		}
	}
}

int lineInterpreter(char** parsed) //COMMAND LINE INTERPRETER via SWITCH STATEMENTS
{
	int numberC = 4;		//4 common commands
	int counter;			//counter needed for cycle for the parsed arguments	
	int parsedArgs = 0;		//storing parsed argumets

	char* AllCommads[numberC];	//list for all comands
								//for the switch statement would be useful because 
	AllCommads[0] = "exit";		//[0] will go to case 1
	AllCommads[1] = "cd";		//[1] will go to case 2
	AllCommads[2] = "&";		//[2] will go to case 3 - here I think this is the part B - i.e. THE EXTRA FUNCTIONALITY for the assignment
	AllCommads[3] = "mdkir";	//[3] will go to case 4

	for (counter = 0; counter < numberC; counter++) //the for loop for the procedure with the cases
	{
		if (strcmp(parsed[0], AllCommads[counter]) == 0)
		{
			parsedArgs = counter + 1;	//indeed [counter] goes to case [counter++]
			break;
		}
	}

	switch (parsedArgs)
	{
	case 1:	//exiting
		printf("\n\n");
		exit(0);

	case 2: //go to some directory 
		chdir(parsed[1]);
		return 1;

	case 3: //exiting as well, without waiting for termination of child or parent process
		printf("\nGoodbye, It has exited!\n");
		exit(0);

	case 4: //making folder in the current directory 
		makeFolder(parsed[1]);
		return 1;

	default:
		break;
	}

	return 0;
}
 
int gettingPipe(char* arg, char** argpiped) //function for finding pipe
{
	int i;
	for (i = 0; i < 2; i++) 
	{
		argpiped[i] = strsep(&arg, "|");	//not to get 2 commmands or more with |(or signal/char)
		if (argpiped[i] == NULL)
		{
			break;
		}
	}
	if (argpiped[1] == NULL) 
	{
		return 0; // returns zero if no pipe is found.
	}
	else 
	{
		return 1; //else contitue
	}
}

void getCommand(char* str, char** Cparsed) //function for parsing the command words/comands
{
	int i;
	for (i = 0; i < COMMANDS; i++) 
	{
		Cparsed[i] = strsep(&str, " ");	//checking for space and if we have command + argument,
		if (Cparsed[i] == NULL)			//it knows execlty that the command is on 1st place then SPACE than the command word
		{
			break;
		}
		if (strlen(Cparsed[i]) == 0)
		{
			i--;
		}
	}
}

int processingCommands(char* str, char** parsed, char** parsedpipe)  //processing the strings which are typed by the user
{
	//declarations
	char* strpiped[2];	
	int piped = 0;	
	piped = gettingPipe(str, strpiped); //getting the arguments piped 

	if (piped) 
	{
		getCommand(strpiped[0], parsed); //separating the command from the argument
		getCommand(strpiped[1], parsedpipe); //parsing it into the pipe
	}
	else 
	{
		getCommand(str, parsed); //the input string is parsed as a command if it is the only 
	}

	if (lineInterpreter(parsed)) //here the function about the commands is included 
	{							//and distinguishing about cd,mkdir,exit ot exiting with &
		return 0;
	}
	else
	{
		return 1 + piped; //else
	}
}

void makeFolder(char* argumentsE)               //making forlder function 
{							
	if (mkdir(argumentsE,0777))					//making folder function in LINUX, for Windows is without "0777" the checking method
	{											
		printf("Error creating directory.\n");
		return;
	}
}