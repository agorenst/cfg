#ifndef PARSE_TREE_H
#define PARSE_TREE_H

#include "cfg.h"
using namespace cfg;
#include <memory>
using namespace std;
#include <cassert>

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
class parse_tree {
    public:
        // The possible states a parse_tree node can be in.
        // It is invariant that the parse_tree is in one
        // of these states.
        enum class node_state {
            terminal_leaf,
            undeveloped_nonterminal,
            developed_nonterminal
        };
    public:
        // The data type used to express a node of a parse tree.
        // It is always in one of the three node states enumerated
        // below.
        class node {
            public:
            const grammar& g;
            int i = -1;
            sequence<std::shared_ptr<node>> c = {};
            const symbol s;
            node(const symbol s): s(s) {}

            void develop(const cfg::production& p, int index);
            
            shared_ptr<node> first_undeveloped_child() {
                assert(state() != node_state::undeveloped_nonterminal);
                for (auto&& child : c) {
                    // check our child
                    if (child->state() == node_state::undeveloped_nonterminal) {
                        return child;
                    }
                    // check our descendants
                    shared_ptr<node> und_cand = child->first_undeveloped_child();
                    if (und_cand != nullptr) { return und_cand; }
                }
                //fail
                return nullptr;
            }
            shared_ptr<const node> first_undeveloped_child() const {
                assert(state() != node_state::undeveloped_nonterminal);
                for (auto&& child : c) {
                    // check our child
                    if (child->state() == node_state::undeveloped_nonterminal) {
                        return child;
                    }
                    // check our descendants
                    shared_ptr<node> und_cand = child->first_undeveloped_child();
                    if (und_cand != nullptr) { return und_cand; }
                }
                //fail
                return nullptr;
            }
            node_state state() const;
        };

        // Essentially, the class "parse_tree" is just a wrapper over
        // some shared_ptr<node>. We could be cute and operate on just
        // the shared_ptrs directly, but that makes some things (e.g.,
        // implementing methods) difficult.
    public:
        // A parse tree only makes sense relative to some fixed grammar.
        const grammar& g;


        std::shared_ptr<node> root;

        parse_tree(const grammar& g, const symbol s): g(g), root(std::make_shared<node>(s)) {}
        parse_tree deep_copy() const;

        sequence<symbol> print_leaves();

        shared_ptr<const node> first_undeveloped() const {
            if (root->state() == node_state::undeveloped_nonterminal) {
                return root;
            }
            return root->first_undeveloped_child();
        }
        shared_ptr<node> first_undeveloped() {
            if (root->state() == node_state::undeveloped_nonterminal) {
                return root;
            }
            return root->first_undeveloped_child();
        }

        sequence<parse_tree> develop_first() const {
            sequence<parse_tree> ret_val = {};

            shared_ptr<const node> to_develop = first_undeveloped();
            // if there is no undeveloped nodes, then we're done.
            if (to_develop == nullptr) { return ret_val; }

            symbol nonterminal = to_develop->s;

            assert(g.is_nonterminal(nonterminal));
            sequence<production> all_productions = g.productions_from_nonterminal(nonterminal);
            for (auto&& prod : all_productions) {

                // create q
                parse_tree p = deep_copy();

                // find that analogous node to "to_develop"
                // (TODO: what's a nice way of asserting that?)
                shared_ptr<node> sub_develop = p.first_undeveloped();

                // develop it
                sub_develop->develop(prod, p.g.index_of(prod));

                // save it
                ret_val.push_back(p);
            }
            return ret_val;
        }
};


#endif
