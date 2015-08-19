#include "parse_tree.h"

using namespace std;

const cfg::grammar arithmetic{
    {"S", "S", "+", "S"},
    {"S", "S", "-", "S"},
    {"S", "S", "/", "S"},
    {"S", "S", "*", "S"},
    {"S", "n"}
};

int main() {
    parse_tree::parse_tree start(arithmetic);
    auto next = start.apply_production(2);

    cout << start << endl;
    cout << next << endl;
}
