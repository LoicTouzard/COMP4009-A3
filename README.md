# COMP4009-A3
MPI programming for cluster - SYSC4009 - Assignment 3 - Carleton University  
Loïc Touzard - 101057279

## Objective

This programm is a simulation of the Conway's game of Life for cluster programming with MPI.  

## Have it running

To compile, use the command `make`.
To spread the programm on the diferent cluster's machines I use `make spread`, you may use some other script to copy your programm.  
To run the programm use the command : `mpirun -np P --hostfile ./hostfile assignment3 N K M FILENAME` where the parameters are :  
* N size of the N*N board  
* k number of evolutionary steps to do  
* m output state in a file each mth step (m = 0 no output in file)
* FILENAME inputinput file name  

Fill the `hostfile` with the machines you want MPI to run.  
You can use directly the command `make run` to use the default values : `P=4`, `N=10`, `K=12`, `M=4`, `FILENAME=input.txt`. Be careful though, it will execute the `make spread` before. Change it or delete the `spread` dependancy in `run`.  
You can run the test and output the runtimes in the fle runtimes.txt with the command `make runtest`. Same remark as before about the `spread` dependancy.


## Run the tests
Use the command `make runtest` it will output the tests in two files : `runtimes.txt` and `testsOutput.txt`.  
Once more it is dependant on `spread` part in the Makefile. be sure to have it set correctly.
