
The ray.c program is the basis for the two progams you need to write
for assignment 1. Copy ray.c to data.c and add the Pthreads to make it
a data parallel program. Copy ray.c to task.c and add the Pthreads to make
it a task parallel program. 

Usage: [ray -s <int>] [-t <int>] [-o]

   -s Indicates the scale of the output image. The default scale is 1 which
      results in an image of 800x600 pixels. If -s is used then each dimension
      will be multiplied by that amount. A scale of 2 will result in an image
      that is four times the size of the original image.
   -t Indicates the number of threads used to process the image. The variable
      threads stores this value. You should use to indicate how many threads
      will be created. You will need to write the Pthread code.
   -o A flag which writes the ray traced image to an output file named
      image.ppm. When runing the final timing tests you don't need to create
      the output file.  When adding the Pthreads to the progam you should
      examine the output file to ensure that it is correct and that the
      introduction of threads has not broken the program.


NOTES
-----
1. There is no threading code in the program. It is part of the assignment
   to add the Pthreads to the system and to use the -t command line argument
   to control the threads.

2. There are comments in the ray.c file indicating where you need to begin
   and end recording the time used by the rendering system. They look like this:

        /*** start timing here ****/
        /*** end timing here ****/

   These have been chosen so emphasize the timing of the computational elements
   of the ray tracer and to avoid timing elements of the program which will not
   benefit from the use of Pthreads (such as initializing variables and memory,
   and writing the output file). Print your timing informaiton to the
   screen (stdout) after the ending comment.

3. Do not change the command line arguments for -s, -t, and -o. The TA will
   use them to test your progam and if they do not work as expected then he
   will deduct marks.


