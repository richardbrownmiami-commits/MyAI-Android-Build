#pragma once
#include "brain/atomspace/atomspace.h"
#include <string>
#include <vector>

namespace brain {

class BrainOrchestrator {
public:
    BrainOrchestrator();
    std::string remember_inheritance(const std::string& subject, const std::string& predicate, const std::string& source = "android");
    std::string process_text(const std::string& text, int cycles = 2);
    std::string reason(const std::string& narsese, int cycles = 1);
    std::string snapshot_json() const;
    std::size_t atom_count() const noexcept;
private:
    atomspace::AtomSpace space_;
    std::vector<std::string> narsese_history_;
    bool nars_ready_{false};
};

}
