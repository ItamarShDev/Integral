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
//socket
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
//shared memory
#include <sys/ipc.h>
#include <sys/shm.h>


typedef struct Integral
{
	char* integral;
	float from, to;
	float result;
	float interval;
}Integral;

#define PRCS 4
Integral* CreateSHM(Integral* in);
void freeVars(char** vars);
void Create_Integral(char* buffer, Integral *ing);
void sig_handler(int sig);//catch signal
void Creat_vars(Integral *shm_int);//creates an variables array
inline void Run_Calc(char** int_vars,Integral* shm_int);//starting to claculate
void Calc_Integral(char** vars, Integral* shm_int, int count);//runs the calculation itself
inline float _power(float x, int power);//to immitate the POWER function
void UseSocket(Integral* shm_int);
int sigusr;//to future use
int main(int argc, char *argv[])
{
	Integral* shm_int;
	shm_int =CreateSHM(shm_int);
	if(argc<1)// if didnt entered arguments
	{
		perror("wrong input");
		return EXIT_FAILURE;
	}
	UseSocket(shm_int);
	return EXIT_SUCCESS;
}
 // creates shared memory
Integral* CreateSHM(Integral* in)
{
	key_t key;
	int shm_id;//id of  shared memory
	key=ftok("/tmp",10);//create key to shm
	if ((shm_id=shmget(key,1000,IPC_CREAT |0600))==-1)//search for the memory
	{ perror("failed to get memory");}

	if ((in=(Integral*)shmat(shm_id,NULL,0))<0)//attach the integral
	{ perror("failed to get memory");}
	return in;
}
/*
 * opens the sockets then getting the integral in loop
 * until the client enter either "quit" or "exit"
 * then clean after itself and close
 */
void UseSocket(Integral* shm_int)
{
	int sockfd,newsockfd,portno,clilen, n;
	char buffer[256];
	struct sockaddr_in serv_addr,cli_addr;
	sockfd= socket(AF_INET,SOCK_STREAM,0);
	if (sockfd<0)
		perror("ERROR opening socket");

	bzero((char *) &serv_addr,sizeof(serv_addr));
	portno=1717;
	printf("\nthe port is: %d\n",portno);
	serv_addr.sin_family= AF_INET;
	serv_addr.sin_addr.s_addr= INADDR_ANY;
	serv_addr.sin_port=htons(portno);

	if (bind(sockfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
		perror("ERROR on binding");
	listen(sockfd,5);
	do
	{
		clilen=sizeof(cli_addr);
		fputs("waiting for integral to be written into shared memory\n", stdout);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
			perror("ERROR on accept");
		shm_int->result = 0;
		bzero(buffer,256);
		n = read(newsockfd, buffer, 256);
		if (n < 0)
			perror("ERROR reading from socket");
		if(strncmp(buffer,"quit", 4)==0||strncmp(buffer,"exit",4)==0)
		{
			fputs("\nClient Ended Session\n", stdout);
			close(newsockfd);//free socket
			break;
		}
		Create_Integral(buffer,shm_int);
		Creat_vars(shm_int);//create an array of variables
		printf("\nthe result is: %f\n\n",shm_int->result);
		close(newsockfd);//free socket
	}while(1);
	close(sockfd);
	shmdt(shm_int);
}
//dividing the input into integral, from, to
void Create_Integral(char* buffer, Integral *ing)
{
	int i= 0;
	char temp_int[300] ,*temp;
	strcpy(temp_int, buffer);
	temp =strtok(temp_int, ",");
	ing->integral = (char*)malloc((strlen(temp)+1)*sizeof(char));
	while (temp!=NULL)//acctually creates the array
	{
		switch(i)
		{
		case 0:
			strcpy(ing->integral,temp);//copy to temporary string
			break;
		case 1:
			ing->from = atof(temp);
			break;
		case 2:
			ing->to = atof(temp);
			break;
		}
		temp = strtok(NULL,",");
		++i;
	}
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
	freeVars(int_vars);
}
//catch signal
void sig_handler(int sig)
{
	switch(sig)
	{
	case SIGUSR1://signal from usr in program 2, to notify that integral has been inputed
		sigusr= 1;//raise flag!
		break;
	default://case other signal caught
		sigusr= 0;
	}
}

inline void Run_Calc(char** int_vars,Integral* shm_int)
{
	int c_pid[PRCS], i, status;
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
	}
	for (i = 0; i <PRCS; i++)
	{
		waitpid(c_pid[i],&status,0); //wait for child
	}
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
		for(j=0; int_vars[i][j]!='\0';j++)//search all letters
		{
			if(int_vars[i][j]=='^')//found power
			{
				//a is the first point
				//b is the a + dx
				x1 = _power((shm_int->from+count*(shm_int->interval)), atoi(&int_vars[i][j+1]) );
				x1 = x1*shm_int->interval;//calculte the square
				a+=x1;//add to the sum of a
				x2 =  _power(shm_int->from+count*(shm_int->interval)+ shm_int->interval, atoi(&int_vars[i][j+1]));
				x2 = x2*shm_int->interval;//calculte the square
				b+=x2;//add to the sum of b
				x_exist = 1;// found power
			}

			if(int_vars[i][j]=='x'&&int_vars[i][j+1]!='^')// case x without power
			{
				a+=shm_int->from+count*(shm_int->interval);//add the number*dx
				b+=shm_int->from+count*(shm_int->interval)+shm_int->interval;//add the number*dx
				a=a*shm_int->interval;
				b=b*shm_int->interval;
			}
		}
		if(!x_exist)//if didnt fount any x
		{
			a+=atof(int_vars[i]);
			b+=atof(int_vars[i]);
			x_exist =1;
		}
	}
	shm_int->result+= a>b?b:a;
}
//imitates the power function
inline float _power(float x, int power)
{
	int  i;
	float a=x;//starting with x
	for(i=1;i<power;i++)//multiply by itself as much as needed
		a= a*x;
	return a;
}
void freeVars(char** vars)
{
	int i;
	for(i=0;vars[i]!=NULL;i++)
	{
		free(vars[i]);
	}
	free(vars);
}
