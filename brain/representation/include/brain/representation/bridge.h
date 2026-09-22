#pragma once
#include "brain/atomspace/atomspace.h"
#include <string>
namespace brain::representation {
std::string atom_to_narsese(const atomspace::AtomSpace&, atomspace::AtomId);
atomspace::AtomId inheritance(atomspace::AtomSpace&, const std::string&, const std::string&, double frequency=1.0, double confidence=0.9);
}
