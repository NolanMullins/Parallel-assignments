
-starting code for assignment 3
-you need to add the code to implment the three rules for Boids
-the program will not operate correctly until you implement the three Boids rules
-then add the omp code to make the program run in parallel
-the makefile will compile two executables:
	boids -the graphical version of boids which uses ncurses
	boidsomp -the parallel version which you can use for timing tests


usage: boids
  -will run the graphical version of the boids program
  -press q to quit

usage: boidsomp <number>
  -run the parallel version of the boids program
  -<number> is the number of iterations it will perform before exiting, the default is 1000

