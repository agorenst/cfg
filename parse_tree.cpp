#include "parse_tree.h"

#include <sstream>

using namespace cfg;
using namespace std;

parse_tree::node_state parse_tree::state(node const * n) const {
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

bool parse_tree::verify_children(node const * n) const {
    production p = g[n->production_index];

    auto match_seq = std::mismatch(p.rhs.begin(), p.rhs.end(), n->children.begin(),
    [](const symbol& s, const std::shared_ptr<node> t) { return s == t->my_symbol; });
    return match_seq.second == n->children.end();
}

std::shared_ptr<parse_tree::node> parse_tree::deep_copy(node const* p) const {
    assert(p != nullptr);
    auto new_root = make_shared<node>(p->my_symbol);
    for (auto&& c : p->children) {
        new_root->children.push_back(deep_copy(c.get()));
    }
    new_root->production_index = p->production_index;
    return new_root;
}

bool parse_tree::internal_apply_production(int production_index) {
    auto child = undeveloped_child();
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
            auto developed_child = make_shared<node>(s);
            child->children.push_back(developed_child);
        }
        child->production_index = production_index;
        assert(state(child) == node_state::developed_nonterminal);
        return true;
    }
}

// Given the result of the << operator, be able to create a new tree
// from that.

pair<size_t, string> parse_tree_line(string line) {
    auto depth = line.find_first_not_of(" ");

    stringstream strstr(line);
    string value;
    strstr >> value;

    if (   depth % 2 != 0
        || value.size() == 0) {
        return make_pair(-1, ""); // error
    }

    return make_pair(depth/2, value);
}

parse_tree::node* parse_tree::read_tree(std::istream& i) {
    stack<pair<size_t, node*>> working_stack;
    string nextline;
    while(getline(i,nextline)) {

        size_t depth;
        string value;
        tie(depth, value) = parse_tree_line(nextline);

        // skip lines we can't parse.
        if (depth == -1 || value.size() == 0) { continue; }

        // this is our root
        if (working_stack.size() == 0) {
            if (depth != 0) { return nullptr; } // fail
            working_stack.push(make_pair(depth, new node(value)));
        }
        else {
            // pop until we see our parent
            while(working_stack.top().first >= depth) {
                working_stack.pop();
            }

            int current_depth = -1;
            node* current_parent = nullptr;

            // can't deal with forests right now.
            if (working_stack.size() == 0) { return nullptr; }
            tie(current_depth, current_parent) = working_stack.top();

            // assume everything works as we want, put in error-checking later
            auto node_to_add = make_shared<node>(value);

            // add us to the tree itself
            current_parent->children.push_back(node_to_add);

            // add us to the stack.
            working_stack.push(make_pair(depth, node_to_add.get()));
        }
    }
    while(working_stack.size() > 1) { working_stack.pop(); }
    return working_stack.top().second;
}


std::ostream& operator<<(std::ostream& o, const cfg::parse_tree& p) {
    p.print_tree(o);
    return o;
}

void parse_tree::print_terminals_dfs(std::ostream& o) {
    for_each(begin(), end(), [&](node* n) {
        if (g.is_terminal(n->my_symbol)) { o << n->my_symbol << " "; }
    });
}
