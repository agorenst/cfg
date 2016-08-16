closure_and_goto: cfg closure_and_goto.cpp
	clang++ -Wall -std=c++11 cfg.o closure_and_goto.cpp -o closure_and_goto
remove_left_recursion: cfg remove_left_recursion.cpp
	clang++ -Wall -std=c++11 cfg.o remove_left_recursion.cpp -o remove_left_recursion
all: print_parse_trees first

first: cfg first.cpp
	clang++ -Wall -std=c++11 cfg.o first.cpp -o first

cfg: cfg.cpp cfg.h
	clang++ -g -Wall -std=c++11 cfg.cpp -c
parse_tree: cfg.h parse_tree.cpp parse_tree.h
	clang++ -g -Wall -std=c++11 parse_tree.cpp -c
print_parse_trees: cfg parse_tree
	clang++ -g -Wall -std=c++11 cfg.o parse_tree.o print_parse_trees.cpp -o print_parse_trees

clean:
	rm -f *~ *.o parse_tree a.out cfg print_parse_trees first remove_left_recursion closure_and_goto
