#ifndef PARSE_TREE_H
#define PARSE_TREE_H

//////////////////////////////////////////////////////////////////////////////
// This modules presents a basic parse tree datatype.
// A parse tree is always defined relative to some (constant) grammar.
// The simpler case is that all the leaves are terminals, and all the inner
// nodes represent the appropriate productions.
//
// But in a more computational environment, we would want to represent a
// "parse-in progress", in which some nodes representing nonterminals are
// "undeveloped", meaning that they have a fixed nonterminal, but which
// production they embody is not yet decided. Thus, a string like so:
// aaBcdEfgH, in which capital letters are nonterminal, would have a parse
// tree with 9 leaves (one for each symbol), 3 of those (the 3 nonterminals)
// being "undeveloped" nonterminal leaves. This object is worth inspecting
// in its own right, but is presumably representing a "parse-in-progress" and
// it's implied that those 3 undeveloped leaves will be developed into
// inner nodes with fixed productions, inducing more leaves etc.
//
// I suppose for specifity I should say that in the above example I assume
// epsilon is not in the language.
//
// In explicit coding terms:
// A parse tree is really best understood as an algebraic data type (I think?)
// with the following three forms:
//   1) A terminal leaf, which has no children and a terminal symbol.
//   2) An undeveloped nonterminal leaf, which has a nonterminal symbol but
//      no children
//   3) A developed inner node, which has a nonterminal symbol and a fixed
//      production, and a sequence of children nodes whose symbols exactly
//      correspond to those in the prodcuction.
//////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <cassert>
#include <stack>
#include <algorithm>
#include <iterator>

#include <iostream>
using namespace std;

// A parse tree only makes sense relative to some grammar
#include "cfg.h"

namespace cfg {
    // An explicit reperesentation of a parse tree.
    // The fundamental action we care about is finding and "developing"
    // a leaf.
    class parse_tree {
        public:
            const grammar& g;
        private:
            // The possible states a parse_tree node can be in.
            // It is invariant that each node is in one
            // of these states.
            enum class node_state {
                terminal_leaf,
                undeveloped_nonterminal,
                developed_nonterminal
            };


            // The recursive node type defining our tree.
            // It is just a container for some bits of data.
            class node {
                public:
                    // we assume that g provides productions
                    // in a fixed order, so this characterizer
                    // what production this node entails.
                    int production_index = -1;
                    std::list<std::shared_ptr<node>> children = {};
                    const symbol my_symbol;
                public:
                    node(const symbol my_symbol): my_symbol(my_symbol) {}
            };


            // Not a real iterator, but enough to hook into some basic
            // std::algorithm stuff.
            class const_iterator : public std::iterator<forward_iterator_tag, node const*>{
                private:
                node* t;
                std::stack<node*> work_list;
                public:
                const_iterator (node* t): t(t) {
                    if (t != nullptr) {
                        for (auto it = t->children.rbegin(); it != t->children.rend(); ++it) {
                            work_list.push(it->get());
                        }
                    }
                }
                void operator++() {
                    if (work_list.size()) {
                        t = work_list.top();
                        work_list.pop();
                        for (auto it = t->children.rbegin(); it != t->children.rend(); ++it) {
                            work_list.push(it->get());
                        }
                    }
                    else {
                        t = nullptr;
                    }
                }
                node* operator*() { return t; }
                bool operator!=(const const_iterator& it) const { return t != it.t; }
                bool operator==(const const_iterator& it) const { return t == it.t; }
            };

            const_iterator begin() const { return const_iterator{root.get()}; }
            const_iterator end() const { return const_iterator{nullptr}; } 
            

            // These are the only two fields in the parse tree!
            // The root pointer, and a reference to the grammar.
            std::shared_ptr<node> root = nullptr;

            // Assert that children are consistent with the
            // production associated with n.
            bool verify_children(node const* n) const;

            // Given a tree, make a deep copy of it.
            std::shared_ptr<node> deep_copy(node const* p) const;

            // What state is a node in?
            node_state state(node const * n) const;


            // Helper function to find the "first" undeveloped child
            node* undeveloped_child() const {
                auto res = find_if(begin(), end(), [&](node const* t) {
                    return state(t) == node_state::undeveloped_nonterminal;
                });
                if (res == end()) { return nullptr; }
                return *res;
            }


            // The main action: apply the production g[production_index]
            // to the first undeveloped node. This transforms the tree.
            bool internal_apply_production(int production_index);

            parse_tree(const grammar& g, std::shared_ptr<node> new_root):
                g(g), root(new_root) { assert(root != nullptr); }

            void print_tree(std::ostream& o, node* p) const {
                // if I'm a leaf, print me
                if (p->children.size() == 0) {
                    o << p->my_symbol;
                }
                // Otherwise, get to my kids.
                else {
                    for(auto&& c : p->children) {
                        print_tree(o, c.get());
                    }
                }
            }

            void print_tree_rec(std::ostream& o, node const* t, int d = 0) const {
                for (int i = 0; i < 2 *d; ++i) {
                    o << " ";
                }
                o << t->my_symbol << endl;
                for (auto&& c : t->children) {
                    print_tree_rec(o, c.get(), d+1);
                }
            }

            node* read_tree(std::istream& in);

        public:
            parse_tree(const parse_tree& p): g(p.g), root(p.root) {}
            parse_tree(const grammar& g):
                g(g), root(new node(g.start_symbol())) {}
            parse_tree(const grammar& g, std::istream& in):
                g(g), root(read_tree(in)) {}

            // create a new parse tree, a copy of this one but with
            // a production applied
            parse_tree apply_production(int production_index) const {
                std::shared_ptr<node> new_root = deep_copy(root.get());
                parse_tree ret_value(g, new_root);
                ret_value.internal_apply_production(production_index);
                return ret_value;
            }
            
            bool has_undeveloped() const {
                return undeveloped_child() != nullptr;
            }

            symbol undeveloped_symbol() const {
                node* und = undeveloped_child();
                assert(und != nullptr);
                return und->my_symbol;
            }

            void print_leaves(std::ostream& o) const {
                print_tree(o, root.get());
            }
            void print_tree(std::ostream& o) const {
                print_tree_rec(o, root.get());
            }

            int size() const {
                return std::distance(begin(), end());
            }
            int leaf_count() const {
                return std::count_if(begin(), end(), [&](node const* t) {
                    return state(t) == node_state::terminal_leaf;
                });
            }
            bool is_fully_developed() {
                return std::none_of(begin(), end(), [&](node const* t) {
                    return state(t) == node_state::undeveloped_nonterminal;
                });
            }
            void print_stack(std::stack<std::pair<size_t, node*>>);
    };

}

std::ostream& operator<<(std::ostream& o, const cfg::parse_tree& p);

#endif
