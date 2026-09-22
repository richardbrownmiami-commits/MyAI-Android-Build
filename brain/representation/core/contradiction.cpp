#include "brain/representation/contradiction.h"

namespace brain::representation {
bool is_explicit_contradiction(const atomspace::AtomSpace& space, atomspace::AtomId id) {
    auto atom = space.get(id);
    if (!atom) return false;
    return atom->type() == atomspace::AtomType::NegationLink;
}
}
