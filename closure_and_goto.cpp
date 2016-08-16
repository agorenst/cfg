#include <vector>
#include <iterator>
#include <map>
#include "cfg.h"

using namespace std;
using namespace cfg;

const symbol EPS = "";


grammar Augment(const grammar& g) {
    auto start = g.start_symbol();
    sequence<production> new_productions{{start+"'", {start}}};
    for (auto&& p : g.prods) {
        new_productions.push_back(p);
    }
    return grammar{new_productions};
}

struct item {
    const int production_id;
    const int dot_index;
    bool operator<(const item& it) const {
        if (production_id < it.production_id) {
            return true;
        }
        if (production_id > it.production_id) {
            return false;
        }
        return dot_index < it.dot_index;
    }
};

set<item> compute_closure(set<item> I, const grammar g) {
    set<item> closure{I};

    bool workDone = true;
    while (workDone) {
        workDone = false;
        auto old_size = closure.size();
        vector<item> to_add;
        for (auto& it : closure) {
            auto prod = g[it.production_id];
            auto b_iter = prod.rhs.begin();
            advance(b_iter, it.dot_index);
            if (b_iter == prod.rhs.end() || g.is_terminal(*b_iter)) {
                continue;
            }
            for (auto&& p : g.productions_from_nonterminal(*b_iter)) {
                auto i = g.index_of(p);
                to_add.push_back({i,0}); 
            }
        }
        closure.insert(to_add.begin(), to_add.end());
        workDone = old_size != closure.size();
    }
    return closure;
}

//map<item, set<item>> closure(const grammar g) {
//    map<item, set<item>> ret;
//    for (int i = 0; i < g.size(); ++i) {
//        for (int j = 0; j < g[i].rhs.size(); ++j) {
//            item my_item{i,j};
//            ret[my_item].insert(my_item);
//        }
//    }
//
//    bool workDone = true;
//    while (workDone) {
//        workDone = false;
//        for (auto&& item_and_set : ret) {
//            auto it = item_and_set.first;
//            auto prod = g[it.production_id];
//            auto Biter = prod.rhs.begin();
//            advance(Biter, it.dot_index);
//            if (Biter == prod.rhs.end() || g.is_terminal(*Biter)) {
//                continue;
//            }
//            // otherwise, Biter is an iterator pointing to some nonterminal B.
//            set<item>& set_to_modify = ret[it];
//            auto old_size = set_to_modify.size();
//            for (auto&& p : g.productions_from_nonterminal(*Biter)) {
//                auto i = g.index_of(p);
//                set_to_modify.insert({i,0});
//            }
//            if (set_to_modify.size() != old_size) {
//                workDone = true;
//            }
//        }
//    }
//    return ret;
//}

void print_item(item it, const grammar g) {
    auto prod = g[it.production_id];
    cout << "[" << prod.lhs << " -> ";
    int i = 0;
    for (auto&& s : prod.rhs) {
        if (i == it.dot_index) {
            cout << ".";
        }
        cout << s << " ";
        ++i;
    }
    cout << "]";
    if (i < it.dot_index) {
        cout << "ERROR " << prod << endl;
    }
}

set<item> compute_goto(set<item> I, symbol X, const grammar& g) {
    set<item> goto_set;
    for (auto&& it : I) {
        const auto prod = g[it.production_id];
        auto x_iter = prod.rhs.begin();
        advance(x_iter, it.dot_index);
        if (x_iter != prod.rhs.end() && *x_iter == X) {
            goto_set.insert({it.production_id, it.dot_index+1});
        }
    }
    return compute_closure(goto_set,g);
}
//void print_closure(const map<item, set<item>> closure, const grammar g) {
//    for (auto&& item_and_set : closure) {
//        cout << "{";
//        print_item(item_and_set.first, g);
//        cout << " { ";
//        for (auto& it : item_and_set.second) {
//            print_item(it, g);
//        }
//        cout << " } }" << endl;
//    }
//}


void print_set(const set<item>& c, const grammar& g) {
    for (auto&& it : c) {
        print_item(it,g);
    }
}
set<set<item>> canonical_collection(const grammar& g) {
    auto closure = compute_closure({{0,0,}}, g);
    set<set<item>> canonicalCollection;
    canonicalCollection.insert(closure);
    bool workDone = true;
    while (workDone) {
        workDone = false;
        for (auto&& c : canonicalCollection) {
            for (auto&& X : g.all_symbols()) {
                auto goto_result = compute_goto(c, X, g);
                cout << "goto result of "; print_set(c,g); cout << " and " << X << " is: " << endl;
                print_set(goto_result,g);
                cout << endl;
                if (goto_result.size() > 0
                    && canonicalCollection.find(goto_result) == canonicalCollection.end()) {
                    canonicalCollection.insert(goto_result);
                    workDone = true;
                }
            }
        }
    }
    return canonicalCollection;
}

int main() {
    auto G = read_grammar(cin);
    cout << G << endl;
    auto Gprime = Augment(G);
    cout << Gprime << endl;

    auto c = compute_closure({{0,0}}, Gprime);
    print_set(c,Gprime);

    auto cc = canonical_collection(Gprime);
    for (auto& subset : cc) {
        cout << "NEXT SET" << endl;
        print_set(subset, Gprime);
        cout << endl;
    }
    cout << endl;
}
