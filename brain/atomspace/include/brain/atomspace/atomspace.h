#pragma once
#include "brain/atomspace/atom.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace brain::atomspace {

class AtomSpace {
public:
    AtomSpace() = default;
    std::shared_ptr<Node> add_node(AtomType type, const std::string& name, TruthValue truth = {});
    std::shared_ptr<Link> add_link(AtomType type, const std::vector<AtomId>& outgoing, TruthValue truth = {});
    std::shared_ptr<Atom> get(AtomId id) const;
    std::shared_ptr<Node> find_node(AtomType type, const std::string& name) const;
    std::vector<std::shared_ptr<Atom>> atoms() const;
    std::size_t size() const noexcept { return atoms_.size(); }
    void clear();
private:
    AtomId next_id_{1};
    std::unordered_map<AtomId, std::shared_ptr<Atom>> atoms_;
    std::unordered_map<std::string, AtomId> key_index_;
};

} // namespace brain::atomspace
