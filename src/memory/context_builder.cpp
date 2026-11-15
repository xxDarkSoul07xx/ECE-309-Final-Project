// src/memory/context_builder.cpp — YOU IMPLEMENT. Spec §4.4.

#include "memory/context_builder.h"
#include "util/min_heap.h"

ContextPlan select_context(const std::vector<ContextItem>& items,
                           std::size_t budget, std::size_t keep_recent) {
    // where does recent window start
    std::size_t recentStart;
    if (keep_recent >= items.size()) {
        recentStart = 0; // everything is recent
    } else {
        recentStart = items.size() - keep_recent;
    }

    // total up tokens
    std::size_t total = 0;
    for (const auto& item : items) total += item.tokens;

    // for which items we are gonna keep
    std::vector<bool> keep(items.size(), true);

    // if everything fits we are done
    if (total <= budget) {
        ContextPlan plan;
        for (std::size_t i = 0; i < items.size(); i++) plan.keep.push_back(i);
        plan.total_tokens = total;
        plan.over_budget = false;
        return plan;
    }

    // push every unprotected item onto the heap keyed on (drop_rank, index), lowest rank = dropped first, oldest index = dropped first among ties
    MinHeap<std::pair<int, std::size_t>> heap;
    for (std::size_t i = 0; i < items.size(); i++) {
        // skip protected items
        if (items[i].role == Role::System) continue;
        if (items[i].pinned) continue;
        if (i >= recentStart) continue;
        heap.push({drop_rank(items[i].role), i});
    }

    // pop and drop until we fit in budget or heap empty
    while (total > budget && !heap.empty()) {
        auto [rank, idx] = heap.pop();
        (void)rank;
        total -= items[idx].tokens;
        keep[idx] = false;
    }

    // build it
    ContextPlan plan;
    for (std::size_t i = 0; i < items.size(); i++) {
        if (keep[i]) plan.keep.push_back(i);
    }
    plan.total_tokens = total;
    plan.over_budget = (total > budget);
    return plan;
}
