# makefile

module2: module2.o
	mpicc -std=c99 -g -lm module2.o -o  module2
	mpirun -n 2 ./module2

module2.o: module2.c matrixFunctions.c
	mpicc -std=c99 -g -c  module2.c

sqltest: sqltest.c
	gcc -std=c99 sqltest.c -o sqltest -lsqlite3

clean:
	rm *.o
	rm -f time module2 
