CXX=clang++
CXXFLAGS=-O2 -Wall -std=c++14 -I Catch/include
# we set this because cc is used to link.
CC=clang++

# We rely on implicit rules for C++ files.

programs=first_driver print_parse_trees remove_left_recursion closure_and_goto left_factor test_first cfg1_to_cfg

all: $(programs)

first_driver: cfg.o first.o
print_parse_trees: parse_tree.o cfg.o
remove_left_recursion: cfg.o
closure_and_goto: cfg.o
left_factor: cfg.o
test_first: catch_main.o first.o cfg.o
cfg1_to_cfg: cfg.o

clean:
	rm -f -r *.o *~ $(programs)
