// src/policy/policies.cpp — YOU IMPLEMENT. Spec §5.2.

#include "policy/policies.h"

#include <utility>

AllowlistPolicy::AllowlistPolicy(const std::vector<std::string>& programs) {
    programs_ = std::unordered_set<std::string>(programs.begin(), programs.end());
}

Decision AllowlistPolicy::evaluate(const CommandRequest& req) {
    // check if the program is on the allowlist
    if (programs_.count(req.program()) > 0) {
        return Decision::allow("program is allowed", name());
    }
    return Decision::deny("'" + req.program() + "' is not on the allowlist", name());
}

ArgumentPatternPolicy::ArgumentPatternPolicy(std::vector<std::string> patterns)
    : patterns_(std::move(patterns)) {}

Decision ArgumentPatternPolicy::evaluate(const CommandRequest& req) {
    std::string line = req.command_line();
    // check each pattern against the full command line
    for (const auto& p : patterns_) {
        // skip empty patterns
        if (p.empty()) continue;
        if (line.find(p) != std::string::npos) {
            return Decision::deny("command contains forbidden pattern '" + p + "'", name());
        }
    }
    return Decision::allow("no forbidden patterns found", name());
}

ConfirmPolicy::ConfirmPolicy(const std::vector<std::string>& programs) {
    programs_ = std::unordered_set<std::string>(programs.begin(), programs.end());
}

Decision ConfirmPolicy::evaluate(const CommandRequest& req) {
    // ask for confirmation if program is in the list
    if (programs_.count(req.program()) > 0) {
        return Decision::ask("'" + req.program() + "' requires confirmation", name());
    }
    return Decision::allow("no confirmation needed", name());
}

RateLimitPolicy::RateLimitPolicy(std::size_t max_commands, double window_seconds, const Clock& clock)
    : max_(max_commands), window_(window_seconds), clock_(clock) {}

Decision RateLimitPolicy::evaluate(const CommandRequest& req) {
    double now = clock_.now_seconds();

    // drop timestamps that are outside the window
    while (!times_.empty() && now - times_.front() >= window_) {
        times_.pop();
    }

    // deny if at limit
    if (times_.size() >= max_) {
        return Decision::deny("rate limit reached", name());
    }

    // record this request and allow
    times_.push(now);
    return Decision::allow("within rate limit", name());
}

std::size_t RateLimitPolicy::tracked() const noexcept { return times_.size(); }
