# COMP4009 MPI Assignment3 world makefile

CC = mpicc
LIBS = -lm
USER_CFLAGS =  -O $(INCLUDE_DIR)

EXE = assignment3
MAIN = assignment3.c

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
	(cd .. & mpirun -np 4 --hostfile ./hostfile $(EXE))
	
