/* SYSTEM PROGRAMMING Fall 2017 - Austin Day
 * 
 * 	jsh.c
 *  A simplistic shell program implemented completely in C
 *  Usage: ./jsh [prompt]
 *  Functionality: io redirect, pipes, zombie process cleanup
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "jval.h"
#include "fields.h"
#include "jrb.h"

int readFromStdIn(char *in);
int parseInput(char **args, char *input);
int getArgs(char **dest, char *src);
int runall(char *cmd);
int run(char **args, int fdin, int reading, int writing);
int redirectInput(char **args);
int directPipes(int fdin, int reading, int writing);
int cleanZombies();

/* File-scope variables */
static int numargs = 0; // Number of arguments when calling getArgs()
static int numChildren = 0; // Number of active child processes
static int numProcs = 0; // Number of processes to be forked off by command
//static JRB procs, node;
static int pipes[2]; // PIPES


int main(int argc, char* argv[]) {

	/* Optionally input a prompt, or use the default. */
	char prompt[64];
	int reading = 1;
	strcpy(prompt, "jsh3: ");
	if (argc == 2) { 
		/* Copy argument to the prompt if specified */
		sprintf(prompt, "%s ", argv[1]);

		/* If the prompt is input as '-', have no prompt. */
		if (!strcmp(prompt, "- ")) strcpy(prompt, "");
	}


	int stop = 0;


	/* Begin the prompt loop */
	while (!stop) {
		printf("%s", prompt);
		fflush(NULL);

		char *input = malloc(1024);

		/* Get the input from command line, handle EOF */
		if (readFromStdIn(input)) exit(0);

		/* Replace trailing newline with nul if there */
		if (input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';

		
		runall(input);

		free(input);
	}

	cleanZombies();
	return 0;


}



/* Redirect Stdin and Stdout */
int redirectInput(char **args) {
	
	int i = -1;
	int fd1, fd2;
	char *in, *out;
	in = NULL;
	out = NULL;
	int append = 0;

	/* Find terms */
	while(args[++i] != NULL) {
	
		/* Redirect stdin to come from a file */
		if (!strcmp(args[i], "<")) {
			//printf("in\n");
			args[i] = NULL;
			in = args[++i];
			if (i <= numargs) numargs = i-1;
		}

		/* Redirect stdout to a file */
		if (!strcmp(args[i], ">")) {
			//printf("out\n");
			args[i] = NULL;
			out = args[++i];
			if (i <= numargs) numargs = i-1;
		}

		/* Redirect stdout to append to a file */
		if (!strcmp(args[i], ">>")) {
			//printf("outapp\n");
			args[i] = NULL;
			out = args[++i];
			if (i <= numargs) numargs = i - 1;
			append = 1;
		}

	}
	
	/* Redirect input if needed */

	if (in != NULL) {
		//printf("In");
		fd1 = open(in, O_RDONLY);
		if (fd1 < 0) {
			perror(in);
			exit(1);
		}
		if (dup2(fd1, 0) != 0) {
			perror("fd1 redirect");
			exit(1);
		}

		close(fd1);

		args[numargs] = (char *) 0;
	}

	/* Redirect output if needed */

	if (out != NULL) {
		//printf("out");
		if(!append) fd2 = open(out, O_WRONLY | O_TRUNC | O_CREAT, 0644);
		if(append) fd2 = open(out, O_WRONLY | O_CREAT | O_APPEND, 0644);


		if (fd2 < 0) {
			perror(out);
			exit(2);
		}
		if (dup2(fd2, 1) != 1) {
			perror(out);
			exit(1);
		}

		close(fd2);
	}

	return 0;



}

/* Divide according to pipes, parse, run sequentially */
int runall(char *cmd) {

	char *args[256];
	char *next;
	int reading = 0;
	int fdin = 0;
	int wait;



	/* Find first pipe */
	next = strchr(cmd, '|');

	/* Loop until you reach the last pipe */
	while (next != NULL) {
		
		/* Replace pipe with string terminating nulchar */
		next[0] = '\0';

		/* Point next at the rest of the command */
		next++;

		/* Parse the first part as args and run */
		parseInput(args, cmd);
		fdin = run(args, fdin, reading, 1);
		reading = 1;

		/* Point cmd to the next part and restart the process */
		cmd = next;
		next = strchr(cmd, '|');
	}

	/* Run the last segment of the command */
	parseInput(args, cmd);
	run(args, fdin, reading, 0);

	

	return 0;
}

/* Directs pipes - called in child */
int directPipes(int fdin, int reading, int writing) {

	/* If reading from pipes (i.e. not first) */
	if (reading) {
		/* Redirect read side of pipe to stdin */
		dup2(fdin, 0);
		close(fdin);
	}

	/* If writing to pipes (i.e. not last) */
	if (writing) {
		/* Redirect write pipe to stdout */
		dup2(pipes[1], 1);
		close(pipes[1]);
	}

	return 0;

}

/* Run a parsed command, specifying fd + whether it is being piped to/from */
int run(char **args, int fdin, int reading, int writing) {
	
	
	/* Dammit, I forked up */
	int pid, i, status, hold;

	/* Initialize our pipes */
	if( pipe(pipes) < 0 ) {
		perror("Pipes");
		exit(1);
	}

	/* See if the last term is & */
	hold = 1;
	if (!strcmp(args[numargs-1], "&")) {
		hold = 0;
		numargs--;
		args[numargs] = NULL;
	}

	numChildren++;
	pid = fork();

	/* Execvp in child process */
	if (pid == 0) {

		directPipes(fdin, reading, writing);
		redirectInput(args);

		close(3);
		close(4);
		execvp(args[0], args);
		perror(args[0]);
		exit(1);
	}

	/* Manage open files */

	/* Nothing more needs to be written */
	if (fdin != 0) close(fdin);
	close(pipes[1]);

	/* If this is the last term, nothing needs to be read */ 
	if (!writing) close(pipes[0]);

	i = 0;

	/* If command doesn't end with &, wait */
	if (hold) {
		/* Clean up zombie processes until correct child is returned */
		while (i != pid) {
			i = wait(&status);
			numChildren--;
		}
	} else {
		numChildren++;
	}

	return pipes[0];
}


/* Kill all zombie processes - called at end */
int cleanZombies() {
	while (numChildren > 0) {
		wait(NULL);
		numChildren--;
	}

	return 0;
}

/* Command line polling function - copy line from stdin to specified buffer */
int readFromStdIn(char *in) {
	if(!fgets(in, 1024, stdin)) {
		/* Return 1 if EOF */
		return 1;
	}
	/* Return 0 for any other input */
	return 0;
}

/* Parse commands */
int parseInput(char **args, char *input) {
	/* Kill program if "exit" is typed */

	if (!strcmp(input, "exit")) exit(1);

	/* Return silently to prompt if nothing is entered */
	if (!strlen(input)) return 0;

	/* Divide input into args */
	numargs = getArgs(args, input);

	return 0;
}

/* Get an array of args from an input string */
int getArgs(char **dest, char *src) {

	int index = 0;
	char *next;

	/* Point "next" to the input string and begin loop */
	next = src;
	while (next != NULL) {

		/* Skip any whitespace */
		while (isspace(*src)) src++;

		/* Find the next space */
		next = strchr(src, ' ');

		/* If the next space doesn't exist, end */
		if (src == '\0' || next == NULL) break;

		/* Replace the space with nul to terminate the string */
		next[0] = '\0';

		/* Point the arg to the newly formed string */
		dest[index++] = src;

		/* Point the source string to the rest of the remaining string */
		src = next + 1;

	}

	/* Finish building the array */
	if (src[0] != '\0') {
		dest[index] = src;
		index++;
	}

	dest[index] = NULL;

	/* Return number of args */
	return index;

}


