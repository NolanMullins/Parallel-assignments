all: main.c
	mpicc main.c -o a2 -lm 

run: 
	mpiexec -n 4 ./a2 4 1000

runG:
	mpiexec -n 4 ./a2 4 1 -g

runBig: 
	mpiexec -n 4 ./a2 4 20000

runSmall: 
	mpiexec -n 3 ./a2 3 3

clean:
	rm a2