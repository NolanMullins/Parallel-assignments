
all: boids boidsomp go

boids: boids.c
	gcc boids.c -o boids -lncurses -lm 

boidsomp: boids.c
	gcc boids.c -o boidsomp -lm -DNOGRAPHICS

go:
	clear && ./boids

clean:
	rm boids boidsomp
	

