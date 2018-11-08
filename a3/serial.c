/* Boids using ASCII graphics
	-original NCurses code from "Game Programming in C with the Ncurses Library"
	 https://www.viget.com/articles/game-programming-in-c-with-the-ncurses-library/
	 and from "NCURSES Programming HOWTO"
	 http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
	-Boids algorithms from "Boids Pseudocode:
	 http://www.kfish.org/boids/pseudocode.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#ifndef NOGRAPHICS
#include <unistd.h>
#include <ncurses.h>
#endif

#define DELAY 50000

// population size, number of boids
#define POPSIZE 50 
// maximum screen size, both height and width
#define SCREENSIZE 100

// boid location (x,y,z) and velocity (vx,vy,vz) in boidArray[][]
#define BX 0
#define BY 1
#define BZ 2
#define VX 3
#define VY 4
#define VZ 5

#ifndef NOGRAPHICS
// maximum screen dimensions
int max_y = 0, max_x = 0;
#endif

// location and velocity of boids
float boidArray[POPSIZE][6];

// change in velocity is stored for each boid (x,y,z)
// this is applied after all changes are calculated for each boid
float boidUpdate[POPSIZE][3];

#define DEBUG_LOG 0
char debug[128];

void initBoids()
{
	int i;
	// calculate initial random locations for each boid, scaled based on the screen size
	for (i = 0; i < POPSIZE; i++)
	{
		boidArray[i][BX] = (float)(random() % SCREENSIZE);
		boidArray[i][BY] = (float)(random() % SCREENSIZE);
		boidArray[i][BZ] = (float)(random() % SCREENSIZE);
		boidArray[i][VX] = 0.0;
		boidArray[i][VY] = 0.0;
		boidArray[i][VZ] = 0.0;
	}
}

#ifndef NOGRAPHICS
int drawBoids()
{
	int c, i;
	float multx, multy;

	// update screen maximum size
	getmaxyx(stdscr, max_y, max_x);

	// used to scale position of boids based on screen size
	multx = (float)max_x / SCREENSIZE;
	multy = (float)max_y / SCREENSIZE;

	clear();

	// display boids
	for (i = 0; i < POPSIZE; i++)
	{
		mvprintw((int)(boidArray[i][BX] * multy), (int)(boidArray[i][BY] * multx), "o");
	}

	if (DEBUG_LOG)
		mvprintw(0,0,debug);

	refresh();

	usleep(DELAY);

	// read keyboard and exit if 'q' pressed
	c = getch();
	if (c == 'q')
		return (1);
	else
		return (0);
}
#endif

// move the flock between two points
// you can optimize this funciton if you wish, or you can replace it and move it's
//    functionality into another function
void moveFlock()
{
	int i;
	static int count = 0;
	static int sign = 1;
	float px, py, pz; // (x,y,z) point which the flock moves towards

	// pull flock towards two points as the program runs
	// every 200 iterations change point that flock is pulled towards
	if (count % 200 == 0)
	{
		sign = sign * -1;
	}
	if (sign == 1)
	{
		// move flock towards position (40,40,40)
		px = 40.0;
		py = 40.0;
		pz = 40.0;
	}
	else
	{
		// move flock towards position (60,60,60)
		px = 60.0;
		py = 60.0;
		pz = 60.0;
	}
	// add offset (px,py,pz) to each boid location (x,y,z) in order to pull it
	// towards the current target point
	for (i = 0; i < POPSIZE; i++)
	{
		boidUpdate[i][BX] += (px - boidArray[i][BX]) / 200.0;
		boidUpdate[i][BY] += (py - boidArray[i][BY]) / 200.0;
		boidUpdate[i][BZ] += (pz - boidArray[i][BZ]) / 200.0;
	}
	count++;
}

void rule1(int boid)
{
	float cen[3];
	memset(cen, 0, sizeof(int)*3);
	int i, j;
	for (i = 0; i < POPSIZE; i++)
		if (i != boid)
			for (j = 0; j < 3; j++)
				cen[j] += boidArray[i][j];

	for (i = 0; i < 3; i++)
		boidUpdate[boid][i] += ((cen[i] / (POPSIZE-1)) - boidArray[boid][i]) / 150.0 ;
}

float dist(float *p1, float *p2)
{
	float sum = 0;
	int i;
	for (i = 0; i < 3; i++)
	{
		float x = p2[i] - p1[i];
		sum += x * x;
	}
	return sqrt(sum);
}

void rule2(int boid)
{
	float c[3];
	memset(c, 0, sizeof(int)*3);
	int i, j;
	for (i = 0; i < POPSIZE; i++)
		if (i != boid && dist(boidArray[boid], boidArray[i]) < 10.0)
			for (j = 0; j < 3; j++)
				c[j] -= (boidArray[i][j] - boidArray[boid][j])/5.0;
	for (i=0; i<3; i++)
		boidUpdate[boid][i] += c[i];
}

void rule3(int boid)
{
	float v[3];
	int i, j;
	for (i = 0; i < POPSIZE; i++)
		if (i != boid)
			for (j=0; j<3; j++)
				v[j] += boidArray[i][j+3];
	for (i=0; i<3; i++)
		v[i] = ((v[i]/(POPSIZE-1))-boidArray[boid][i+3])/4.0;
	for (i=0; i<3; i++)
		boidUpdate[boid][i] += v[i];
}

void (*rules[3])(int boid) = {rule1, rule2, rule3};

void moveBoids()
{
	int i, j;

	for (i = 0; i < POPSIZE; i++)
		for (j = 0; j < 3; j++)
			boidUpdate[i][j] = 0;

	/*** Your code goes here ***/
	/*** add the code to implement the three boids rules ***/
	/*** do not change any other code except to add timing to the main() routine ***/
	/*** do not replace the data structures defined in this program ***/
	/*** omp code should be used here ***/
	/*** you can call other functions from this location ***/

	for (i = 0; i < 3; i++)
		for (j = 0; j < POPSIZE; j++)
			rules[i](j);

	moveFlock();
	
	//sprintf(debug, "vel == %.1lf %.1lf", boidUpdate[0][0], boidUpdate[0][1]);

	// move boids by calculating updated velocity and new position
	// you can optimize this code if you wish and you can add omp directives here
	for (i = 0; i < POPSIZE; i++)
	{
		// update velocity for each boid using value stored in boidUpdate[][]
		boidArray[i][VX] += boidUpdate[i][BX];
		boidArray[i][VY] += boidUpdate[i][BY];
		boidArray[i][VZ] += boidUpdate[i][BZ];
		// update position for each boid using newly updated velocity
		boidArray[i][BX] += boidArray[i][VX];
		boidArray[i][BY] += boidArray[i][VY];
		boidArray[i][BZ] += boidArray[i][VZ];
	}
}

