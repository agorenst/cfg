#ifndef CFG_H
#define CFG_H

#include <string>
#include <list>
#include <initializer_list>
#include <iostream>
#include <set>

//////////////////////////////////////////////////////////////////////////////
// This modules present a basic CFG representation in the namespace "cfg".
// The three core datatypes:
//   1) Symbol, either a terminal or nonterminal. Unsurprisingly, this is
//      just a string.
//   2) Production, which has a symbol as the left-hand-side (lhs) and a
//      sequence of symbols as the right-hand side (rhs). This is the
//      production type composing the grammar proper.
//   3) Grammar, an explicit CFG representation. Fundamentally it is an
//      ordered sequence of productions---ordered because it is useful in
//      certain situations.
//
// 
//////////////////////////////////////////////////////////////////////////////

namespace cfg {
    // A symbol is just a basic string.
    typedef std::string symbol;

    // This is a helper type: we just want an ordered sequence
    // of objects. Probably not very efficient, but for now
    // the input I care about is very small.
    template <typename T> using sequence = std::list<T>;

    // A production has two fields, lhs -> {rhs}.
    // We make the fields const so that we can make them public without
    // being too concerned about other functions changing them under
    // our feet. This allows things to easily inspect a production.
    // We won't really be changing these too often anyways.
    struct production {
        public:
            const symbol lhs;
            const sequence<symbol> rhs;
            bool operator==(const production& p) const {
                return lhs == p.lhs && rhs == p.rhs;
            }
            bool operator>(const production& p) const {
              if (lhs == p.lhs) {
                return rhs > p.rhs;
              }
              return lhs > p.lhs;
            }
            bool operator>=(const production& p) const {
              return (*this) == p || (*this) > p;
            }
            // allows us to write something like:
            // {"a", "b", "c"} for the production a -> bc
            production(const std::initializer_list<symbol>& l):
                lhs(*l.begin()), rhs(next(l.begin()), l.end()) {}
            production(const symbol& s, const sequence<symbol>& seq): lhs(s), rhs(seq) {}

            bool operator<(const production& p) const {
                if (lhs < p.lhs) { return true; }
                if (lhs > p.lhs) { return false; }
                return rhs < p.rhs;
            }
    };

    std::ostream& operator<<(std::ostream& o, const production& p);

    // The fields here are const for the same reason as in a production.
    // There's a good number of helper functions to both formalize extra info
    // about the grammar (e.g., what defines the start symbol?) and to help
    // the user answer basic queries about the grammar.
    class grammar{
        public:
        const sequence<production> prods;
        grammar(sequence<production>& prods): prods(prods) {}

        // Allows a succinct way of writing grammars in-code.
        grammar(const std::initializer_list<production>& lst): prods(lst.begin(), lst.end()) {}

        // Maps interegers to productions and back again.
        production operator[](const int i) const;
        int index_of(const production& p) const;

        // Reasons about symbols and nonterminals.
        symbol start_symbol() const;
        sequence<production> productions_from_nonterminal(const symbol lhs) const;
        int size() const { return prods.size(); }

        // A nonterminal is any symbol that appears as a LHS in a production,
        // is any that never appears as a LHS.
        bool is_nonterminal(const symbol& p) const;
        bool is_terminal(const symbol& p) const;

        // pretty inefficient!
        std::set<symbol> all_symbols() const;
        std::set<symbol> all_nonterminals() const;
        std::set<symbol> all_terminals() const;
        const sequence<production>& all_productions() const;
    };

    std::ostream& operator<<(std::ostream& o, const grammar& g);
    // We don't use the >> operator because a grammar is all-const.
    grammar read_grammar(std::istream& o);
};

#endif
