# makefile

module2: module2.o
	mpicc -std=c99 -g -lm module2.o -o  module2
	mpirun -n 2 ./module2

module2.o: module2.c matrixFunctions.c
	mpicc -std=c99 -g -c  module2.c

sqltest: sqltest.c
	gcc -std=c99 sqltest.c -o sqltest -lsqlite3

module1: tree.c 
	mpicc -std=c99 -g -lm module1.c -o  module1

clean:
	rm -f *.o
	rm -f time module2 
	rm -f module1

