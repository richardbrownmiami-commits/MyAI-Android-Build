#include "brain/brain/brain_orchestrator.h"
#include "brain/nars/ona_bridge.h"
#include "brain/representation/bridge.h"

namespace brain {

BrainOrchestrator::BrainOrchestrator() {
    nars::init();
    nars_ready_ = true;
}

std::string BrainOrchestrator::remember_inheritance(const std::string& subject, const std::string& predicate, const std::string&) {
    const auto id = representation::inheritance(space_, subject, predicate);
    return representation::atom_to_narsese(space_, id);
}

std::string BrainOrchestrator::reason(const std::string& narsese, int cycles) {
    if (!nars_ready_) return {};
    nars::add_narsese(narsese.c_str());
    nars::cycles(cycles);
    return narsese;
}

std::size_t BrainOrchestrator::atom_count() const noexcept {
    return space_.size();
}

}
