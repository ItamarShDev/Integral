/*
 * program2.c
 *
 *  Created on: May 10, 2014
 *      Author: itamar
 */


//IO includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// shell includes
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//signals includes
#include <signal.h>
#include <unistd.h>
//shared memory
#include <sys/ipc.h>
#include <sys/shm.h>


#define PRCS 4
typedef struct Integral
{
	char integral[100];
	float from, to;
	double result,interval;
}Integral;

int main(int argc, char *argv[])
{
	Integral *shm_int;
	int shm_id;//the id to shared memory
	int pid;//to check if connected to the eight pid
	key_t key;
	if(argc<3)// case user has wrong input
	{
		perror("Not Enough Arguments");
		exit(1);
	}
	if(argc>3)// case user has wrong input
	{
		perror("Too Much Arguments");
		exit(1);
	}
	pid = atoi(argv[2]);
	key=ftok("/tmp",atoi(argv[1]));// connect to the shm
	if ((shm_id=shmget(key,0,0600))==-1)//if not exist
	{ perror("failed to get memory"); return EXIT_FAILURE;}

	if ((shm_int=(Integral*)shmat(shm_id,NULL,0))<0)//if integral isnt there
	{ perror("failed to get memory"); return EXIT_FAILURE;}
	while(1)
	{
		printf("please enter the integral:\n");
		scanf("%s",shm_int->integral);
		if(strcmp(shm_int->integral,"quit")==0||strcmp(shm_int->integral,"exit")==0)
		{
			kill(pid, SIGUSR1);// send the signal to alert the shm is ready
			return EXIT_SUCCESS;
		}
		else
		{
			scanf("%f, %f",&shm_int->from,&shm_int->to );
			kill(pid, SIGUSR1);// send the signal to alert the shm is ready
		}
	}
}

