/* SYSTEM PROGRAMMING FALL 2017
 *	
 *	fakemake.c
 *	
 *	driver to compile projects according to the contents of a fmakefile file (or entered as arg on call)
 *  imitation of make/makefile functionality
 *	
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include "jval.h"
#include "dllist.h"
#include "fields.h"

/* Read names from an array of strings into a Dllist */
int readToDLL(Dllist dll, char** fields, int size) {
	int i;
	
	if (size == 1) return 0;

	for (i = 1; i < size; i++) {
		dll_append(dll, new_jval_s(strdup(fields[i])));
	}

	return 0;
}

/* Subroutine to build a string by adding members of a Dllist to the end */
int compileStringBuilder(char* buf, int buflen, Dllist files) {
	Dllist tmp;
	char name[256];

	dll_traverse(tmp, files) {
		strcpy(name, jval_s(tmp->val));
		
		if (strlen(name) + strlen(buf) + 1 >= buflen) return 1;
		strcat(buf, " ");
		strcat(buf, name);
	}

	return 0;	
}

/* Subroutine to build the gcc string for each C file on demand */
/* Returns system() */
int buildCFile(Dllist flags, char* filename) {
	char compileStr[256];

	strcpy(compileStr, "gcc -c");
	if (compileStringBuilder(compileStr, 256, flags)) return 1;

	if (strlen(compileStr) + strlen(filename) + 1 >= 256) return 1;

	strcat(compileStr, " ");
	strcat(compileStr, filename);

	printf("%s\n", compileStr);
	return system(compileStr);
}

