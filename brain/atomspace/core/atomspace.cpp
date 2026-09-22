#include "brain/atomspace/atomspace.h"
#include <algorithm>

namespace brain::atomspace {
std::shared_ptr<Node> AtomSpace::add_node(AtomType type, const std::string& name, TruthValue truth) {
    auto probe = Node(0, type, name);
    auto it = key_index_.find(probe.key());
    if (it != key_index_.end()) return std::static_pointer_cast<Node>(atoms_.at(it->second));
    auto n = std::make_shared<Node>(next_id_++, type, name, truth);
    key_index_[n->key()] = n->id();
    atoms_[n->id()] = n;
    return n;
}
std::shared_ptr<Link> AtomSpace::add_link(AtomType type, const std::vector<AtomId>& outgoing, TruthValue truth) {
    auto probe = Link(0, type, outgoing);
    auto it = key_index_.find(probe.key());
    if (it != key_index_.end()) return std::static_pointer_cast<Link>(atoms_.at(it->second));
    auto l = std::make_shared<Link>(next_id_++, type, outgoing, truth);
    key_index_[l->key()] = l->id();
    atoms_[l->id()] = l;
    return l;
}
std::shared_ptr<Atom> AtomSpace::get(AtomId id) const {
    auto it = atoms_.find(id); return it == atoms_.end() ? nullptr : it->second;
}
std::shared_ptr<Node> AtomSpace::find_node(AtomType type, const std::string& name) const {
    Node probe(0, type, name);
    auto it = key_index_.find(probe.key());
    if (it == key_index_.end()) return nullptr;
    return std::static_pointer_cast<Node>(atoms_.at(it->second));
}
std::vector<std::shared_ptr<Atom>> AtomSpace::atoms() const {
    std::vector<std::shared_ptr<Atom>> out; out.reserve(atoms_.size());
    for (const auto& p : atoms_) out.push_back(p.second);
    std::sort(out.begin(), out.end(), [](const auto& a,const auto& b){return a->id()<b->id();});
    return out;
}
void AtomSpace::clear() { atoms_.clear(); key_index_.clear(); next_id_=1; }
} // namespace brain::atomspace
