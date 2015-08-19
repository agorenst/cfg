all: cfg parse_tree

cfg: cfg.cpp cfg.h
	clang++ -Wall -std=c++11 cfg.cpp -o cfg
parse_tree: cfg
	clang++ -Wall -std=c++11 parse_tree.cpp -o parse_tree
