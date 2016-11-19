# COMP4009 MPI Assignment3 world makefile

CC = mpic++
LIBS = -lm
USER_CFLAGS =  -O $(INCLUDE_DIR)

EXE = assignment3
MAIN = assignment3.cpp

# runtime arguments
P = 4
N = 10
K = 12
M = 4
FILENAME = input.txt

default: $(EXE)

clean:
	rm -f $(EXE) *.o

$(EXE): $(MAIN)
	$(CC) $(USER_CFLAGS) $(MAIN) -o $(EXE) $(LIBS)
	
spread: $(EXE)
	cp $(EXE) ~
	scp $(EXE) node1:~
	scp $(EXE) node2:~
	scp $(EXE) node3:~

run: spread
	mpirun -np $(P) --hostfile ./hostfile $(EXE) $(N) $(K) $(M) $(FILENAME)

runtest: spread
	echo "Runtimes :" > runtimes.txt;
	mpirun -np 1 --hostfile ./hostfile $(EXE) 10 1 0 assignment-3-test-files/test\ 1\ input.txt >> runtimes.txt
	mpirun -np 2 --hostfile ./hostfile $(EXE) 10 2 0 assignment-3-test-files/test\ 1\ input.txt >> runtimes.txt
	mpirun -np 3 --hostfile ./hostfile $(EXE) 10 3 0 assignment-3-test-files/test\ 1\ input.txt >> runtimes.txt
	mpirun -np 4 --hostfile ./hostfile $(EXE) 10 4 0 assignment-3-test-files/test\ 1\ input.txt >> runtimes.txt
	mpirun -np 5 --hostfile ./hostfile $(EXE) 10 5 0 assignment-3-test-files/test\ 1\ input.txt >> runtimes.txt
	mpirun -np 6 --hostfile ./hostfile $(EXE) 10 6 0 assignment-3-test-files/test\ 1\ input.txt >> runtimes.txt
	mpirun -np 7 --hostfile ./hostfile $(EXE) 10 7 0 assignment-3-test-files/test\ 1\ input.txt >> runtimes.txt
	mpirun -np 8 --hostfile ./hostfile $(EXE) 10 8 0 assignment-3-test-files/test\ 1\ input.txt >> runtimes.txt


	
