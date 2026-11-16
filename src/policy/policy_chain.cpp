// src/policy/policy_chain.cpp — YOU IMPLEMENT. Spec §5.3.

#include "policy/policy_chain.h"

#include <utility>

bool PolicyChain::add(std::unique_ptr<Policy> child) {
    if (child == nullptr) return false;
    children_.push_back(std::move(child));
    return true;
}

std::size_t PolicyChain::size() const noexcept { return children_.size(); }

Decision PolicyChain::evaluate(const CommandRequest& req) {
    // empty chain denies by default
    if (children_.empty()) {
        return Decision::deny("no policies in chain", name());
    }

    bool hasAsk = false;
    Decision firstAsk = Decision::deny("", name()); // placeholder
    // evaluate each child in order
    for (const auto& child : children_) {
        Decision d = child->evaluate(req);
        if (d.kind() == Decision::Kind::Deny) {
            // stop immediately on first deny
            return d;
        }
        if (d.kind() == Decision::Kind::Ask && !hasAsk) {
            firstAsk = d;
            hasAsk = true;
        }
    }

    // return the first ask if there was one
    if (hasAsk) {
        return firstAsk;
    }

    return Decision::allow("all policies passed", name());
}
