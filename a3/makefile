
all: boids boidsomp

boids: boids.c
	gcc boids.c -o boids -lncurses -lm 

boidsomp: boids.c
	gcc boids.c -o boidsomp -lm -DNOGRAPHICS -std=gnu99

go:
	clear && ./boids

clean:
	rm boids boidsomp
	

