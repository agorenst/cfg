#include "cfg1_to_cfg.h"

// Does the command-line version for cfg1_to_cfg.cpp

using namespace std;
using namespace cfg;

int main() {
  cout << parse_cfg1_file(std::cin);
}
