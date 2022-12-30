To compile the program on the linux command line, run the following command:
	g++ -std=c++11 -o pagesim pagingsimulator.cpp
  
To run, the program takes command line arguments in the following forms:
	./pagesim <tracefile> <nframes> <lru|fifo> <debug|quiet> 
		or
	./pagesim <tracefile> <nframes> <vms> <p> <debug|quiet>

The first argument gives the name of the memory trace file to use (<tracefile>).
 
The second argument gives the number of page frames in the simulated memory. 

The third argument gives the page replacement algorithm to use (fifo, lru, vms). 

If the algorithm is "vms", then the fourth argument is the percentage of the total
	program memory to be used in the secondary buffer (in between 0 and 100).
	
The fourth(fifth if algorithm is "vms") argument may be "debug" or "quiet". "quiet" would 
	run the program silently with no output until the very end, at which point it 
	should print out a few simple statistics, while "debug" should print out messages 
	displaying the details of each event in the trace.
