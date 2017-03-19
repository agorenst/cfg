#include "catch.hpp"

#include "first.h"
#include "cfg.h"

using namespace std;
using namespace cfg;

template <class K>
void print_set(const map<K, set<symbol>>& S) {
  cout << "{" << endl;
  for (auto& p : S) {
    cout << p.first << " : { ";
    for (auto& s : p.second) {
      if (s == "") { cout << "<EPS>"; }
      else { cout << s; }
      cout << " ";
    }
    cout << "}" << endl;
  }
  cout << "}" << endl;
}

TEST_CASE("Cooper and Tarczon page 105") {
  grammar right_recursive = {
    {"Goal", "Expr"},
    {"Expr", "Term", "Expr'"},
    {"Expr'", "+", "Term", "Expr'"},
    {"Expr'", "-", "Term", "Expr'"},
    {"Expr'"},
    {"Term", "Factor", "Term'"},
    {"Term'", "*", "Factor", "Term'"},
    {"Term'", "/", "Factor", "Term'"},
    {"Term'"},
    {"Factor", "(", "Expr", ")"},
    {"Factor", "num"},
    {"Factor", "name"}
  };

  map<symbol, set<symbol>> book_firsts = {
    {"Expr", {"(", "name", "num"}},
    {"Expr'", {"+", "-", ""}},
    {"Term", {"(", "name", "num"}},
    {"Term'", {"*", "/", ""}},
    {"Factor", {"(", "name", "num"}},
    // For some reason "Goal" wasn't listed in the book?
    {"Goal", {"(", "name", "num"}},
    // These terminal cases were added as the "base case" table.
    {")", {")"}},
    {"(", {"("}},
    {"name", {"name"}},
    {"num", {"num"}},
    {"*", {"*"}},
    {"+", {"+"}},
    {"-", {"-"}},
    {"/", {"/"}},
  };

  auto result = compute_first(right_recursive);
  REQUIRE(result == book_firsts);
}

TEST_CASE("Cooper and Tarczon page 106") {
  grammar right_recursive = {
    // We add "eof" here which wasn't in the first grammar...
    {"Goal", "Expr", "eof"},
    {"Expr", "Term", "Expr'"},
    {"Expr'", "+", "Term", "Expr'"},
    {"Expr'", "-", "Term", "Expr'"},
    {"Expr'"},
    {"Term", "Factor", "Term'"},
    {"Term'", "*", "Factor", "Term'"},
    {"Term'", "/", "Factor", "Term'"},
    {"Term'"},
    {"Factor", "(", "Expr", ")"},
    {"Factor", "num"},
    {"Factor", "name"}
  };

  map<symbol, set<symbol>> book_follows = {
    {"Expr", {"eof", ")"}},
    {"Expr'", {"eof", ")"}},
    {"Term", {"eof", "+", "-", ")"}},
    {"Term'", {"eof", "+", "-", ")"}},
    {"Factor", {"eof", "+", "-", "*", "/", ")"}},
    // For some reason "Goal" wasn't listed in the book?
    {"Goal", {}}
  };

  auto result = compute_follow(right_recursive);
  REQUIRE(result == book_follows);
}

