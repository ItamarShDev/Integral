Integral
========

Reimann's Integral Calculator

*****************************************
DESCREPTION:
*****************************************
This program demonstrate the Reimann's method of Integral calculation of polynomal functions (see the
bootom of the file for an explanation)
It uses two main programming methods:
- Shared memory in combine with signals
- Sockets in client-server comunication
**


Program 1: <files ex12,ex11>
--Using Shared memory to calculate--
this program gets from the user the polynomal function and its limits, sends it to other file through the
shared memory co caculate
then prints the answer on screen.
it will continue to get integrals untill the user will enter either "quit" or "exit".


**


Program 2: <files ex21,ex22>
--using client-server Sockets method to calculate--
this program get from user the polynomal function and its limits, sends it to the server using sockets.
The server then calculate the integral and print the answer on screen.
the client will keep sending the input untill the user will enter wither "quit" or "exit".


******************************************
HOW TO:
******************************************
this program supplied with an advance Makefile.


**


COMPILE:
-cd to the files' folder in terminal
-enter "make"


**


RUN THE PROGRAM:
-cd to the files' folder in terminal
options:
-type make first for first method
-type make second for second method
-follow on the explanation int the terminal
HOW IT WORKS:
typing "make first" ot "make second"
wil run automatically the first file on the method in the terminal
then will open another terminal for the user to open the second file.
The name of the file that the user need to open will be written in the terminal.


**


USING THE PROGRAM:
After excecuting the program, youu will need to enter you function in this structure:
<FUNCTION>, <FROM>, <TO>
the function need to be entered like this:
x^1 or x for normal x
x^2,x^3...
and not 3x^2
between all variables need to be only plus ('+') and not minus.


**


AFTER USE:
after you done please type "make clean" to delete all tamporary files created and leave only the *.c files
