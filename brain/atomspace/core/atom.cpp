#include "brain/atomspace/atom.h"
#include <sstream>
#include <utility>
namespace brain::atomspace {
Atom::Atom(AtomId id, AtomType type, TruthValue truth, AtomMetadata metadata)
    : id_(id), type_(type), truth_(truth), metadata_(std::move(metadata)) {}
Node::Node(AtomId id, AtomType type, std::string name, TruthValue truth, AtomMetadata metadata)
    : Atom(id, type, truth, std::move(metadata)), name_(std::move(name)) {}
std::string Node::key() const { return std::to_string(static_cast<TypeId>(type())) + ":N:" + name_; }
Link::Link(AtomId id, AtomType type, std::vector<AtomId> outgoing, TruthValue truth, AtomMetadata metadata)
    : Atom(id, type, truth, std::move(metadata)), outgoing_(std::move(outgoing)) {}
std::string Link::key() const {
    std::ostringstream s; s << static_cast<TypeId>(type()) << ":L:";
    for (AtomId id : outgoing_) s << id << ',';
    return s.str();
}
} // namespace brain::atomspace
