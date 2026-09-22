#include "brain/representation/contradiction.h"
namespace brain::representation {
bool is_explicit_contradiction(const atomspace::AtomSpace& as, atomspace::AtomId id) {
    auto a=as.get(id); return a && a->type()==atomspace::AtomType::NegationLink;
}
}
