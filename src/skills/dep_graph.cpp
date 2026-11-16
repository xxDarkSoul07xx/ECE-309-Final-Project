// src/skills/dep_graph.cpp — YOU IMPLEMENT. Spec §6.2.

#include "skills/dep_graph.h"
#include <algorithm>

bool DepGraph::add_node(const std::string& name) {
    // don't add duplicates
    if (requires_.count(name) > 0) return false;
    requires_[name] = {};
    return true;
}

bool DepGraph::add_edge(const std::string& from, const std::string& to) {
    // from must be a node
    if (requires_.count(from) == 0) return false;
    // check if edge already exists
    auto& reqs = requires_[from];
    for (const auto& r : reqs) {
        if (r == to) return false;
    }
    reqs.push_back(to);
    // fill reverse map for BFS
    required_by_[to].push_back(from);
    return true;
}

bool DepGraph::has_node(const std::string& name) const {
    return requires_.count(name) > 0;
}

std::size_t DepGraph::node_count() const noexcept {
    return requires_.size();
}

const std::vector<std::string>& DepGraph::requirements(const std::string& name) const {
    auto it = requires_.find(name);
    static const std::vector<std::string> kNone;
    if (it == requires_.end()) return kNone;
    return it->second;
}

bool DepGraph::order_from(const std::string& root, std::vector<std::string>* out,
                          std::string* error) const {
    // check root exists
    if (requires_.count(root) == 0) {
        *error = "unknown: " + root;
        return false;
    }

    // 0 = white 1 = gray 2 = black
    std::unordered_map<std::string, int> color;
    std::vector<std::string> path;
    out->clear();
    return dfs(root, color, path, out, error);
}

std::vector<std::string> DepGraph::dependents(const std::string& name) const {
    std::vector<std::string> result;
    // mark name as seen so it won't appear in results
    std::unordered_map<std::string, bool> seen;
    seen[name] = true;

    // BF
    std::vector<std::string> current;

    auto it = required_by_.find(name);
    if (it != required_by_.end()) {
        for (const auto& dep : it->second) {
            if (!seen[dep]) {
                seen[dep] = true;
                current.push_back(dep);
            }
        }
    }

    // sort each level alphabetically before adding to result
    while (!current.empty()) {
        std::sort(current.begin(), current.end());
        for (const auto& dep : current) {
            result.push_back(dep);
        }
        // build next level
        std::vector<std::string> next;
        for (const auto& dep : current) {
            auto jt = required_by_.find(dep);
            if (jt != required_by_.end()) {
                for (const auto& d2 : jt->second) {
                    if (!seen[d2]) {
                        seen[d2] = true;
                        next.push_back(d2);
                    }
                }
            }
        }
        current = next;
    }

    return result;
}


bool DepGraph::dfs(const std::string& node,
                   std::unordered_map<std::string, int>& color,
                   std::vector<std::string>& path,
                   std::vector<std::string>* out,
                   std::string* error) const {
    // mark gray and push onto path
    color[node] = 1;
    path.push_back(node);

    auto it = requires_.find(node);
    if (it != requires_.end()) {
        for (const auto& req : it->second) {
            int c = 0;
            auto cit = color.find(req);
            if (cit != color.end()) c = cit->second;

            if (c == 0) {
                // white
                if (requires_.count(req) == 0) {
                    *error = "missing: " + req + " (required by " + node + ")";
                    return false;
                }
                if (!dfs(req, color, path, out, error)) return false;
            } else if (c == 1) {
                // gray
                std::string cycle = "";
                bool found = false;
                for (const auto& p : path) {
                    if (p == req) found = true;
                    if (found) cycle += p + " -> ";
                }
                cycle += req;
                *error = "cycle: " + cycle;
                return false;
            }
            // black
        }
    }

    // mark black
    color[node] = 2;
    path.pop_back();
    out->push_back(node);
    return true;
}
