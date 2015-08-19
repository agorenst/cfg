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

namespace parse_tree {
    // An explicit reperesentation of a parse tree.
    // The fundamental action we care about is finding and "developing"
    // a leaf.
    class parse_tree {
        typedef cfg::symbol symbol;
        typedef cfg::grammar grammar;
        typedef cfg::production production;
        private:
            // The possible states a parse_tree node can be in.
            // It is invariant that each node is in one
            // of these states.
            enum class node_state {
                terminal_leaf,
                undeveloped_nonterminal,
                developed_nonterminal
            };

            // The grammar
            const cfg::grammar& g;

            // The recursive node type defining our tree.
            // It is just a container for some bits of data.
            class node {
                public:
                    // we assume that g provides productions
                    // in a fixed order, so this characterizer
                    // what production this node entails.
                    int production_index = -1;
                    std::list<node*> children = {};
                    const symbol my_symbol;
                public:
                    node(const symbol my_symbol): my_symbol(my_symbol) {}
            };

            // What state is a node in?
            node_state state(node* n) const {
                if (g.is_terminal(n->my_symbol)) {
                    assert(n->children.size() == 0);
                    assert(n->production_index == -1);
                    return node_state::terminal_leaf;
                }
                else {
                    if (n->production_index == -1) {
                        assert(n->children.size() == 0);
                        return node_state::undeveloped_nonterminal;
                    }
                    else {
                        assert(n->children.size() > 0);
                        assert(verify_children(n));
                        return node_state::developed_nonterminal;
                    }
                }
            }

            node* root = nullptr;

            // given a node, assert that the children it has
            // match the production the node thinks it is.
            bool verify_children(node* n) const {
                //assert(state(n) == node_state::developed_nonterminal);

                production p = g[n->production_index];

                auto production_rhs_iterator = p.rhs.begin();
                auto children_iterator =  n->children.begin();

                for (; children_iterator != n->children.end()
                    && production_rhs_iterator != p.rhs.end();
                    ++children_iterator, ++production_rhs_iterator) {
                    
                    if ((*children_iterator)->my_symbol != *production_rhs_iterator) {
                        return false;
                    }
                }
                return children_iterator == n->children.end()
                    && production_rhs_iterator == p.rhs.end();
            }


            // Helper function to find the "first" undeveloped child
            node* undeveloped_child(node* p) {
                assert(p != nullptr);
                auto s = state(p);
                if (s == node_state::undeveloped_nonterminal) { return p; }
                else if (s == node_state::terminal_leaf) { return nullptr; }
                else {
                    for (auto&& c : p->children) {
                        auto d = undeveloped_child(c);
                        if (d != nullptr) { return d; }
                    }
                }
                return nullptr;
            }

            // Given a tree, make a deep copy of it! A fun problem.
            node* deep_copy(node* p) const {
                assert(p != nullptr);
                node* new_root = new node(p->my_symbol);
                for (auto&& c : p->children) {
                    new_root->children.push_back(deep_copy(c));
                }
                new_root->production_index = p->production_index;
                return new_root;
            }

            // The main action: apply the production g[production_index]
            // to the first undeveloped node. This transforms the tree.
            bool internal_apply_production(int production_index) {
                auto child = undeveloped_child(root);
                if (child == nullptr) { return false; }
                if (state(child) != node_state::undeveloped_nonterminal) {
                    return false;
                }
                production new_production = g[production_index];
                if (child->my_symbol != new_production.lhs) {
                    return false;
                }
                else {
                    for (auto&& s : new_production.rhs) {
                        auto developed_child = new node(s);
                        child->children.push_back(developed_child);
                    }
                    child->production_index = production_index;
                    assert(state(child) == node_state::developed_nonterminal);
                    return true;
                }
            }
            parse_tree(const grammar& g, node* new_root):
                g(g), root(new_root) {
                    assert(root != nullptr);
                }

            void print_tree(std::ostream& o, node* p) const {
                // if I'm a leaf, print me
                if (p->children.size() == 0) {
                    o << p->my_symbol;
                }
                // Otherwise, get to my kids.
                else {
                    for(auto&& c : p->children) {
                        print_tree(o, c);
                    }
                }
            }


        public:
            parse_tree(const parse_tree& p): g(p.g), root(p.root) {}
            parse_tree(const grammar& g):
                g(g), root(new node(g.start_symbol())) {}

            // create a new parse tree, a copy of this one but with
            // a production applied
            parse_tree apply_production(int production_index) const {
                node* new_root = deep_copy(root);
                parse_tree ret_value(g, new_root);
                ret_value.internal_apply_production(production_index);
                return ret_value;
            }

            void print_leaves(std::ostream& o) const {
                print_tree(o, root);
            }
    };

}

std::ostream& operator<<(std::ostream& o, const parse_tree::parse_tree& p) {
    p.print_leaves(o);
    return o;
}

#endif
