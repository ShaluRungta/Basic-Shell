#include<stdio.h>
#include <unistd.h>
#include<stdlib.h>
#include  <sys/types.h>
#include <sys/wait.h>
#include<string.h>
#include <fcntl.h>
#define HISTORY_COUNT 10
#define MAX_CMD_LEN  128
char*** separate_piped_commands(char **, int *);
int process_spwanner(int, int, char**);
int clear_history(char *[]);
void  parse(char *, char **);
void  execute(char **);
void history(char *[], int);
char ** isredir(char **, int *, int *, char **,char **);
char * execn(char *[], int, int);
void execredir(char **, int, int, char *, char *);
char*** separate_piped_commands(char **args, int *no_of_piped_commands)
{

	char*** piped_commands = (char*** )malloc(5 * sizeof(char**));
	int k = 0;	//number of pipes 
	int i,j=0;
	piped_commands[k] = (char** ) malloc(20 * sizeof(char*));

	for (i = 0; args[i] != NULL; i++) 
	{
		if (strcmp(args[i], "|") == 0)
		{
			k++;
			j=0;
			piped_commands[k] = (char** ) malloc(20 * sizeof(char*));
		}
		else
		{
			piped_commands[k][j] = (char* )malloc(100 *sizeof(char));
			strcpy(piped_commands[k][j], args[i]);
			j++;
		}
	}

	*no_of_piped_commands = k;
	return piped_commands;	
}	

int process_spwanner(int in, int out, char** piped_commands) {
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "Fork error");
		return -1;
	}
	else if (pid == 0) {
		if (in != 0) {
			dup2(in, 0);
			close(in);
		}
		if (out != 1) {
			dup2(out, 1);
			close(out);
		}

		

			return execvp (piped_commands[0], piped_commands);
		}
		else {
			return pid;
		}
	}

	int iter_pipes(char*** piped_commands, int n)
	{
		int i, in, fd[2], status;
		pid_t pid = fork();
		if (pid < 0) {
			printf("Fork error");
			return -1;
		}
		else if (pid == 0) {
			in = 0;
			
			for (i = 0; i < n - 1; ++i) {
				pipe(fd);
				
				process_spwanner(in, fd[1], piped_commands[i]);
				
				close(fd[1]);
				
				in = fd[0];
			}
			if (in != 0) {
				dup2(in, 0);
			}
			execvp (piped_commands[i][0],piped_commands[i]);
			exit(EXIT_FAILURE);
		}
		else {
			do {
				waitpid(pid, &status, WUNTRACED);
			}
			while (!WIFEXITED(status) && !WIFSIGNALED(status));
			return 1;
		}	

		return;
	}	

void history(char *hist[], int current)
{
	pid_t  pid;
	int    status;
	pid = fork();
	if (pid < 0) {     
		printf("ERROR: forking child process failed\n");
		return;
	}
	else if (pid == 0) {        
		int i = current;
		int hist_num = 1;
		do {
			if (hist[i]) {

				hist_num++;
			}

			i = (i + 1) % HISTORY_COUNT;

		} while (i != current);

		hist_num--;
		i=current;

		do {
			if (hist[i]) {
				printf("%4d  %s\n", hist_num, hist[i]);
				hist_num--;
			}

			i = (i + 1) % HISTORY_COUNT;

		} while (i != current);
	}
	else {                                 
		while (wait(&status) != pid);
	}
}

int clear_history(char *hist[])
{
	int i;

	for (i = 0; i < HISTORY_COUNT; i++) {
		free(hist[i]);
		hist[i] = NULL;
	}

	return 0;
}

void  parse(char *line, char **argv)
{


	while (*line != '\0') {       
		while (*line == ' ' || *line == '\t' || *line == '\n')
			*line++ = '\0';     
		*argv++ = line;          
		while (*line != '\0' && *line != ' ' && 
				*line != '\t' && *line != '\n') 
			line++;            
	}
	*argv = '\0';

}
void  execute(char **argv)
{
	pid_t  pid;
	int    status;
	pid = fork();
	if (pid < 0) {     
		printf("ERROR: forking child process failed\n");
		return;
	}
	else if (pid == 0) {          
		if (execvp(argv[0], argv) < 0) {     
			printf("ERROR: exec failed\n");
			return;
		}
	}
	else {                                 
		while (wait(&status) != pid);
	}
}

char * execn(char *hist[], int current, int n)
{
	int i = current;
	int hist_num = 1;
	do {
		if (hist[i]) {

			hist_num++;
		}

		i = (i + 1) % HISTORY_COUNT;

	} while (i != current);

	hist_num--;
	i=current;
	int prev;
	do {
		if (hist[i]) {

			hist_num--;
		}
		prev=i;
		i = (i + 1) % HISTORY_COUNT;

	} while (hist_num+1 != n);

	return hist[prev]; 
}

