/***********************/
 * cis3090 A3          *
 * Name: Nolan Mullins *
 * Student #: 0939720  *
/***********************/


-the makefile will compile two executables:
	boids -the graphical version of boids which uses ncurses
	boidsomp -the parallel version which you can use for timing tests


usage: boids
  -will run the graphical version of the boids program
  -press q to quit

usage: boidsomp <number>
  -run the parallel version of the boids program
  -<number> is the number of iterations it will perform before exiting, the default is 1000

-----------------------------------
Hardware used
-----------------------------------
    CPU: i7 4790k @ 4.30 GHz 4 cores 8 Threads
    RAM: 16GB ddr3 in duel channel
-----------------------------------

-----------------------------------
Results
-----------------------------------

Iterations      Serial (s)      OMP (s)
5 000           0.363           0.068 
10 000          0.718           0.135
50 000          3.592           0.681
100 000         7.206           1.301
500 000         36.201          6.440
1 000 000       72.062          12.589