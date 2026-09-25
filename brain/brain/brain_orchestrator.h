#pragma once
#include "brain/atomspace/atomspace.h"
#include <cstddef>
#include <mutex>
#include <string>
#include <vector>
namespace brain {
class BrainOrchestrator {
public:
    BrainOrchestrator();
    ~BrainOrchestrator();
    std::string remember_inheritance(const std::string& subject, const std::string& predicate, const std::string& source = "android");
    std::string process_text(const std::string& text, int cycles = 2);
    std::string answer_question(const std::string& text) const;
    std::string reason(const std::string& narsese, int cycles = 1);
    std::string snapshot_json() const;
    std::size_t atom_count() const noexcept;
    std::size_t history_count() const noexcept;
    bool has_explicit_contradiction() const noexcept;
    void set_history_limit(std::size_t limit);
private:
    atomspace::AtomSpace space_;
    std::vector<std::string> narsese_history_;
    bool nars_ready_{false};
    std::size_t history_limit_{4096};
    mutable std::mutex mutex_;
    void record_history(const std::string& value);
};
}
