/* 	SYSTEM PROGRAMMING - FALL 2017
 *	
 *	jtar.c
 *	Library to reduce files and directory trees into a single file and reextract them elsewhere
 *	
 * */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <utime.h>
#include "jval.h"
#include "jrb.h"
#include "dllist.h"
#include "fields.h"


int process_file(char* path, JRB inodes, JRB realpaths, int vf) {
	struct stat statbuf;
	struct dirent *de;
	DIR *d;
	char *s, *rp, *hlinkpath;
	int exists;
	char isHardLink;
	JRB rnode;

	/* Dllists for recursive directory access */
	Dllist files, tmp;
	files = new_dllist();


	/* Begin checking the input file path */
	exists = lstat(path, &statbuf);

	if (exists < 0) {
		fprintf(stderr, "%s not found\n", path);
		exit(1);
	}

	if (S_ISLNK(statbuf.st_mode)) {
		if (vf) fprintf(stderr, "Ignoring Soft Link %s\n", path);
		return 0;
	}

	/* Check current file against RB trees to detect duplicates/hard links */

	/** inode keyed tree to find hard links **/
	isHardLink = 0;

	rnode = jrb_find_int(inodes,statbuf.st_ino);
	if (rnode == NULL || S_ISDIR(statbuf.st_mode)) {

		jrb_insert_int(inodes, statbuf.st_ino, new_jval_s(strdup(path)));
	
	} else {
	
		isHardLink = 1;
		hlinkpath = rnode->val.s;

	}

	/** Realpath red black tree to find duplicates **/
	rp = realpath(path, NULL);
	if (jrb_find_str(realpaths, rp) == NULL) {
		jrb_insert_str(realpaths, strdup(rp), JNULL);
	} else {
		/** No point continuing this call if this file has been found before */
		return 0;
	}
	free (rp);



	/* Create a buffer of appropriate size to hold new path */
	s = (char *) malloc(sizeof(char) * (strlen(path) + 258));


	/** Processing every file **/

	
	/* Store hardlink info */
	int linkpathsize;
	fwrite(&isHardLink, sizeof(char), 1, stdout);

	if (isHardLink) {
		
		linkpathsize = strlen(hlinkpath) + 1;
		fwrite(&linkpathsize, sizeof(int), 1, stdout);
		fwrite(hlinkpath, sizeof(char) * linkpathsize, 1, stdout);

		if (vf) fprintf(stderr, "Link %s to %s\n", hlinkpath, path);

	}

	/* Store how long the file name is */
	int namesize;
	namesize = strlen(path) + 1;

	//fprintf(stderr, "adding %s with namelen %d\n", path, namesize);

	/* Write the length of the name, then the name to file */
	fwrite(&namesize, sizeof(int), 1, stdout);
	fwrite(path, namesize * sizeof(char), 1, stdout);

	/* Write the stat to stdout */
	fwrite(&statbuf, sizeof(struct stat), 1, stdout);


	/* Write file contents to stdout */
	if (!S_ISDIR(statbuf.st_mode) && !isHardLink) {
		

		/* Plank would muder me, but plank isn't teaching this class */
		FILE *fp;
		int sizeC = statbuf.st_size;
		int bufsize = 1024;
		int left = bufsize;
		char content[bufsize];

		if (vf) fprintf(stderr, "File %s    %d bytes\n", path, sizeC);
	
		fp = fopen(path, "r");
		
		/* use buffer defined by bufsize for loop */
		while (sizeC > -1) {
		
			if (sizeC > bufsize) left = bufsize;
			if (sizeC <= bufsize) { 
				left = sizeC;
				sizeC = -1;
			}
			fread(content, left, 1, fp);
			fwrite(content, left, 1, stdout);
		
			sizeC -= bufsize;

		}
		
		fclose(fp);

	}


	/* If it is a directory */
	/* Directory traversal */
	if (S_ISDIR(statbuf.st_mode)) {


		if (vf) fprintf(stderr, "Directory %s\n", path);
		d = opendir(path);
		//printf("%s/\n", path);

		/* Loop through dir members */
		for (de = readdir(d); de != NULL; de = readdir(d)) {


			/* Ignore . and .. */
			if (!strcmp(de->d_name, "..") || !strcmp(de->d_name, ".")) continue;

			/* Set path buffer to hold dir/filename */
			sprintf(s, "%s/%s", path, de->d_name);


			/* Add it to the files dllist and TODO our trees if not a duplicate */

			dll_append(files, new_jval_s(strdup(s)));

		}

		/* Close dir before traversal so we won't have multiple dirs open */
		closedir(d);

		/* Traverse an recursively call process file */
		/* With memory management so it doesnt outgrow its memory */
		dll_traverse(tmp, files) {
			process_file(tmp->val.s, inodes, realpaths, vf);
			free (tmp->val.s);

		}

		free_dllist(files);


	}



	/** Take out the trash **/
	free(s);

	return 0;
}



/* Subroutine to process one file from stdin */

