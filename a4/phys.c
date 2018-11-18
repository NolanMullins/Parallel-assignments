/* Simple 2D physics for circles using ASCII graphics
	-original NCurses code from "Game Programming in C with the Ncurses Library"
	 https://www.viget.com/articles/game-programming-in-c-with-the-ncurses-library/
	 and from "NCURSES Programming HOWTO"
	 http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
	-Physics code and algorithms from "How to Create a Custom 2D Physics
	 Engine: The Basics and Impulse Resolution"
	 https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#endif

// used to slow curses animation
#define DELAY 50000

// number of balls
#define POPSIZE 50
// ball radius, all circles have the same radius
#define RADIUS 1.0
// indicate if balls collide or not
#define COLLIDE 1
#define NOCOLLIDE 0
// restitution controls how bounce the objects will be
#define RESTITUTION 0.5
// object mass
#define MASS 1.0

// maximum screen size, both height and width
#define SCREENSIZE 100

// ball location (x,y,z) and velocity (vx,vy,vz) in ballArray[][]
#define BX 0
#define BY 1
#define VX 2
#define VY 3

cl_device_id create_device();
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);

// maximum screen dimensions
int max_y = 0, max_x = 0;

// location and velocity of ball
float ballArray[POPSIZE][4];
// change in velocity is stored for each ball (x,y,z)
float ballUpdate[POPSIZE][2];

//used to output info to the screen
#define DEBUG_LOG 1
char debug[256];

void initBalls()
{
	int i;
	// calculate initial random locations for each ball, scaled based on the screen size
	for (i = 0; i < POPSIZE; i++)
	{
		ballArray[i][BX] = (float)(random() % SCREENSIZE);
		ballArray[i][BY] = (float)(random() % SCREENSIZE);
		ballArray[i][VX] = (float)((random() % 5) - 2);
		ballArray[i][VY] = (float)((random() % 5) - 2);
		ballUpdate[i][BX] = 0.0;
		ballUpdate[i][BY] = 0.0;
	}
}

int drawBalls()
{
	int c, i;
	float multx, multy;

	// update screen maximum size
	getmaxyx(stdscr, max_y, max_x);

	// used to scale position of balls based on screen size
	multx = (float)max_x / SCREENSIZE;
	multy = (float)max_y / SCREENSIZE;

	clear();

	// display balls
	for (i = 0; i < POPSIZE; i++)
	{
		mvprintw((int)(ballArray[i][BX] * multy), (int)(ballArray[i][BY] * multx), "o");
	}

	if (DEBUG_LOG)
		mvprintw(0, 0, debug);

	refresh();

	usleep(DELAY);

	// read keyboard and exit if 'q' pressed
	c = getch();
	if (c == 'q')
		return (1);
	else
		return (0);
}

// determine if two balls in ballArray collide
// calcualte distance between the two balls and compare to the
//	sum of the radii
// use balli and ballj to identify elements in ballArray[]
int ballCollision(int balli, int ballj)
{
	float distance;
	float radiiSum;

	// Pythagorean distance
	distance = sqrtf(powf((ballArray[balli][BX] - ballArray[ballj][BX]), 2) + powf((ballArray[balli][BY] - ballArray[ballj][BY]), 2));
	radiiSum = RADIUS + RADIUS;
	// if the sum of the two radii is less than the distance
	// between the balls then they collide, otherwise they
	// do not collide
	if (distance < radiiSum)
		return (COLLIDE);
	else
		return (NOCOLLIDE);
}

// calculate the dot product between two vectors
float dotProduct(float x1, float y1, float x2, float y2)
{
	return (x1 * x2 + y1 * y2);
}

// calculate effects of collision between ballArray[i][] and
// ballArray[j][] where i and j are the parameters to the function
void resolveCollision(int i, int j)
{
	float rvx, rvy;
	float nx, ny;
	float distance;
	float vnormal;
	float impulse;
	float ix, iy;

	// calculate relative velocity
	rvx = ballArray[j][VX] - ballArray[i][VX];
	rvy = ballArray[j][VY] - ballArray[i][VY];

	// calculate collision normal
	nx = ballArray[j][BX] - ballArray[i][BX];
	ny = ballArray[j][BY] - ballArray[i][BY];

	// Pythagorean distance
	distance = sqrtf(powf((ballArray[j][BX] - ballArray[i][BX]), 2) + powf((ballArray[j][BY] - ballArray[i][BY]), 2));
	if (distance == 0)
		return;

	nx = nx / distance;
	ny = ny / distance;

	// Calculate relative velocity in terms of the normal direction
	vnormal = dotProduct(rvx, rvy, nx, ny);

	// Do not resolve if velocities are separating
	if (vnormal > 0)
		return;

	// Calculate impulse scalar
	impulse = -(1 + RESTITUTION) * vnormal;
	impulse /= ((1 / MASS) + (1 / MASS));

	// Apply impulse
	ix = impulse * nx;
	iy = impulse * ny;
	ballUpdate[i][BX] = ballArray[i][VX] - ((1 / MASS) * impulse);
	ballUpdate[i][BY] = ballArray[i][VY] - ((1 / MASS) * impulse);
	ballUpdate[j][BX] = ballArray[j][VX] + ((1 / MASS) * impulse);
	ballUpdate[j][BY] = ballArray[j][VY] + ((1 / MASS) * impulse);
}

void moveBalls()
{
	int i, j;

	// update velocity of balls based upon collisions
	// compare all balls to all other circles using two loops
	for (i = 0; i < POPSIZE; i++)
	{
		for (j = i + 1; j < POPSIZE; j++)
		{
			if (ballCollision(i, j) == COLLIDE)
			{
				resolveCollision(i, j);
			}
		}
	}

	// move balls by calculating updating velocity and position
	for (i = 0; i < POPSIZE; i++)
	{
		// update velocity for each ball
		if (ballUpdate[i][BX] != 0.0)
		{
			ballArray[i][VX] = ballUpdate[i][BX];
			ballUpdate[i][BX] = 0.0;
		}
		if (ballUpdate[i][BY] != 0.0)
		{
			ballArray[i][VY] = ballUpdate[i][BY];
			ballUpdate[i][BY] = 0.0;
		}

		// enforce maximum velocity of 2.0 in each axis
		// done to make it easier to see collisions
		if (ballArray[i][VX] > 2.0)
			ballArray[i][VX] = 2.0;
		if (ballArray[i][VY] > 2.0)
			ballArray[i][VY] = 2.0;

		// update position for each ball
		ballArray[i][BX] += ballArray[i][VX];
		ballArray[i][BY] += ballArray[i][VY];

		// if ball moves off the screen then reverse velocity so it bounces
		// back onto the screen, and move it onto the screen
		if (ballArray[i][BX] > (SCREENSIZE - 1))
		{
			ballArray[i][VX] *= -1.0;
			ballArray[i][BX] = SCREENSIZE - 1.5;
		}
		if (ballArray[i][BX] < 0.0)
		{
			ballArray[i][VX] *= -1.0;
			ballArray[i][BX] = 0.5;
		}
		if (ballArray[i][BY] > (SCREENSIZE - 1))
		{
			ballArray[i][VY] *= -1.0;
			ballArray[i][BY] = SCREENSIZE - 1.5;
		}
		if (ballArray[i][BY] < 0.0)
		{
			ballArray[i][VY] *= -1.0;
			ballArray[i][BY] = 0.5;
		}
	}
}

int main(int argc, char *argv[])
{
	int i, j, count;

	/* OpenCL structures */
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue queue;
	cl_int a, b, err;
	size_t local_size, global_size;

	/* Data and buffers */
	int arrSize = 32;
	float data[arrSize];
	float sum[32], total;
	cl_mem input_buffer, out_buffer;
	cl_int num_groups;

	for (i = 0; i < arrSize; i++)
		data[i] = 1;

	/* Create device and context */
	device = create_device();
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0)
	{
		perror("Couldn't create a context");
		exit(1);
	}
	//device info
	char name[64];
	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), &name, NULL);
	cl_uint clock_frequency;
	clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
	cl_ulong mem;
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem), &mem, NULL);

	sprintf(debug, "%s (%u Mhz) (%u MB)", name, clock_frequency, (unsigned int)(mem / (1024 * 1024)));

	/* Build program */
	program = build_program(context, device, "phys.cl");

	/* Create data buffer */
	global_size = 32;
	local_size = 1;
	num_groups = global_size / local_size;
	input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, arrSize * sizeof(float), data, &err);
	out_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, num_groups * sizeof(float), sum, &err);
	if (err < 0)
	{
		perror("Couldn't create a buffer");
		exit(1);
	};

	/* Create a command queue */
	queue = clCreateCommandQueue(context, device, 0, &err);
	if (err < 0)
	{
		perror("Couldn't create a command queue");
		exit(1);
	};

	/* Create a kernel */
	kernel = clCreateKernel(program, "phys", &err);
	if (err < 0)
	{
		perror("Couldn't create a kernel");
		exit(1);
	};

	/* Create kernel arguments */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
	err |= clSetKernelArg(kernel, 1, local_size * sizeof(float), NULL);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &out_buffer);
	if (err < 0)
	{
		perror("Couldn't create a kernel argument");
		exit(1);
	}

	/* Enqueue kernel */
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
	if (err < 0)
	{
		perror("Couldn't enqueue the kernel");
		exit(1);
	}

	/* Read the kernel's output */
	err = clEnqueueReadBuffer(queue, out_buffer, CL_TRUE, 0, sizeof(sum), sum, 0, NULL, NULL);
	if (err < 0)
	{
		perror("Couldn't read the buffer");
		exit(1);
	}

	/* Check result */
	total = 0.0f;
	for (j = 0; j < num_groups; j++)
	{
		printf("sum[%d] = %.1f\n", j, sum[j]);
		total += sum[j];
	}
	printf("Computed sum = %.1f.\n", total);
	

	// initialize curses
	/*initscr();
	noecho();
	cbreak();
	timeout(0);
	curs_set(FALSE);
	// Global var `stdscr` is created by the call to `initscr()`
	getmaxyx(stdscr, max_y, max_x);

	// place balls in initial position
	initBalls();

	// draw and move balls using ncurses
	while (1)
	{
		if (drawBalls() == 1)
			break;
		moveBalls();
	}

	// shut down curses
	endwin();*/

	/* Deallocate resources */
	clReleaseKernel(kernel);
	clReleaseMemObject(out_buffer);
	clReleaseMemObject(input_buffer);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
}

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device()
{

	cl_platform_id platform;
	cl_device_id dev;
	int err;

	/* Identify a platform */
	err = clGetPlatformIDs(1, &platform, NULL);
	if (err < 0)
	{
		perror("Couldn't identify a platform");
		exit(1);
	}

	/* Access a device */
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
	if (err == CL_DEVICE_NOT_FOUND)
	{
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
	}
	if (err < 0)
	{
		perror("Couldn't access any devices");
		exit(1);
	}

	return dev;
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename)
{

	cl_program program;
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	int err;

	/* Read program file and place content into buffer */
	program_handle = fopen(filename, "r");
	if (program_handle == NULL)
	{
		perror("Couldn't find the program file");
		exit(1);
	}
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char *)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	/* Create program from file */
	program = clCreateProgramWithSource(ctx, 1,
										(const char **)&program_buffer, &program_size, &err);
	if (err < 0)
	{
		perror("Couldn't create the program");
		exit(1);
	}
	free(program_buffer);

	/* Build program */
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err < 0)
	{

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
							  0, NULL, &log_size);
		program_log = (char *)malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
							  log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		exit(1);
	}

	return program;
}