char ** isredir(char **argv, int *in, int *out, char **input,char **output)
{

	int i=0;
	int k=0;
	int l=0;
	char** argv2=argv2 = (char** )malloc(64 * sizeof(char*));
	while(argv[i])
	{
		if(!strcmp(argv[i],">"))
		{
			*out=1;
			*output=argv[i+1];
			if(k=0)
				k=i;
			i++;
			break;
		}
		else if(!strcmp(argv[i],"<"))
		{
			*in=1;
			*input=argv[i+1];
			if(k=0)
				k=i;
			i++;

		}
		else{
			argv2[l] = (char* ) malloc(20 * sizeof(char));
			argv2[l]=strdup(argv[i]);
			i++;
			l++;

		}	

	}	
	return argv2;

}

void execredir(char **argv2, int in, int out, char *input, char *output)    // after one > only one word as output file is allowed     
{										// multiple inputs then use  < 1 2 3     donot give multiple <		
	pid_t  pid;
	int    status;
	if ((pid = fork()) < 0)
	{
		printf("ERROR: forking child process failed\n");
		return;

	}
	else if (pid == 0)
	{
		/* Be childish */
		if (in)
		{
			int fd0 = open(input, O_RDONLY);
			dup2(fd0, STDIN_FILENO);
			close(fd0);
		}

		if (out)
		{
			int fd1 = creat(output , 0644) ;
			dup2(fd1, STDOUT_FILENO);
			close(fd1);
		}
		// ...now the child has stdin coming from the input file, 
		// ...stdout going to the output file, and no extra files open.
		// ...it is safe to execute the command to be executed.
		if (execvp(argv2[0], argv2) < 0) {     
			printf("ERROR:aaaa exec failed\n");
			return;
		}

	}
	else {                                 
		while (wait(&status) != pid);
	}
}
void  main(void)
{
	int saved_stdout = dup(1);	
	dup2(STDOUT_FILENO, 1);
	char  line[1024];             
	char  *argv[64];
	char  *argv1[64];
	char ***temp;
	char *hist_temp;
	char *hist[HISTORY_COUNT];
	int i, current = 0, num_pipes=0;
	int in,out;
	char *input,*output;
	for (i = 0; i < HISTORY_COUNT; i++)
		hist[i] = NULL;   
	char *prev="";           
	while (1) {      
		in=0;  out=0;
		printf("Shell -> ");     
		gets(line);
		//printf("%s4444",line);
		free(hist[current]);
		if(line[0]!= '!' && strcmp(prev,line) && line!=NULL)     // ! !! not saved in history and in this case no new commaqnds are enterned into the history
		{	 hist[current] = strdup(line);
			prev=strdup(line);
			current = (current + 1) % HISTORY_COUNT;     
		}
		parse(line, argv);
		char **argv2=isredir(argv,&in,&out,&input,&output);
		if(!strcmp(prev,""))
			continue;
		if (strcmp(argv[0], "exit") == 0)  
			return;
		if(strcmp(argv[0], "history")==0)
			history(hist, current);
		else if (strcmp(line, "hc") == 0)
			clear_history(hist);    
		else if(strcmp(argv[0], "!!")==0)
		{	
			if(!strcmp(hist[(current+9)%10],"history"))
				history(hist,current);
			else
			{	hist_temp=strdup(hist[(current+9)%10]);
				parse(hist_temp, argv);
				temp=separate_piped_commands(argv, &num_pipes);						
				iter_pipes(temp, num_pipes+1); 
			}
		}
		else if(argv[0][0]== '!')
		{
			int j;
			int  num,sum=0;;
			for(j=1;argv[0][j]!='\0';j++)
			{
				num=argv[0][j]-'0';
				sum=sum*10+num;
			}
			hist_temp=strdup(execn(hist,current,sum));	
			parse(hist_temp,argv1);
			if(strcmp(argv1[0],"history")==0)	
				history(hist, current);
			else
			{	
				temp=separate_piped_commands(argv1, &num_pipes);						
				iter_pipes(temp, num_pipes+1);  
			}
		}  


		else if(out || in)
		{

			execredir(argv2,in,out,input,output);
		}     
		else
		{

			temp=separate_piped_commands(argv, &num_pipes);						
			iter_pipes(temp, num_pipes+1);  
		}  	
		//error in only cat command

	}
	dup2(saved_stdout, 1);
	close(saved_stdout);

}



