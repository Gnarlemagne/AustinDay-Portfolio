/* System programming fall 2017 - Austin Day
 *	
 *	jmalloc.c
 *	
 *  Reimplementation of malloc library using free-list implementation of heap memory management
 *	
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <strings.h>
#include "malloc.h"

typedef struct flist {
	int size;
	struct flist *flink;
	struct flist *blink;
} *Flist;

/* Global variables for managing malloc */

void *malloc_head = NULL;


void *malloc(size_t size) {

	if (size <= 0) {
		return NULL;
	}

	void *loc;
	int realsize;
	Flist f, ftemp;

	/* Realsize = nearest multiple of 8 equal or above size, plus 8 bytes */
	realsize = size + 8;
	if (size % 8 != 0) realsize += (8 - (size%8));

	
	/* Create our freelist if the heap is null */
	if (malloc_head == NULL) {
		malloc_head = sbrk(8192);
		f = (Flist) malloc_head;
		f->size = 8192;
		f->flink = NULL;
		f->blink = NULL;
	} 

	/* Point our freelist node to the head of the list */
	f = (Flist) malloc_head;
	


	/* Loop through the freelist nodes searching for sufficiently large chunk */
	while (realsize >= f->size) {

		/* If the last node is not big enough, allocate more memory until sufficient */
		if (f->flink == NULL) {

			sbrk(8192);
			f->size += 8192;
		
		} else {
			
			if (f->size == realsize) break;
			f = f->flink;

		}
	}


	/** By this point we know we are at a large enough node f **/


	/* Check if the chunk is barely larger than the required space*/
	/*  If so, just allocate the whole chunk unless it is the end node */
	/*  In that case, allocate more to the heap. */
	if (f->size - realsize <= 8) {
		
		if (f->flink == NULL) {
			sbrk(8192);
			f->size += 8192;
		} else {
			realsize = f->size;
		}

	}


	/* Set the location we malloc to where the current node is */
	loc = f;

	/* If the whole chunk is allocated, destroy the node unless it is the end node */
	if (f->size - realsize == 0) {
		
		if (f->flink != NULL) {
			
			f->flink->blink = f->blink;
			if (f->blink != NULL) f->blink->flink = f->flink; 
		
		}
	}
	/* Otherwise, carve off from the chunk and refactor the node accordingly */
	else {
		
		/* New node location offset (realsize) from old one */
		ftemp = (Flist) ((void *) f+realsize);
		
		/* Copy 12 bytes from the old node to the new one then modify accordingly */
		bcopy(f, ftemp, 12);
		ftemp->size -= realsize;

		/* Update freelist pointers */
		if (ftemp->blink != NULL) ftemp->blink->flink = ftemp;
		if (ftemp->flink != NULL) ftemp->flink->blink = ftemp;

		/* If it is the start node, update malloc_head */
		if (ftemp->blink == NULL) {
			malloc_head = (void *) ftemp;
		}

	}	




	((int*) loc)[0] = realsize;

	return loc + 8;

}

void free(void *ptr) {

	if (ptr == NULL) return;

	Flist freed, head;

	
	freed = (Flist) ((void *)ptr-8);
	
	head = (Flist) malloc_head;

	freed->blink = NULL;
	freed->flink = head;

	head->blink = freed;

	malloc_head = (void *) freed;

}

void *calloc(size_t nmemb, size_t size) {
	
	void *loc = malloc(size * nmemb);
	bzero(loc, size*nmemb);

	return loc;

}

void *realloc(void *ptr, size_t size) {

	void *loc = malloc(size);
	bcopy(ptr, loc, size);
	free(ptr);
	return loc;

}

/* Testing tool */
void print_flist() {
	Flist f;

	f = (Flist) malloc_head;

	printf("Freelist head: 0x%x\n", malloc_head);

	while (f != NULL) {
		printf("NODE AT: 0x%x  -  Size = %d, ", (void *) f, f->size);
		if (f->blink != NULL) printf("blink = 0x%x, ", f->blink);
		if (f->flink == NULL) printf("flink = NULL\n");
		else printf("flink = 0x%x\n", f->flink);

		f = f->flink;
	}
}

/* Jmalloc test case */
/*int main() {

	int *i1, *i2, *i3, *i4, *i5;

	

	//printf("sbrk(0) before malloc(4): 0x%x\n", sbrk(0));
	i1 = (int *) malloc(4);
	//printf("sbrk(0) after `i1 = (int *) malloc(4)': 0x%x\n", sbrk(0));
	i2 = (int *) malloc(8196);
	*i2 = 3;
	//printf("sbrk(0) after `i2 = (int *) malloc(8196)': 0x%x\n", sbrk(0));
	i3 = (int *) calloc(8, 2048);
	//printf("sbrk(0) after `i3 = (int *) calloc(8, 2048)': 0x%x\n", sbrk(0));
	
	printf("i2 = 0x%x, i3 = 0x%x\n", i2, i3);
	free((void *)i3);
	printf("Freed i3\n");

	print_flist();
	
	i4 = (int *) realloc(i2, 10000);
	printf("Realloced i4\n");
	//printf("sbrk(0) after `i4 = (int *) realloc(i2, 10000)': 0x%x\n", sbrk(0));
	i5 = (int *) malloc(2048);
	printf("i1 = 0x%x, i2 = 0x%x, i3 = 0x%x, i4 = 0x%x, i5 = 0x%x\n\n", i1, i2, i3, i4, i5);

	print_flist();
	

}*/