TEST_CASE("Scott page 81") {
  // from page 72
  grammar simple_calculator = {
    {"program", "stmt_list", "$$"},
    {"stmt_list", "stmt", "stmt_list"},
    {"stmt_list"},
    {"stmt", "id", ":=", "expr"},
    {"stmt", "read", "id"},
    {"stmt", "write", "expr"},
    {"expr", "term", "term_tail"},
    {"term_tail", "add_op", "term", "term_tail"},
    {"term_tail"},
    {"term", "factor", "factor_tail"},
    {"factor_tail", "mult_op", "factor", "factor_tail"},
    {"factor_tail"},
    {"factor", "(", "expr", ")"},
    {"factor", "id"},
    {"factor", "number"},
    {"add_op", "+"},
    {"add_op", "-"},
    {"mult_op", "*"},
    {"mult_op", "/"}
  };
  map<symbol, set<symbol>> book_first = {
    {"program", {"id", "read", "write", "$$"}},
    {"stmt_list", {"id", "read", "write"}},
    {"stmt", {"id", "read", "write"}},
    {"expr", {"(", "id", "number"}},
    {"term_tail", {"+", "-"}},
    {"term", {"(", "id", "number"}},
    {"factor_tail", {"*", "/"}},
    {"factor", {"(", "id", "number"}},
    {"add_op", {"+", "-"}},
    {"mult_op", {"*", "/"}}
  };
  for (auto&& t : simple_calculator.all_terminals()) {
    book_first[t] = {t};
  }

  // Note: C&T allow \epsilon in the FIRST set, but
  // Scott does not. No elegant way to reconcile,
  // just need to be conscious of which "mode" we're in.
  auto result = compute_first(simple_calculator); // C&T algorithm.
  for (auto&& p : result) {
    p.second.erase("");
  }
  //print_set(result);
  //print_set(book_first);

  REQUIRE(result == book_first);

  map<symbol, set<symbol>> book_follow = {
    {"id", {"+", "-", "*", "/", ")", ":=", "id", "read", "write", "$$"}},
    {"number", {"+", "-", "*", "/", ")", "id", "read", "write", "$$"}},
    {"read", {"id"}},
    {"write", {"(", "id", "number"}},
    {"(", {"(", "id", "number"}},
    {")", {"+", "-", "*", "/", ")", "id", "read", "write", "$$"}},
    {":=", {"(", "id", "number"}},
    {"+", {"(", "id", "number"}},
    {"-", {"(", "id", "number"}},
    {"*", {"(", "id", "number"}},
    {"/", {"(", "id", "number"}},
    {"$$", {}},
    {"program", {}},
    {"stmt_list", {"$$"}},
    {"stmt", {"id", "read", "write", "$$"}},
    {"expr", {")", "id", "read", "write", "$$"}},
    {"term_tail", {")", "id", "read", "write", "$$"}},
    {"term", {"+", "-", ")", "id", "read", "write", "$$"}},
    {"factor_tail", {"+", "-", ")", "id", "read", "write", "$$"}},
    {"factor", {"+", "-", "*", "/", ")", "id", "read", "write", "$$"}},
    {"add_op", {"(", "id", "number"}},
    {"mult_op", {"(", "id", "number"}}
  };

  auto result_follow = compute_follow(simple_calculator, true);
  //print_set(result_follow);
  //print_set(book_follow);
  REQUIRE(result_follow == book_follow);

  map<production, set<symbol>> book_predict = {
    {{"program", "stmt_list", "$$"}, {"id", "read", "write", "$$"}},
    {{"stmt_list", "stmt", "stmt_list"}, {"id", "read", "write"}},
    {{"stmt_list"}, {"$$"}},
    {{"stmt", "id", ":=", "expr"}, {"id"}},
    {{"stmt", "read", "id"}, {"read"}},
    {{"stmt", "write", "expr"}, {"write"}},
    {{"expr", "term", "term_tail"}, {"(", "id", "number"}},
    {{"term_tail", "add_op", "term", "term_tail"}, {"+", "-"}},
    {{"term_tail"}, {")", "id", "read", "write", "$$"}},
    {{"term", "factor", "factor_tail"}, {"(", "id", "number"}},
    {{"factor_tail", "mult_op", "factor", "factor_tail"}, {"*", "/"}},
    {{"factor_tail"}, {"+", "-", ")", "id", "read", "write", "$$"}},
    {{"factor", "(", "expr", ")"}, {"("}},
    {{"factor", "id"}, {"id"}},
    {{"factor", "number"}, {"number"}},
    {{"add_op", "+"}, {"+"}},
    {{"add_op", "-"}, {"-"}},
    {{"mult_op", "*"}, {"*"}},
    {{"mult_op", "/"}, {"/"}},
  };

  auto result_predict = compute_predict(simple_calculator);
  // Only comes up in "program"...
  for (auto&& p : result_predict) {
    p.second.erase("");
  }
  //print_set(result_predict);
  //print_set(book_predict);
  REQUIRE(result_predict == book_predict);
}