int main(int argc, char *argv[])
{
	int i, count;

#ifndef NOGRAPHICS
	// initialize curses
	initscr();
	noecho();
	cbreak();
	timeout(0);
	curs_set(FALSE);
	// Global var `stdscr` is created by the call to `initscr()`
	getmaxyx(stdscr, max_y, max_x);
#endif

	// place boids in initial position
	initBoids();

	// draw and move boids using ncurses
	// do not calculate timing in this loop, ncurses will reduce performance
#ifndef NOGRAPHICS
	while (1)
	{
		if (drawBoids() == 1)
			break;
		moveBoids();
	}
#endif

	// calculate movement of boids but do not use ncurses to draw
	// this is used for timing the parallel implementation of the program
#ifdef NOGRAPHICS
	count = 1000;
	// read number of iterations from the command line
	if (argc == 2)
		sscanf(argv[1], "%d", &count);
	printf("Number of iterations %d\n", count);

	/*** Start timing here ***/
	/*** omp code can be used here ***/
    struct timeval  start, finish;
    double elapsed;

    gettimeofday(&start, NULL);

	for (i = 0; i < count; i++)
	{
		moveBoids();
	}
	/*** End timing here ***/
	/*** print timing results ***/
    gettimeofday(&finish, NULL);

    elapsed = (double) (finish.tv_sec - start.tv_sec);
    elapsed += (double) (finish.tv_usec - start.tv_usec) / 1000000.0 ;
    //elapsed *= 1000;

    printf("time taken: %.3lfs\n", elapsed);

#endif

#ifndef NOGRAPHICS
	// shut down curses
	endwin();
#endif
}
