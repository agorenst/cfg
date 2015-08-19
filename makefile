all: cfg parse_tree

cfg: cfg.cpp cfg.h
	clang++ -Wall -std=c++11 cfg.cpp -c
parse_tree: cfg parse_tree.cpp parse_tree.h
	clang++ -Wall -std=c++11 parse_tree.cpp cfg.o -o parse_tree
