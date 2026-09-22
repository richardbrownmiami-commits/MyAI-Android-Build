#include "brain/atomspace/atom.h"
#include <sstream>

namespace brain::atomspace {
Atom::Atom(AtomId id, AtomType type, TruthValue truth) : id_(id), type_(type), truth_(truth) {}
Node::Node(AtomId id, AtomType type, std::string name, TruthValue truth)
    : Atom(id, type, truth), name_(std::move(name)) {}
std::string Node::key() const { return std::to_string(static_cast<TypeId>(type())) + ":N:" + name_; }
Link::Link(AtomId id, AtomType type, std::vector<AtomId> outgoing, TruthValue truth)
    : Atom(id, type, truth), outgoing_(std::move(outgoing)) {}
std::string Link::key() const {
    std::ostringstream s;
    s << static_cast<TypeId>(type()) << ":L:";
    for (AtomId id : outgoing_) s << id << ',';
    return s.str();
}
} // namespace brain::atomspace
