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

void parse_tree::print_stack(stack<pair<size_t, parse_tree::node*>> s) {
    cout << "[";
    while(s.size() > 0) {
        cout << "(" << s.top().first << " " << s.top().second->my_symbol << ")";
        s.pop();
    }
    cout << "]";
    cout << endl;
}

// Given the result of the << operator, be able to create a new tree
// from that.
parse_tree::node* parse_tree::read_tree(std::istream& i) {
    stack<pair<size_t, node*>> working_stack;
    string nextline;
    while(getline(i,nextline)) {
        print_stack(working_stack);

        // parse the input. Fisrt, the depth
        auto depth = nextline.find_first_not_of(" ");
        if (depth % 2 != 0) { return nullptr; } // fail
        depth /= 2;

        // Then the value of the node we're reading in.
        stringstream strstr(nextline);
        string value;
        strstr >> value;

        // this is our root
        if (working_stack.size() == 0) {
            if (depth != 0) { return nullptr; } // fail
            working_stack.push(make_pair(depth, new node(value)));
        }
        else {
            int current_depth = -1;
            node* current_parent = nullptr;
            // pop until we see our parent
            tie(current_depth, current_parent) = working_stack.top();
            while(current_depth >= depth) {
                working_stack.pop();
                tie(current_depth, current_parent) = working_stack.top();
            }
            tie(current_depth, current_parent) = working_stack.top();
            // assume everything works as we want, put in error-checking later
            auto node_to_add = make_shared<node>(value);
            current_parent->children.push_back(node_to_add);
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
