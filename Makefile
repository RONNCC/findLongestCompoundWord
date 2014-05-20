CC     = g++ # I built using g++ and clang++
CFLAGS = -O3 # ask the compiler to optimize for us
DEPS   =
OBJ    = findLongest.o 
LIBS   = 
BINS   = findLongest findLongestProf

#create object files from the C files
%.o: %.c $(DEPS)
	%(CC) -c -o $@ $< $(CFLAGS)

#create the program given the objects needed
findLongest: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) 

#clean any old files generated from source
clean:
	rm -f *.o *.out *~ $(BINS) callgrind* prof*

#run valgrind to check the binary for memory leaks (requires valgrind)
checkmem:
	make
	valgrind --tool=memcheck --leak-check=yes ./findLongest

#use gprof to profile the program (requires gprof)
profile: $(OBJ)
	@$(CC) -pg -o findLongestProf $^  
	@./findLongestProf > /dev/null 2>&1
	@gprof findLongestProf gmon.out > prof_analysis.txt
	@echo "Please Read the prof_analysis.txt file for the profile output"
