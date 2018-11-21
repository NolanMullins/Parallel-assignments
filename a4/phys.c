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
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#endif

#include "data.h"

// used to slow curses animation
#define DELAY 50000

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

//used to output info to the screen
#define DEBUG_LOG 1
#define DISPLAY 1 
char debug[32][256];

void initBalls()
{
	time_t t;
	srand((unsigned) time(&t));
	int i;
	// calculate initial random locations for each ball, scaled based on the screen size
	for (i = 0; i < POPSIZE; i++)
	{
		ballArray[i][BX] =  (float)(random() % SCREENSIZE);
		ballArray[i][BY] = (float)(random() % SCREENSIZE);
		ballArray[i][VX] = (float)((random() % 5) - 2);
		ballArray[i][VY] = (float)((random() % 5) - 2);
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
		mvprintw((int)(ballArray[i][BY] * multy), (int)(ballArray[i][BX] * multx), "o");
	}

	if (DEBUG_LOG)
		for (i = 0; i < 32; i++)
			mvprintw(i, 0, debug[i]);

	refresh();

	usleep(DELAY);

	// read keyboard and exit if 'q' pressed
	c = getch();
	if (c == 'q')
		return (1);
	else
		return (0);
}

void moveBalls(cl_device_id device, cl_context* context, cl_program* program)
{
	int i, j;
	cl_int a, b, err;

	cl_kernel kernel;
	cl_command_queue queue;

	/* Data and buffers */
	float sum[32], total;
	cl_mem input_buffer, out_buffer;
	cl_int num_groups;

	/* Create data buffer */
	size_t local_size = 1, global_size = POPSIZE;
	num_groups = global_size / local_size;
	input_buffer = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, POPSIZE * 4 * sizeof(float), ballArray, &err);
	out_buffer = clCreateBuffer(*context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, POPSIZE * 4 * sizeof(float), ballArray, &err);
	if (err < 0)
	{
		perror("Couldn't create a buffer");
		exit(1);
	};

	/* Create a command queue */
	queue = clCreateCommandQueue(*context, device, 0, &err);
	if (err < 0)
	{
		perror("Couldn't create a command queue");
		exit(1);
	};

	/* Create a kernel */
	kernel = clCreateKernel(*program, "phys", &err);
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
	err = clEnqueueReadBuffer(queue, out_buffer, CL_TRUE, 0, POPSIZE * 4 * sizeof(float), ballArray, 0, NULL, NULL);
	if (err < 0)
	{
		perror("Couldn't read the buffer");
		exit(1);
	}

	clReleaseKernel(kernel);
	clReleaseMemObject(out_buffer);
	clReleaseMemObject(input_buffer);
	clReleaseCommandQueue(queue);

	//if (!DISPLAY)
	//	usleep(5000000);
}

void addVal(double* arr, double val, int index)
{
    int i;
    for (i = 0; i < index; i++)
        arr[i] = arr[i+1];
    arr[index] = val;
}

int main(int argc, char *argv[])
{
	int i, j, count;

	/* OpenCL structures */
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_int err;

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

	sprintf(debug[0], "%s (%u Mhz) (%u MB)", name, clock_frequency, (unsigned int)(mem / (1024 * 1024)));

	/* Build program */
	program = build_program(context, device, "phys.cl");

	// initialize curses
	if (DISPLAY)
	{
		initscr();
		noecho();
		cbreak();
		timeout(0);
		curs_set(FALSE);
		// Global var `stdscr` is created by the call to `initscr()`
		getmaxyx(stdscr, max_y, max_x);
	}
	else 
	{
		max_x = 50;
		max_y = 50;
	}

	// place balls in initial position
	initBalls();

	//timing information
	struct timeval start, finish;
    double elapsed;
    double times[maxTimingHist];
    int index = 0;
    memset(&times, 0, sizeof(times));

	// draw and move balls using ncurses
	while (1)
	{
		if (DISPLAY)
			if (drawBalls() == 1)
				break;
        gettimeofday(&start, NULL);
		moveBalls(device, &context, &program);
		gettimeofday(&finish, NULL);
        elapsed = (double)(finish.tv_sec - start.tv_sec);
        elapsed += (double)(finish.tv_usec - start.tv_usec) / 1000000.0;
        elapsed *= 1000;
        addVal(times, elapsed, index);
        if (index<maxTimingHist-1)
            index++;
        double tot = 0;
        for (i = 0; i < index; i++)
            tot += times[i];
        sprintf(debug[1], "Avg time: %.5lfms", tot/(double)index);
	}

	// shut down curses
	if (DISPLAY)
		endwin();

	/* Deallocate resources */
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
