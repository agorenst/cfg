#include "cfg.h"

#include <map>
#include <set>

std::map<cfg::symbol, std::set<cfg::symbol>> compute_first(const cfg::grammar& g);
std::map<cfg::symbol, std::set<cfg::symbol>> compute_follow(const cfg::grammar& g, bool Scott = false);
std::map<cfg::production, std::set<cfg::symbol>> compute_predict(const cfg::grammar& g);
std::pair<cfg::production, cfg::production> compute_predict_predict_conflict(const cfg::grammar& g);
