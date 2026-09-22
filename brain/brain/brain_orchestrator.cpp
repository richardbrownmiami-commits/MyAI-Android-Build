#include "brain_orchestrator.h"
#include "ona_bridge.h"
#include "brain/representation/bridge.h"
#include <sstream>
#include <utility>
namespace brain {
BrainOrchestrator::BrainOrchestrator(){ nars::init(); nars_ready_=true; }
BrainOrchestrator::~BrainOrchestrator() = default;
void BrainOrchestrator::record_history(const std::string& value){
    narsese_history_.push_back(value);
    if(narsese_history_.size()>history_limit_)
        narsese_history_.erase(narsese_history_.begin(), narsese_history_.begin() + (narsese_history_.size()-history_limit_));
}
std::string BrainOrchestrator::remember_inheritance(const std::string& subject,const std::string& predicate,const std::string& source){
    std::lock_guard<std::mutex> lock(mutex_);
    atomspace::AtomMetadata metadata; metadata.source=source;
    const auto id=representation::inheritance(space_,subject,predicate,1.0,0.9,metadata);
    const auto sentence=representation::atom_to_narsese(space_,id);
    if(nars_ready_){ nars::add_narsese(sentence.c_str()); nars::cycles(1); record_history(sentence); }
    return sentence;
}
std::string BrainOrchestrator::process_text(const std::string& text,int cycles){
    std::lock_guard<std::mutex> lock(mutex_);
    const auto marker=text.find(" is ");
    if(marker!=std::string::npos&&marker>0&&marker+4<text.size()){
        const auto subject=text.substr(0,marker), predicate=text.substr(marker+4);
        const auto id=representation::inheritance(space_,subject,predicate);
        const auto sentence=representation::atom_to_narsese(space_,id);
        if(nars_ready_){nars::add_narsese(sentence.c_str());nars::cycles(1);record_history(sentence);}
        return sentence;
    }
    const auto open=text.find('<'), arrow=text.find(" --> "), close=text.find('>');
    if(open!=std::string::npos&&arrow!=std::string::npos&&close!=std::string::npos&&arrow>open+1&&close>arrow+5){
        const auto subject=text.substr(open+1,arrow-open-1), predicate=text.substr(arrow+5,close-arrow-5);
        if(!subject.empty()&&!predicate.empty()){
            const auto sentence=representation::atom_to_narsese(space_,representation::inheritance(space_,subject,predicate));
            if(nars_ready_){nars::add_narsese(sentence.c_str());nars::cycles(1);record_history(sentence);}
            return sentence;
        }
    }
    if(!nars_ready_||text.empty()) return {};
    nars::add_narsese(text.c_str()); nars::cycles(cycles); record_history(text); return text;
}
std::string BrainOrchestrator::reason(const std::string& narsese,int cycles){
    std::lock_guard<std::mutex> lock(mutex_);
    if(!nars_ready_||narsese.empty()) return {};
    nars::add_narsese(narsese.c_str()); nars::cycles(cycles); record_history(narsese); return narsese;
}
std::string BrainOrchestrator::snapshot_json() const{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream out; out<<"{\"version\":2,\"atoms\":[";
    bool first=true;
    for(const auto& atom:space_.atoms()){
        if(!first)out<<","; first=false;
        if(auto node=std::dynamic_pointer_cast<atomspace::Node>(atom)){
            out<<"{\"id\":"<<node->id()<<",\"kind\":\"node\",\"type\":"<<static_cast<int>(node->type())
               <<",\"name\":\""<<node->name()<<"\",\"frequency\":"<<node->truth().frequency
               <<",\"confidence\":"<<node->truth().confidence<<"}";
        } else if(auto link=std::dynamic_pointer_cast<atomspace::Link>(atom)){
            out<<"{\"id\":"<<link->id()<<",\"kind\":\"link\",\"type\":"<<static_cast<int>(link->type())<<",\"outgoing\":[";
            for(size_t i=0;i<link->outgoing().size();++i){if(i)out<<",";out<<link->outgoing()[i];}
            out<<"],\"frequency\":"<<link->truth().frequency<<",\"confidence\":"<<link->truth().confidence
               <<",\"source\":\""<<link->metadata().source<<"\",\"timestamp_ms\":"<<link->metadata().timestamp_ms
               <<",\"context\":\""<<link->metadata().context<<"\"}";
        }
    }
    out<<"],\"narsese\":[";
    for(size_t i=0;i<narsese_history_.size();++i){if(i)out<<",";out<<"\""<<narsese_history_[i]<<"\"";}
    out<<"]}";
    return out.str();
}
std::size_t BrainOrchestrator::atom_count() const noexcept{std::lock_guard<std::mutex> lock(mutex_);return space_.size();}
std::size_t BrainOrchestrator::history_count() const noexcept{std::lock_guard<std::mutex> lock(mutex_);return narsese_history_.size();}
void BrainOrchestrator::set_history_limit(std::size_t limit){std::lock_guard<std::mutex> lock(mutex_);history_limit_=limit;if(narsese_history_.size()>limit)narsese_history_.erase(narsese_history_.begin(),narsese_history_.begin()+(narsese_history_.size()-limit));}
}
