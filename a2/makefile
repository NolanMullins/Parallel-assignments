all: src/main.c
	mpicc src/main.c -o a2 -lm -std=c99

run: 
	mpiexec -n 4 ./a2

clean:
	rm a2