int main(int argc, char** argv) {
	
	/** Variable Declarations **/
	/* Inputstructure for reading from file */
	IS is;

	/* Doubly linked string lists for C files, H files, O files, flags, and libraries */ 
	Dllist Cfiles, Hfiles, Ofiles, flags, libs;
	Cfiles = new_dllist();
	Hfiles = new_dllist();
	Ofiles = new_dllist();
	flags = new_dllist();
	libs = new_dllist();

	Dllist tmp; // for traversal

	/* Executable name and latest time of modifications for files */
	char execName[128];
	strcpy(execName, "");
	time_t latestO, latestC, latestH, latestE;

	/* Other useful stuff */
	char gccStr[1024];
	char filePath[256];
	struct stat sbuf;
	int i, exists, needsRecompile;
	

	/** Loading the descriptor file **/
	/* Not specified */
	

	if (argc == 1) {
		is = new_inputstruct("fmakefile");
	} else {
		is = new_inputstruct(argv[1]);
	}

	/* File does not exist error */
	if (is == NULL) {
		fprintf(stderr, "No such file");
		exit(1);
	}


	/** Main loop - read the descriptor file **/
	while(get_line(is) >= 0) {

		/* Blank line */
		if ((is->NF) <= 0) {
		
		}
		
		/* Read C file names */	
		else if (strcmp(is->fields[0], "C") == 0) {
			
			/* Call readToDLL to loop through fields and add filenames to Dllist */
			readToDLL(Cfiles, is->fields, is->NF);
		}
		
		/* Read H file names */
		else if (strcmp(is->fields[0], "H") == 0) {
			readToDLL(Hfiles, is->fields, is->NF);
		}

		/* Read flags */
		else if (strcmp(is->fields[0], "F") == 0) {
			readToDLL(flags, is->fields, is->NF);
		}

		/* Read libs */
		else if (strcmp(is->fields[0], "L") == 0) {
			readToDLL(libs, is->fields, is->NF);
		}

		/* Read executable name, check for error */
		else if (strcmp(is->fields[0], "E") == 0) {
			if (strcmp(execName, "") != 0) {
				fprintf(stderr, "fmakefile (%d) cannot have more than one E line\n", is->line);
				exit(1);
			}
			if (is->NF != 2) {
				fprintf(stderr, "Error: Invalid E line");
				exit(1);
			}

			strcpy(execName, is->fields[1]);
		}

		/* Catch error on unprocessable lines */
		else {
			fprintf(stderr, "Error: Invalid line");
			exit(1);
		}

	}

	/* Catch error if there is no E line */
	if (strcmp(execName, "") == 0) {
		fprintf(stderr, "No executable specified\n");
		exit(1);
	}


	/** Check if things need to be compiled / recompiled **/
	
	needsRecompile = 0;
	
	/* determine time of most recent header file modification */
	/* Set the latest value to the max possible so it'll always be bigger */
	latestH = LONG_MAX;

	/* If it's empty it shouldn't have a big time, though */
	if (dll_empty(Hfiles)) latestH = 0;



	/* Traverse Dllist of header file names */
	dll_traverse(tmp, Hfiles) {

		/* Get the file path for this H file */
		strcpy(filePath, jval_s(tmp->val));

		/* Run stat on the header, catch an error if it doesn't exist */
		exists = stat(filePath, &sbuf);
		if (exists < 0) {
			fprintf(stderr, "%s not found\n", tmp->val);
			exit(1);
		} else {
			/* Store the time of the most recent H file in latestH */
			if (sbuf.st_mtime < latestH) latestH = sbuf.st_mtime;
		}
	}
	
	/* Check for the executable file's last modified time */
	/* If it doesn't exist it needs to be made */

	exists = stat(execName, &sbuf);
	if (exists < 0) {
		needsRecompile = 1;
	} else {
		latestE = sbuf.st_mtime;
	}



	/* Iterate through C files and see if they need recompiling */

	dll_traverse(tmp, Cfiles) {
		
		/* Get C file path */
		strcpy(filePath, jval_s(tmp->val));
		
		latestC = LONG_MAX;
		latestO = LONG_MAX;

		/* Run stat on the C file - catch error if it doesn't exist */
		exists = stat(filePath, &sbuf);
		if (exists < 0) {
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", tmp->val);
			exit(1);
		} else {
			latestC = sbuf.st_mtime;
		}

		/* Change the .c to .o */
		filePath[strlen(filePath) - 1] = 'o';
		dll_append(Ofiles, new_jval_s(strdup(filePath)));

		
		exists = stat(filePath, &sbuf);

		/* Change it back to .c after stat has been run */
		filePath[strlen(filePath) - 1] = 'c';

		/* If the object file doesn't exist, C file needs compiling */
		if (exists < 0) {
			if (buildCFile(flags, jval_s(tmp->val)) != 0) {
				fprintf(stderr, "Command failed.  Exiting\n");
				exit(1);
			}
			needsRecompile = 1;
		} else {
			
			latestO = sbuf.st_mtime;
		
			/* If the object file exists but the C file or any H file is more recent, recompile */
			if (latestO < latestC || latestO < latestH) {
				if (buildCFile(flags, jval_s(tmp->val)) != 0) {
					fprintf(stderr, "Command failed.  Exiting\n");
					exit(1);
				}
				needsRecompile = 1;
			}

			/* Recompile if the executable is less recent */
			if (latestE < latestO) needsRecompile = 1;
		}

	}

	/* Link O files */


	/* Exit if executable doesnt need to be made */
	if (!needsRecompile) {
		printf("%s up to date\n", execName);
		exit(1);
	}


	/* Build string for system() command */

	strcpy(gccStr, "gcc -o ");

	/* Add the executable name */
	strcat(gccStr, execName);

	/* Add flags, then object file names, then finally libraries */
	if (compileStringBuilder(gccStr, 1024, flags)) exit(1);
	if (compileStringBuilder(gccStr, 1024, Ofiles)) exit(1);
	if (compileStringBuilder(gccStr, 1024, libs)) exit(1);


	/* Print and run! */
	printf("%s\n", gccStr);
	
	if (system(gccStr) != 0) {
		fprintf(stderr, "Command failed.  Fakemake exiting\n");
		exit(1);
	};

	jettison_inputstruct(is);
	exit(0);
}
