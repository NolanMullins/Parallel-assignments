all: main.c
	mpicc main.c -o a2 -lm -std=c99

run: 
	mpiexec -n 4 ./a2 4 10

runSmall: 
	mpiexec -n 3 ./a2 3 3

clean:
	rm a2