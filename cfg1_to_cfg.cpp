#include "cfg1_to_cfg.h"

#include <sstream>
#include <string>
#include <cassert>
#include <algorithm>
#include <iterator>
#include <tuple>

#include "cfg.h"

// This converts a slightly more convenient CFG format
// into our raw CFG format.
// We allow for a few meta-symbols:
//   => indicates the production operator.
//   * is the kleene star. It's whitespace-delimited.
//   | is the alternation, allowing us to write multiple productions on one line.
//   \ is the escape character, letting us include those symbols in symbols.
// Note that except for \, these should still be whitespace-delimited.
// | has the lowest precedence, + and * don't really make sense combined so they're the same.
// There are no parens or anything -- we're not supporting full regexes on RHS.
// Needless to say, none of the special characters have that meaning on the LHS.

using namespace std;
using namespace cfg;

std::string delimiter = "|";
char escape_character = '\\';
string closure_command = "*";
std::string production_operator = "=>";


sequence<production> initial_grammar;

// This is the main processing: create new productions
// based on the * operator.
sequence<production> seq_from_star(production transition) {
  // we can edit this rhs
  sequence<symbol> new_rhs(transition.rhs);
  auto star_position = find(begin(new_rhs), end(new_rhs), closure_command);

  assert(star_position != end(new_rhs));
  assert(begin(new_rhs) != star_position);

  assert(find(next(star_position), end(new_rhs), closure_command) == end(new_rhs));

  auto to_repeat_iter = prev(star_position);
  auto old_nonterm = *to_repeat_iter;
  auto new_nonterm = old_nonterm + "_star_seq";
  *to_repeat_iter = new_nonterm;
  new_rhs.erase(star_position);

  return {{transition.lhs, new_rhs},
          {new_nonterm, {old_nonterm, new_nonterm}},
          {new_nonterm, {}}};

}

// Remove the \ character properly. It's used so that
// we don't see "\*" as "*", so we simply wait until the
// end (the star-processing won't notice the \*) and remove
// all the escape characters.
string remove_escapes(string s) {
  auto it = begin(s);
  for (;;) {
    it = std::find(it, end(s), escape_character);
    if (it == end(s)) { break; }
    it = s.erase(it);
    ++it; // the whole point is we skip that character;
  }
  return s;
}
production remove_escapes(production p) {
  auto new_rhs{p.rhs};
  for (auto it = new_rhs.begin(); it != new_rhs.end(); ++it) {
    *it = remove_escapes(*it);
  }
  return {p.lhs, new_rhs};
}

grammar parse_cfg1_file(std::istream& in) {
  string line;
  string word;
  while (getline(in, line)) {
    stringstream tokenizer(line);
    string lhs;

    tokenizer >> lhs;
    if (tokenizer.eof()) {
      continue;
    }
    assert(lhs != production_operator);
    assert(lhs != closure_command);
    assert(lhs != delimiter);

    string prod_sign;
    tokenizer >> prod_sign;

    assert(prod_sign == production_operator);


    // now we read in the sub-productions.
    // This contains some tight logic to allow parsing the |
    // operator correctly.
    string rhs_word;
    while (!tokenizer.eof()) {
      symbol new_lhs;
      sequence<symbol> new_rhs;
      new_lhs = lhs;
      while (tokenizer >> rhs_word && rhs_word != delimiter) {
        new_rhs.push_back(rhs_word);
      }
      initial_grammar.push_back({new_lhs, new_rhs});
    }
  }

  sequence<production> new_grammar;
  for (auto&& p : initial_grammar) {
    if (find(begin(p.rhs), end(p.rhs), closure_command) != end(p.rhs)) {
      for (auto&& r : seq_from_star(p)) {
        // don't add a redundant production (comes up when element* is
        // used in multiple productions)
        if (find(begin(new_grammar), end(new_grammar), r) == end(new_grammar)) {
          new_grammar.push_back(remove_escapes(r));
        }
      }
    }
    else {
      new_grammar.push_back(remove_escapes(p));
    }
  }
  return grammar{new_grammar};
}
