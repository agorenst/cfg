#include "parse_tree.h"

parse_tree::node_state parse_tree::state() const {
    if (g.is_terminal(s)) {
        assert(c.size() == 0);
        assert(i == -1);
        return node_state::terminal_leaf;
    }
    else {
        if (i == -1) {
            assert(c.size() == 0);
            return node_state::undeveloped_nonterminal;
        }
        else {
            assert(c.size() > 0);
            assert(verify_children());
            return node_state::developed_nonterminal;
        }
    }
}

// Simple recursion for copying trees.
void deep_copy_helper(shared_ptr<parse_tree::node> out, shared_ptr<parse_tree::node> in) {
    assert(out->c.size() == 0);
    for (auto&& child : in->c) {
        auto new_c = make_shared<parse_tree::node>(child->s);
        deep_copy_helper(new_c, child);
        out->c.push_back(new_c);
    }

    // Now see if the deep copying worked.
    bool symbol_checker = true;
    bool different_memory_checker = true;
    auto old_child_iter = in->c.begin();
    for (auto&& new_child : out->c) {
        if (new_child == *old_child_iter) { different_memory_checker = false; }
        if (new_child->s != (*old_child_iter)->s) { symbol_checker = false; }
        ++old_child_iter;
    }
    assert(different_memory_checker && symbol_checker);
}

parse_tree parse_tree::deep_copy() const {
    parse_tree t(g, root->s);
    deep_copy_helper(t.root, root);
}


// Private methods:

production parse_tree::get_production() const {
    assert(i >= 0);
    return g[i];
}

void parse_tree::node::develop(const cfg::production& p, int index) {
    assert(state() == undeveloped_nonterminal);

    assert(p.lhs == s);

    for (auto&& r : p.rhs) {
        c.push_back(make_shared<node>(r));
    }
    production_index = i;

    assert(state() == developed_nonterminal);
}

// These are functions that manipulate ptrees.

// In the ptree t, what is the first undeveloped node?
//ptree first_undeveloped_leaf(ptree t) {
//    auto state = t->get_state();
//    switch(state) {
//        case parse_tree::node_state::terminal_leaf:
//            return nullptr;
//        case parse_tree::node_state::undeveloped_nonterminal:
//            return t;
//        case parse_tree::node_state::developed_terminal:
//            for (auto&& p : t->c) {
//                ptree child = first_undeveloped_leaf(t);
//                if (child != nullptr) {
//                    return child;
//                }
//            }
//            return nullptr;
//        default:
//            assert(false);
//            return nullptr;
//    }
//}
//
//ptree deep_copy(ptree t) {
//    ptree root = std::make_shared<parse_tree>(*t);;
//    // reassign a child to its copy.
//    for (auto it = root->c.begin(); it != root->c.end(); ++it) {
//        *it = deep_copy(*it);
//    }
//    return root;
//}
//
//void print_ptree(std::ostream& o, const ptree p, int level ) {
//}
//
//std::ostream& operator<<(std::ostream& o, const ptree p) {
//
//}
