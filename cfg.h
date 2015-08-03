#include <string>

typedef std::string symbol
template<typename T> std::list<T> sequence

class production {
private:
symbol LHS
sequence<symbol> RHS
public:
};

class grammar {
private:
  sequence<production> productions
public:
  // Map integers to productions
  production operator[](const int ix) const {
    auto iter = productions.begin();
    for (int i = 0; i < ix; ++i, ++iter);
    return *iter;
  }

  // Given a symbol, return all productions that have that as their LHS
  // Really a filter over all productions
  sequence<production> productions_from_nonterminal(const symbol lhs) const {
    sequence<production> ret_value = {};
    for (auto&& p : productions) {
      if (p.lhs == lhs) {
        ret_value.insert(p);
      }
    }
    return ret_value;
  }

  // Given a sequence of symbols and an iterator that promises to point to a nonterminal in that sequence,
  // create a new sequence for each possible production implied by that nonterminal.
  // TODO: make this create a parse tree instead? Make it all more implied, you know.
  sequence<sequence<symbol>> develop_seq_at_iter(const sequence<symbol>& seq, sequence<symbol>::const_iterator splitter) const {
    assert(is_nonterminal(*splitter));
    // assert that splitter is an iterator in the collection "seq"
    sequence<sequence<symbol>> ret_value = {}
    for (auto&& prod : production_from_nonterminal(*splitter)) {
      sequence<symbol> next_development = {}
      next_development.insert(next_development.end(), seq.begin(), splitter);
      next_development.insert(next_development.end(), prod.rhs.begin(), prod.rhs.end());
      next_development.insert(next_development.end(), splitter+1, seq.end());
      ret_value.push_back(next_development);
    }
    return ret_value;
  }
};

class parse_tree {
public:
  const int production_id;
  const grammar& grammar;
  const sequence<std::shared_ptr<parse_tree>> children;
}
