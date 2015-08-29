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


            
            const grammar& g;
            std::shared_ptr<node> root = nullptr;

            // Assert that children are consistent with the
            // production associated with n.
            bool verify_children(node const* n) const;

            // Given a tree, make a deep copy of it.
            std::shared_ptr<node> deep_copy(node const* p) const;

            // What state is a node in?
            node_state state(node const * n) const;

            // Helper function to find the "first" undeveloped child
            node* undeveloped_child(node* p) const {
                assert(p != nullptr);
                auto s = state(p);
                if (s == node_state::undeveloped_nonterminal) { return p; }
                else if (s == node_state::terminal_leaf) { return nullptr; }
                else {
                    for (auto&& c : p->children) {
                        auto d = undeveloped_child(c.get());
                        if (d != nullptr) { return d; }
                    }
                }
                return nullptr;
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

            template<typename F>
            void map_tree(node const* t, F f) {
                f(t);
                for (auto&& c : t->children) {
                    map_tree(c.get(), f);
                }
            }

            int size() {
                int sum = 0;
                map_tree(root, [&sum](node const* t) {
                    ++sum;
                });
                return sum;
            }
            int leaf_count() {
            }

            int tree_size(node* t) const {
                if (t == nullptr) { return 0; }

                int sum = 1;
                for (auto&& c : t->children) {
                    sum += tree_size(c.get());
                }
                return sum;
            }

            int leaf_count(node* t) const {
                if (state(t) == node_state::terminal_leaf) {
                    return 1;
                }
                int sum = 0;
                for (auto&& c : t->children) {
                    sum += leaf_count(c.get());
                }
                return sum;
            }

            bool is_fully_developed(node* t) const {
                if (state(t) == node_state::undeveloped_nonterminal) {
                    return false;
                }
                for (auto&& c : t->children) {
                    if (!is_fully_developed(c.get())) {
                        return false;
                    }
                }
                return true;
            }


        public:
            parse_tree(const parse_tree& p): g(p.g), root(p.root) {}
            parse_tree(const grammar& g):
                g(g), root(new node(g.start_symbol())) {}

            // create a new parse tree, a copy of this one but with
            // a production applied
            parse_tree apply_production(int production_index) const {
                std::shared_ptr<node> new_root = deep_copy(root.get());
                parse_tree ret_value(g, new_root);
                ret_value.internal_apply_production(production_index);
                return ret_value;
            }
            
            bool has_undeveloped() const {
                return undeveloped_child(root.get()) != nullptr;
            }

            symbol undeveloped_symbol() const {
                node* und = undeveloped_child(root.get());
                assert(und != nullptr);
                return und->my_symbol;
            }

            void print_leaves(std::ostream& o) const {
                print_tree(o, root.get());
            }

            int size() const {
                return tree_size(root.get());
            }

            int leaf_count() const {
                return leaf_count(root.get());
            }

            bool is_fully_developed() const {
                return is_fully_developed(root.get());
            }

    };

}

std::ostream& operator<<(std::ostream& o, const cfg::parse_tree& p) {
    p.print_leaves(o);
    return o;
}

#endif
