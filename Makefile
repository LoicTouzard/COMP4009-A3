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
	(cd ~ & mpirun -np $(P) --hostfile ./hostfile $(EXE) $(N) $(K) $(M) $(FILENAME))
	
