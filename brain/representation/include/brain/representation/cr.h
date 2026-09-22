#pragma once
#include "brain/atomspace/atomspace.h"
#include <cstdint>
#include <string>

namespace brain::representation {

struct Provenance {
    std::string source;
    std::int64_t timestamp_ms{0};
    std::string context;
};

struct CognitiveFact {
    atomspace::AtomId atom_id{0};
    atomspace::TruthValue truth{};
    Provenance provenance;
    bool contradiction{false};
};

std::string to_narsese(const atomspace::AtomSpace& space, atomspace::AtomId id);
bool project_to_nars(const CognitiveFact& fact, const atomspace::AtomSpace& space, std::string& out);
CognitiveFact make_fact(atomspace::AtomId id, atomspace::TruthValue truth, std::string source, std::int64_t timestamp_ms, std::string context = {});
}
