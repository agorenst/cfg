#include <string>
#include <list>
#include <memory>
#include <algorithm>
#include <cassert>

typedef std::string symbol;

template <typename T> using sequence = std::list<T>;

typedef sequence<symbol>::const_iterator seq_iter;

class production {
    public:
        const symbol lhs;
        const sequence<symbol> rhs;
        bool operator==(const production& p) const {
            return lhs == p.lhs && rhs == p.rhs;
        }
};

class grammar;

class parse_tree;
typedef std::shared_ptr<parse_tree> ptree;


class grammar {
    public:
        const sequence<production> productions;

    public:
        grammar(sequence<production> prods) : productions(prods) {}
        // Map integers to productions
        production operator[](const int ix) const {
            auto iter = productions.begin();
            for (int i = 0; i < ix; ++i, ++iter)
                ;
            return *iter;
        }

        int index_of(const production p) const {
            int i = 0;
            for (auto &&q : productions) {
                if (q == p) {
                    return i;
                }
                ++i;
            }
            return i;
        }

        // Given a symbol, return all productions that have that as their LHS
        // Really a filter over all productions
        sequence<production> productions_from_nonterminal(const symbol lhs) const {
            sequence<production> ret_value = {};
            for (auto &&p : productions) {
                if (p.lhs == lhs) {
                    ret_value.insert(ret_value.begin(), p);
                }
            }
            return ret_value;
        }

        symbol start_symbol() const { return productions.begin()->lhs; }

        // Given a sequence of symbols and an iterator that promises to point to a
        // nonterminal in that sequence,
        // create a new sequence for each possible production implied by that
        // nonterminal.
        // TODO: make this create a parse tree instead? Make it all more implied, you
        // know.
        sequence<sequence<symbol>>
            develop_seq_at_iter(const sequence<symbol> &seq,
                    sequence<symbol>::const_iterator splitter) const;

        bool is_nonterminal(const symbol &s) const;
};
