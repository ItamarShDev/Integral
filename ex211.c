/*
 * program1.c
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
#include <errno.h> //responsible for procees errors
#include <sys/wait.h>
//shared memory
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct Integral
{
	char integral[100];
	float from, to;
	double result,interval;
}Integral;

#define PRCS 4
void init_int(Integral *shm_int); //initiate all parameters to zero/NULL
void sig_handler(int sig);//catch signal
void Creat_vars(Integral *shm_int);//creates an variables array
inline void Run_Calc(char** int_vars,Integral* shm_int);//starting to calculate
void Calc_Integral(char** vars, Integral* shm_int, int count);//runs the calculation itself
inline float _power(float x, int power);//to immitate the POWER function
int sigusr;//to future use
int main(int argc, char *argv[])
{
	int shm_id;//id of  shared memory
	Integral* shm_int;
	key_t key;
	key=ftok("/tmp",atoi(argv[1]));//create key to shm
	if ((shm_id=shmget(key,1000,IPC_CREAT |0600))==-1)//search for the memory
	{ perror("failed to get memory"); return -1;}

	if ((shm_int=(Integral*)shmat(shm_id,NULL,0))<0)//attach the integral
	{ perror("failed to get memory"); return -1;}
	if(argc<2)// didnt entered arguments
	{
		perror("wrong input");
		return EXIT_FAILURE;
	}
	shm_int->result= 0;//to prevent any miscalculations
	printf("the process ID is: %d\n", getpid());//so user can connect
	while(!sigusr)
	{
		init_int(shm_int);
		printf("waiting for integral to be written into shared memory\n");
		signal(SIGUSR1, sig_handler);// catch the USR1 signal
		pause();//wait for signal to start
		if(strcmp(shm_int->integral,"quit")==0||strcmp(shm_int->integral,"exit")==0)
		{
			shmctl(shm_id,IPC_RMID,NULL);//clean memory
			return EXIT_SUCCESS;

		}
		Creat_vars(shm_int);//create an array of variables
	}
	shmctl(shm_id,IPC_RMID,NULL);//clean memory
	return EXIT_SUCCESS;
}

//catch signal
void sig_handler(int sig)
{
	switch(sig)
	{
	case SIGUSR1://signal from usr in program 2, to notify that integral has been inputed
		sigusr= 0;//raise flag!
		break;
	}
}
void init_int(Integral *shm_int)
{
	bzero(shm_int->integral, 100);
	shm_int->from = 0;
	shm_int->to = 0;
	shm_int->result = 0;
	shm_int->interval = 0;
}
/*
 *this function creates an array of variables
 *this is used for ease the calculation
 *
 *example:
 *if the integral is: x^2+X^1+1
 *the result is {x^2,x^1,1}
 */
void Creat_vars(Integral *shm_int)//create varaibles array
{
	char **int_vars;//the result
	int i,j,size = 0;
	char temp_int[100] ,*temp;//the temporary variables
	//count the number of variables
	for(i=0;i<strlen(shm_int->integral);i++)
	{
		if(shm_int->integral[i]=='+'||shm_int->integral[i]=='-')
			size++;//the variables counter
	}
	int_vars =(char**)malloc(sizeof(char*)*(size+1));// allocate the memory
	i = 0;//start from scratch
	//copy to the temp string,
	//this is beacuse the strtok function destroys the source
	strcpy(temp_int, shm_int->integral);
	temp =strtok(temp_int, "+");//get first part until the first +
	while (i<=size)//creates the array
	{
		int_vars[i] = (char*)malloc(sizeof(char)*strlen(temp)+1);//allocate memory for the next variable
		strcpy(int_vars[i],temp);//copy to temporary string
		temp = strtok(NULL,"+");//get next variable
		++i;
	}
	int_vars[i]=NULL;//set last to null for future searches
	for(j=0;j<strlen(int_vars[i-1]);j++)//to eliminate the , from the last variable
	{
		if (int_vars[i-1][j] == ',')
			int_vars[i-1][j]= '\0';//set it to finish here
	}
	Run_Calc(int_vars,shm_int);//run calculation
}

inline void Run_Calc(char** int_vars,Integral* shm_int)
{
	int c_pid[PRCS], i;

	for(i=0;i<PRCS;i++)
	{
		c_pid[i]= fork();//create proccess
		if(c_pid[i]==0)
		{
			Calc_Integral(int_vars,shm_int,i);//run Riemann's algorithm
			exit(1);//done
		}
		if (c_pid[i]<0)
		{
			perror("could'nt create a child");
		}
		if (c_pid[i]>0)// if father, wait for the child to finish
		{
			waitpid(c_pid[i], NULL, 0);
			close(c_pid[i]);
		}
	}
	printf("\nthe result is: %f\n", shm_int->result);
}

/*
 *METHOD:
 *using Riemann's algorithm
 *take X and divide it by the proccess number and call it dx
 *count the function in every point (from, from+dx....to)
 *take the minimum of every two points and moltiply it by dx
 *sum all together to get the result
 *
 *the more proccess, the more accurate the result
 */
void Calc_Integral(char** int_vars,Integral* shm_int, int count)
{
	int i,j=0, x_exist = 0;
	float a=0, b=0, x1, x2;
	shm_int->interval = (shm_int->to-shm_int->from)/PRCS;//dx
	for(i=0; int_vars[i]!=NULL;i++)//run all words
	{
	x_exist = 0;
		for(j=0; int_vars[i][j]!='\0';j++)//search all letters
		{
			if(int_vars[i][j]=='x'&&int_vars[i][j+1]!='^')// case x without power
			{
				a+=shm_int->from+count*(shm_int->interval);//add the number*dx
				b+=shm_int->from+count*(shm_int->interval);//add the number*dx
				x_exist = 1;// found power
			}
			if(int_vars[i][j]=='^')//found power
			{
				//a is the first point
				//b is the a + interval
				x1 = _power((shm_int->from+count*(shm_int->interval)), atoi(&int_vars[i][j+1]) );
				a+=x1;//add to the sum of a
				x2 =  _power(shm_int->from+count*(shm_int->interval)+ shm_int->interval, atoi(&int_vars[i][j+1]));
				b+=x2;//add to the sum of b
				x_exist = 1;// found power
			}
		}
		if(!x_exist)//if didnt fount any x
		{
			a+=atof(int_vars[i]);
			b+=atof(int_vars[i]);
			x_exist =1;
		}	
	}
	a = a*shm_int->interval;//calculte the square
	b = b*shm_int->interval;//calculte the square
	shm_int->result +=a>b?b:a ;// add the minimum
}
//immitates the power function
inline float _power(float x, int power)
{
	int  i;
	float a=x;//starting with x
	for(i=1;i<power;i++)//multiply by itself as much as needed
		a= a*x;
	return a;
}
