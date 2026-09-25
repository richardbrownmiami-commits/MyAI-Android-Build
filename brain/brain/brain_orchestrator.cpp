#include "brain_orchestrator.h"
#include "ona_bridge.h"
#include "brain/representation/bridge.h"
#include "brain/representation/contradiction.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <utility>
namespace brain {
static std::string trim_copy(std::string s) {
    while(!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while(!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    return s;
}
static std::string lower_copy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return s;
}
static bool strip_suffix(std::string& s, const std::string& suffix) {
    if(s.size() >= suffix.size() && s.compare(s.size()-suffix.size(), suffix.size(), suffix)==0) {
        s.erase(s.size()-suffix.size()); s=trim_copy(s); return true;
    }
    return false;
}
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
    if(text.empty()) return {};
    atomspace::AtomId created=0;
    if(representation::ingest_narsese(space_,text,&created)){
        const auto sentence=representation::atom_to_narsese(space_,created);
        if(nars_ready_){nars::add_narsese(sentence.c_str());nars::cycles(cycles);record_history(sentence);}
        return sentence;
    }
    const auto marker=text.find(" is ");
    if(marker!=std::string::npos&&marker>0&&marker+4<text.size()){
        const auto subject=trim_copy(text.substr(0,marker)), predicate=trim_copy(text.substr(marker+4));
        const auto id=representation::inheritance(space_,subject,predicate);
        const auto sentence=representation::atom_to_narsese(space_,id);
        if(nars_ready_){nars::add_narsese(sentence.c_str());nars::cycles(cycles);record_history(sentence);}
        return sentence;
    }
    if(!nars_ready_) return {};
    nars::add_narsese(text.c_str()); nars::cycles(cycles); record_history(text); return text;
}
std::string BrainOrchestrator::answer_question(const std::string& text) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string q=trim_copy(text);
    while(!q.empty() && (q.back()=='?'||q.back()=='.'||q.back()=='!')) q.pop_back();
    q=trim_copy(q);
    const std::string lower=lower_copy(q);
    auto truth_for = [&](const std::string& subject, const std::string& predicate, int depth, auto&& truth_for_ref) -> bool {
        if(depth<=0) return false;
        auto s=space_.find_node(atomspace::AtomType::ConceptNode,subject);
        auto p=space_.find_node(atomspace::AtomType::ConceptNode,predicate);
        if(!s || !p) return false;
        for(const auto& a:space_.atoms()) {
            auto l=std::dynamic_pointer_cast<atomspace::Link>(a);
            if(!l || l->type()!=atomspace::AtomType::InheritanceLink || l->outgoing().size()!=2) continue;
            if(l->outgoing()[0]==s->id() && l->outgoing()[1]==p->id()) return true;
            if(l->outgoing()[0]==s->id()) {
                auto next=space_.get(l->outgoing()[1]);
                auto n=std::dynamic_pointer_cast<atomspace::Node>(next);
                if(n && truth_for_ref(n->name(),predicate,depth-1,truth_for_ref)) return true;
            }
        }
        return false;
    };
    if(lower.rfind("is ",0)==0 || lower.rfind("are ",0)==0) {
        const auto start=lower.rfind("is ",0)==0 ? 3u : 4u;
        auto remainder=trim_copy(q.substr(start));
        if(remainder.rfind("a ",0)==0) remainder=trim_copy(remainder.substr(2));
        else if(remainder.rfind("an ",0)==0) remainder=trim_copy(remainder.substr(3));
        const auto normalized=lower_copy(remainder);
        const auto pos_a=normalized.find(" a ");
        const auto pos_an=normalized.find(" an ");
        const auto pos=(pos_a!=std::string::npos && (pos_an==std::string::npos || pos_a<pos_an)) ? pos_a : pos_an;
        const auto op_len=(pos==pos_an)?4u:3u;
        if(pos!=std::string::npos) {
            const auto subject=trim_copy(remainder.substr(0,pos));
            const auto predicate=trim_copy(remainder.substr(pos+op_len));
            if(truth_for(subject,predicate,8,truth_for)) return "Yes. "+subject+" is a "+predicate+".";
            return "I don't know yet whether "+subject+" is a "+predicate+".";
        }
    }
    auto answer_definition = [&](std::string subject) -> std::string {
        auto s=space_.find_node(atomspace::AtomType::ConceptNode,subject);
        if(!s) return {};
        for(const auto& a:space_.atoms()) {
            auto l=std::dynamic_pointer_cast<atomspace::Link>(a);
            if(!l || l->type()!=atomspace::AtomType::InheritanceLink || l->outgoing().size()!=2 || l->outgoing()[0]!=s->id()) continue;
            auto p=std::dynamic_pointer_cast<atomspace::Node>(space_.get(l->outgoing()[1]));
            if(p) return subject+" is a "+p->name()+".";
        }
        return {};
    };
    const std::vector<std::string> prefixes={"what is ","what are ","who is ","tell me about "};
    for(const auto& prefix:prefixes) {
        if(lower.rfind(prefix,0)==0) {
            auto subject=trim_copy(q.substr(prefix.size()));
            const auto subject_lower=lower_copy(subject);
            if(subject_lower.rfind("a ",0)==0) subject=trim_copy(subject.substr(2));
            else if(subject_lower.rfind("an ",0)==0) subject=trim_copy(subject.substr(3));
            auto answer=answer_definition(subject);
            if(!answer.empty()) return answer;
            return "I don't know enough about "+subject+" yet.";
        }
    }
    return {};
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
bool BrainOrchestrator::has_explicit_contradiction() const noexcept{std::lock_guard<std::mutex> lock(mutex_); for(const auto& a:space_.atoms()) if(representation::is_explicit_contradiction(space_,a->id())) return true; return false;}
void BrainOrchestrator::set_history_limit(std::size_t limit){std::lock_guard<std::mutex> lock(mutex_);history_limit_=limit;if(narsese_history_.size()>limit)narsese_history_.erase(narsese_history_.begin(),narsese_history_.begin()+(narsese_history_.size()-limit));}
}
