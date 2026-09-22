#pragma once
#include "brain/atomspace/atomspace.h"

namespace brain::representation {
bool is_explicit_contradiction(const atomspace::AtomSpace& space, atomspace::AtomId id);
}
