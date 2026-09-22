#pragma once
#include "brain/atomspace/atomspace.h"
#include <string>

namespace brain {

class BrainOrchestrator {
public:
    BrainOrchestrator();
    std::string remember_inheritance(const std::string& subject, const std::string& predicate, const std::string& source = "android");
    std::string reason(const std::string& narsese, int cycles = 1);
    std::size_t atom_count() const noexcept;
private:
    atomspace::AtomSpace space_;
    bool nars_ready_{false};
};

}
