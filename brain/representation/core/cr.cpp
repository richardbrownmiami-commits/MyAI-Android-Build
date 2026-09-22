#include "brain/representation/cr.h"
#include "brain/representation/bridge.h"

namespace brain::representation {

std::string to_narsese(const atomspace::AtomSpace& space, atomspace::AtomId id) {
    return atom_to_narsese(space, id);
}

bool project_to_nars(const CognitiveFact& fact, const atomspace::AtomSpace& space, std::string& out) {
    out = atom_to_narsese(space, fact.atom_id);
    return !out.empty();
}

CognitiveFact make_fact(atomspace::AtomId id, atomspace::TruthValue truth, std::string source, std::int64_t timestamp_ms, std::string context) {
    CognitiveFact fact;
    fact.atom_id = id;
    fact.truth = truth;
    fact.provenance = {std::move(source), timestamp_ms, std::move(context)};
    return fact;
}

}