int readTarStdin(int vf, Dllist createdDirs, Dllist dirMode, Dllist dirUtimes) {
	struct stat buf;
	int namelen, hlink, hlinklen;
	char *name, *hlinkname;
	FILE *fp;
	struct utimbuf ubuf;

	/* Read in hard link data */
	fread(&hlink, sizeof(char), 1, stdin);

	if (hlink) {
		
		fread(&hlinklen, sizeof(int), 1, stdin);
		
		hlinkname = (char *) malloc (sizeof(char) * hlinklen);
		fread(hlinkname, hlinklen * sizeof(char), 1, stdin);
		
	}


	/* Read in the name length */
	fread(&namelen, sizeof(int), 1, stdin);

	/* Allocate and read in the name */
	name = (char *) malloc(sizeof(char) * namelen);
	fread(name, namelen * sizeof(char), 1, stdin);

	if (feof(stdin)) return 0;

	/* Read in the stat struct */
	fread(&buf, sizeof(struct stat), 1, stdin);

	//printf("Name: %s    len %d\n", name, namelen);
	//printf("inode number: %d\nowner user ID: %d\nlast accessed: %d\n", buf.st_ino, buf.st_uid, buf.st_atime);


	mode_t mode;
	time_t modtime;
	/* Make dirs and files where needed */
	if (S_ISDIR(buf.st_mode)) {
		
		if (vf) fprintf(stderr, "Directory: %s\n", name);
		mkdir(name, 0777);
		
		mode = buf.st_mode;
		modtime = buf.st_mtime;

		dll_append(createdDirs, new_jval_s(strdup(name)));
		dll_append(dirMode, new_jval_i(mode));
		dll_append(dirUtimes, new_jval_l(modtime));

	} else if (hlink) {
		link(hlinkname, name);
		if (vf) fprintf(stderr, "Link: %s to %s\n", name, hlinkname);
	} else {
		/* Create file with contents */
		
		FILE *fp;
		int size = buf.st_size;
		int left = 0;
		char content[1024];
	
		if (vf) fprintf(stderr, "File %s    %d bytes\n", name, size);
		fp = fopen(name, "w");

		while(size > -1) {
			if (size > 1024) left = 1024;
			if (size <= 1024) { 
				left = size;
				size = -1;
			}
			
			fread(content, left, 1, stdin);
			fwrite(content, left, 1, fp);
			size -= 1024;
		}

		fclose(fp);

		chmod(name, buf.st_mode);
	}

	/* Change time for all files */
	ubuf.modtime = buf.st_mtime;
	ubuf.actime = buf.st_atime;

	utime(name, &ubuf);
	

	return 0;
}

int main(int argc, char** argv) {

	/* Modes: 0 = print to stdout (c or cv), 1 = read from stdin (x or xv) */
	int mode = -1;
	int vMode = 0;
	int i;

	Dllist createdDirs, cDirMode, cDirTimes, tmp, tmp2, tmp3;

	JRB inodes, realpaths;
	inodes = make_jrb();
	realpaths = make_jrb();

	/* lstat struct */
	struct stat statbuf;
	struct utimbuf ubuf;


	/* input handling */
	if (argc < 2) {
		fprintf(stderr, "Incorrect usage");
		exit(1);
	}

	/** c flag */
	if (strcmp(argv[1], "c") == 0 || strcmp(argv[1], "cv") == 0) {

		if (argc < 3) {
			fprintf(stderr, "No files specified\n");
			exit(1);
		}

		mode = 0;

	/** x flag */
	} else if (strcmp(argv[1], "x") == 0 || strcmp(argv[1], "xv") == 0) {

		mode = 1;

	} else {

		fprintf(stderr, "Invalid argument %s\n", argv[1]);

	}

	/* vMode = 1 if you input cv or xv as arg 1 */
	if (strlen(argv[1]) == 2) vMode = 1;


	/* Write/read handling starts here */

	/** Code for c arg **/

	if (mode == 0) {

		/* Loop through files input as arguments */
		for (i = 2; i < argc; i++) {
			if (process_file(argv[i], inodes, realpaths, vMode)) {
				fprintf(stderr, "%s: no such file\n", argv[i]);
				exit(1);
			}
		}
	}

	/** Code for x arg **/
	else if (mode == 1) {
		createdDirs = new_dllist();
		cDirMode = new_dllist();
		cDirTimes = new_dllist();
		while (!feof(stdin)) {	
			readTarStdin(vMode, createdDirs, cDirMode, cDirTimes);
		}

		/* traverse dlls in reverse so you start with deepest dirs */
		tmp2 = cDirMode;
		tmp3 = cDirTimes;
		dll_rtraverse(tmp, createdDirs) {
			tmp2 = tmp2->blink;
			tmp3 = tmp3->blink;

			lstat(tmp->val.s, &statbuf);

			ubuf.modtime = tmp3->val.l;
			ubuf.actime = statbuf.st_atime;
			
			utime(tmp->val.s, &ubuf);
			
			chmod(tmp->val.s, tmp2->val.i);
		}
	}

	return 0;
}
