#include "cfg.h"
#include "parse_tree.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cfg;

const cfg::grammar arithmetic{
    {"S", "S", "+", "S"},
    {"S", "S", "-", "S"},
    {"S", "S", "/", "S"},
    {"S", "S", "*", "S"},
    {"S", "n"}
};

int main() {
    std::ifstream infile;
    infile.open("example_tree_to_read.in");
    parse_tree p(arithmetic, infile);
    cout << p << endl;
}
