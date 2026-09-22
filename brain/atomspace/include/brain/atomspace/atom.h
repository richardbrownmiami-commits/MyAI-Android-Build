#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace brain::atomspace {

using AtomId = std::uint64_t;
using TypeId = std::uint16_t;

enum class AtomType : TypeId {
    Node = 1, Link = 2, ConceptNode = 3, PredicateNode = 4, NumberNode = 5,
    InheritanceLink = 100, SimilarityLink = 101, ImplicationLink = 102,
    EvaluationLink = 103, NegationLink = 104
};

struct TruthValue { double frequency{1.0}; double confidence{0.0}; };

struct AtomMetadata {
    std::string source{"android"};
    std::int64_t timestamp_ms{0};
    std::string context;
};

class Atom {
public:
    Atom(AtomId id, AtomType type, TruthValue truth = {}, AtomMetadata metadata = {});
    virtual ~Atom() = default;
    AtomId id() const noexcept { return id_; }
    AtomType type() const noexcept { return type_; }
    const TruthValue& truth() const noexcept { return truth_; }
    void set_truth(TruthValue v) noexcept { truth_ = v; }
    const AtomMetadata& metadata() const noexcept { return metadata_; }
    void set_metadata(AtomMetadata v) { metadata_ = std::move(v); }
    virtual bool is_node() const noexcept = 0;
    virtual std::string key() const = 0;
private:
    AtomId id_; AtomType type_; TruthValue truth_; AtomMetadata metadata_;
};

class Node final : public Atom {
public:
    Node(AtomId id, AtomType type, std::string name, TruthValue truth = {}, AtomMetadata metadata = {});
    bool is_node() const noexcept override { return true; }
    const std::string& name() const noexcept { return name_; }
    std::string key() const override;
private: std::string name_;
};

class Link final : public Atom {
public:
    Link(AtomId id, AtomType type, std::vector<AtomId> outgoing, TruthValue truth = {}, AtomMetadata metadata = {});
    bool is_node() const noexcept override { return false; }
    const std::vector<AtomId>& outgoing() const noexcept { return outgoing_; }
    std::string key() const override;
private: std::vector<AtomId> outgoing_;
};

} // namespace brain::atomspace
