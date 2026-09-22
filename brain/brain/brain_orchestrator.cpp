#include "brain_orchestrator.h"
#include "brain/nars/ona_bridge.h"
#include "brain/representation/bridge.h"
#include <sstream>

namespace brain {

BrainOrchestrator::BrainOrchestrator() {
    nars::init();
    nars_ready_ = true;
}

std::string BrainOrchestrator::remember_inheritance(const std::string& subject, const std::string& predicate, const std::string&) {
    const auto id = representation::inheritance(space_, subject, predicate);
    const auto sentence = representation::atom_to_narsese(space_, id);
    if (nars_ready_) {
        nars::add_narsese(sentence.c_str());
        nars::cycles(1);
        narsese_history_.push_back(sentence);
    }
    return sentence;
}

std::string BrainOrchestrator::process_text(const std::string& text, int cycles) {
    const auto marker = text.find(" is ");
    if (marker != std::string::npos && marker > 0 && marker + 4 < text.size()) {
        return remember_inheritance(text.substr(0, marker), text.substr(marker + 4));
    }

    const auto open = text.find('<');
    const auto arrow = text.find(" --> ");
    const auto close = text.find('>');
    if (open != std::string::npos && arrow != std::string::npos && close != std::string::npos && arrow > open + 1 && close > arrow + 5) {
        auto subject = text.substr(open + 1, arrow - open - 1);
        auto predicate = text.substr(arrow + 5, close - arrow - 5);
        if (!subject.empty() && !predicate.empty()) {
            const auto sentence = remember_inheritance(subject, predicate);
            return sentence;
        }
    }

    return reason(text, cycles);
}

std::string BrainOrchestrator::reason(const std::string& narsese, int cycles) {
    if (!nars_ready_ || narsese.empty()) return {};
    nars::add_narsese(narsese.c_str());
    nars::cycles(cycles);
    narsese_history_.push_back(narsese);
    return narsese;
}

std::string BrainOrchestrator::snapshot_json() const {
    std::ostringstream out;
    out << "{\"version\":1,\"atoms\":[";
    bool first = true;
    for (const auto& atom : space_.atoms()) {
        if (!first) out << ",";
        first = false;
        if (auto node = std::dynamic_pointer_cast<atomspace::Node>(atom)) {
            out << "{\"id\":" << node->id()
                << ",\"kind\":\"node\",\"type\":" << static_cast<int>(node->type())
                << ",\"name\":\"" << node->name() << "\""
                << ",\"frequency\":" << node->truth().frequency
                << ",\"confidence\":" << node->truth().confidence << "}";
        }
    }
    out << "],\"narsese\":[";
    for (std::size_t i = 0; i < narsese_history_.size(); ++i) {
        if (i) out << ",";
        out << "\"" << narsese_history_[i] << "\"";
    }
    out << "]}";
    return out.str();
}

std::size_t BrainOrchestrator::atom_count() const noexcept {
    return space_.size();
}

}
