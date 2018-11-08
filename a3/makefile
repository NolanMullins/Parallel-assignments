
all: boids boidsomp

boids: boids.c
	gcc boids.c -o boids -lncurses -lm 

boidsomp: boids.c
	gcc boids.c -o boidsomp -lm -DNOGRAPHICS -fopenmp

boidsS: serial.c
	gcc serial.c -o serial -lm -DNOGRAPHICS

go:
	clear && ./boids

clean:
	rm boids boidsomp serial
	

