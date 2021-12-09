# makefile
moduletest: moduletest.o
	mpicc -std=c99 -g -s -lm -lsqlite3 moduletest.o -o  moduletest
	mpirun -n 3 ./moduletest

moduletest.o: moduletest.c matrixFunctions.c
	mpicc -std=c99 -g -c  moduletest.c

sqltest: sqltest.c
	gcc -std=c99 sqltest.c -o sqltest -lsqlite3

module1: tree.c 
	mpicc -std=c99 -g -lm module1.c -o  module1

matrixtest: matrixtest.o
	mpicc -std=c99 -g -lm -lsqlite3 matrixtest.o -o  matrixtest
	mpirun -n 3 ./matrixtest

matrixtest.o: matrixtest.c matrixFunctions.c
	mpicc -std=c99 -g -c  matrixtest.c

clean:
	rm -f *.o
	rm -f time module2 
	rm -f module1
val:
	make moduletest
	mpirun -n 2  valgrind --track-origins=yes ./moduletest
	

