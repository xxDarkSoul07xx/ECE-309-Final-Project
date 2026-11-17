// src/skills/skill_index.cpp — YOU IMPLEMENT. Spec §6.3.

#include "skills/skill_index.h"
#include "util/min_heap.h"
#include "util/text.h"
#include <unordered_map>

void SkillIndex::add(std::size_t doc_id, std::string_view text) {
    // get unique tokens so each word posts this doc only once
    for (const auto& token : unique_tokens(text)) {
        postings_[token].push_back(doc_id);
    }
}

const std::vector<std::size_t>* SkillIndex::postings(std::string_view word) const {
    auto it = postings_.find(std::string(word));
    if (it == postings_.end()) return nullptr;
    return &it->second;
}

std::size_t SkillIndex::word_count() const noexcept {
    return postings_.size();
}

std::vector<std::size_t> SkillIndex::search(std::string_view query, std::size_t k) const {
    // count how many query words each doc matches
    std::unordered_map<std::size_t, std::size_t> score;
    for (const auto& token : unique_tokens(query)) {
        auto it = postings_.find(token);
        if (it == postings_.end()) continue;
        for (std::size_t doc_id : it->second) {
            score[doc_id]++;
        }
    }

    // build the pairs
    std::vector<std::pair<std::size_t, std::size_t>> pairs;
    for (const auto& kv : score) {
        pairs.push_back({kv.second, kv.first});
    }

    // compare
    auto worse = [](const std::pair<std::size_t, std::size_t>& a, const std::pair<std::size_t, std::size_t>& b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second > b.second;
    };

    auto best = top_k(pairs, k, worse);

    // extract just the doc_ids
    std::vector<std::size_t> result;
    for (const auto& p : best) {
        result.push_back(p.second);
    }
    return result;
}