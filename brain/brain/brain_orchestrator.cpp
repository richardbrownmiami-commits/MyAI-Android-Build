#include "brain_orchestrator.h"
#include "ona_bridge.h"
#include "brain/representation/bridge.h"
#include "brain/representation/contradiction.h"
#include <algorithm>
#include <android/log.h>
#include <cctype>
#include <sstream>
#include <utility>

namespace brain {
namespace {
static constexpr const char* TAG="MyAI-Native";

static std::string trim_copy(std::string s){
    while(!s.empty()&&std::isspace(static_cast<unsigned char>(s.front())))s.erase(s.begin());
    while(!s.empty()&&std::isspace(static_cast<unsigned char>(s.back())))s.pop_back();
    return s;
}
static std::string lower_copy(std::string s){
    std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    return s;
}
static std::string clean_term(std::string s){
    s=trim_copy(s);
    while(!s.empty()&&(s.back()=='.'||s.back()=='?'||s.back()=='!'||s.back()==','))s.pop_back();
    s=trim_copy(s);
    std::string lower=lower_copy(s);
    const char* articles[]={"a ","an ","the ","all "};
    for(const char* article:articles){
        const std::string prefix(article);
        if(lower.rfind(prefix,0)==0){s=trim_copy(s.substr(prefix.size()));break;}
    }
    return lower_copy(s);
}
static bool parse_statement(const std::string& text,std::string& subject,std::string& predicate,bool& negative){
    std::string q=trim_copy(text);
    while(!q.empty()&&(q.back()=='.'||q.back()=='!'||q.back()=='?'))q.pop_back();
    const std::string lower=lower_copy(q);
    std::size_t pos=lower.find(" is ");
    std::size_t width=4;
    if(pos==std::string::npos){pos=lower.find(" are ");width=5;}
    if(pos==std::string::npos||pos==0||pos+width>=q.size())return false;
    subject=clean_term(q.substr(0,pos));
    std::string right=trim_copy(q.substr(pos+width));
    std::string right_lower=lower_copy(right);
    negative=false;
    if(right_lower.rfind("not ",0)==0){negative=true;right=trim_copy(right.substr(4));}
    predicate=clean_term(right);
    return !subject.empty()&&!predicate.empty();
}
static bool parse_question(const std::string& text,std::string& subject,std::string& predicate){
    std::string q=trim_copy(text);
    while(!q.empty()&&(q.back()=='?'||q.back()=='.'||q.back()=='!'))q.pop_back();
    const std::string lower=lower_copy(q);
    std::size_t start=std::string::npos;
    if(lower.rfind("is ",0)==0)start=3;
    else if(lower.rfind("are ",0)==0)start=4;
    if(start==std::string::npos)return false;
    std::string remainder=trim_copy(q.substr(start));
    const std::string rem_lower=lower_copy(remainder);
    if(rem_lower.rfind("a ",0)==0)remainder=trim_copy(remainder.substr(2));
    else if(rem_lower.rfind("an ",0)==0)remainder=trim_copy(remainder.substr(3));
    const std::string normalized=lower_copy(remainder);
    std::size_t p=normalized.find(" a ");
    std::size_t p2=normalized.find(" an ");
    if(p==std::string::npos||(p2!=std::string::npos&&p2<p))p=p2;
    std::size_t width=(p2!=std::string::npos&&p==p2)?4:3;
    if(p==std::string::npos)return false;
    subject=clean_term(remainder.substr(0,p));
    predicate=clean_term(remainder.substr(p+width));
    return !subject.empty()&&!predicate.empty();
}
static bool direct_relation(const atomspace::AtomSpace& space,const std::string& subject,const std::string& predicate,atomspace::AtomType type){
    auto s=space.find_node(atomspace::AtomType::ConceptNode,subject);
    auto p=space.find_node(atomspace::AtomType::ConceptNode,predicate);
    if(!s||!p)return false;
    for(const auto& a:space.atoms()){
        auto l=std::dynamic_pointer_cast<atomspace::Link>(a);
        if(l&&l->type()==type&&l->outgoing().size()==2&&l->outgoing()[0]==s->id()&&l->outgoing()[1]==p->id())return true;
    }
    return false;
}
static bool inheritance_truth(const atomspace::AtomSpace& space,const std::string& subject,const std::string& predicate,int depth){
    if(depth<=0)return false;
    if(direct_relation(space,subject,predicate,atomspace::AtomType::InheritanceLink))return true;
    auto s=space.find_node(atomspace::AtomType::ConceptNode,subject);
    if(!s)return false;
    for(const auto& a:space.atoms()){
        auto l=std::dynamic_pointer_cast<atomspace::Link>(a);
        if(!l||l->type()!=atomspace::AtomType::InheritanceLink||l->outgoing().size()!=2||l->outgoing()[0]!=s->id())continue;
        auto next=std::dynamic_pointer_cast<atomspace::Node>(space.get(l->outgoing()[1]));
        if(next&&inheritance_truth(space,next->name(),predicate,depth-1))return true;
    }
    return false;
}
static bool negative_relation(const atomspace::AtomSpace& space,const std::string& subject,const std::string& predicate){
    return direct_relation(space,subject,predicate,atomspace::AtomType::NegationLink);
}
static std::string json_escape(const std::string& s){
    std::string out;
    for(char c:s){
        switch(c){case '\\':out+="\\\\\";break;case '"':out+="\\\"";break;case '\n':out+="\\n";break;case '\r':out+="\\r";break;case '\t':out+="\\t";break;default:out+=c;}
    }
    return out;
}
}

BrainOrchestrator::BrainOrchestrator(){nars::init();nars_ready_=true;__android_log_print(ANDROID_LOG_INFO,TAG,"ONA initialized");}
BrainOrchestrator::~BrainOrchestrator()=default;

void BrainOrchestrator::record_history(const std::string& value){
    narsese_history_.push_back(value);
    if(narsese_history_.size()>history_limit_)
        narsese_history_.erase(narsese_history_.begin(),narsese_history_.begin()+(narsese_history_.size()-history_limit_));
}

std::string BrainOrchestrator::remember_inheritance(const std::string& subject,const std::string& predicate,const std::string& source){
    std::lock_guard<std::mutex> lock(mutex_);
    const std::string s=clean_term(subject),p=clean_term(predicate);
    if(s.empty()||p.empty())return {};
    atomspace::AtomMetadata metadata;metadata.source=source;
    const auto id=representation::inheritance(space_,s,p,1.0,0.9,metadata);
    const auto sentence=representation::atom_to_narsese(space_,id);
    if(nars_ready_){nars::add_narsese(sentence.c_str());nars::cycles(1);record_history(sentence);}
    __android_log_print(ANDROID_LOG_DEBUG,TAG,"learned %s",sentence.c_str());
    return sentence;
}

std::string BrainOrchestrator::process_text(const std::string& text,int cycles){
    std::lock_guard<std::mutex> lock(mutex_);
    if(text.empty())return {};
    atomspace::AtomId created=0;
    if(representation::ingest_narsese(space_,text,&created)){
        const auto sentence=representation::atom_to_narsese(space_,created);
        if(nars_ready_){nars::add_narsese(sentence.c_str());nars::cycles(std::clamp(cycles,1,10000));record_history(sentence);}
        return sentence;
    }
    std::string subject,predicate;bool negative=false;
    if(parse_statement(text,subject,predicate,negative)){
        atomspace::AtomMetadata metadata;metadata.source="conversation";
        created=negative?representation::negation(space_,subject,predicate,metadata):representation::inheritance(space_,subject,predicate,1.0,0.9,metadata);
        const auto sentence=representation::atom_to_narsese(space_,created);
        if(nars_ready_){nars::add_narsese(sentence.c_str());nars::cycles(std::clamp(cycles,1,10000));record_history(sentence);}
        __android_log_print(ANDROID_LOG_DEBUG,TAG,"parsed statement %s",sentence.c_str());
        return sentence;
    }
    if(!nars_ready_)return {};
    nars::add_narsese(text.c_str());nars::cycles(std::clamp(cycles,1,10000));record_history(text);
    return text;
}

std::string BrainOrchestrator::answer_question(const std::string& text){
    std::lock_guard<std::mutex> lock(mutex_);
    std::string subject,predicate;
    if(!parse_question(text,subject,predicate))return {};
    const std::string query="<"+subject+" --> "+predicate+">?";
    if(nars_ready_){nars::add_narsese(query.c_str());nars::cycles(8);record_history(query);}
    if(negative_relation(space_,subject,predicate))
        return "No. I have explicit evidence that "+subject+" is not a "+predicate+".";
    if(inheritance_truth(space_,subject,predicate,8))
        return "Yes. I have evidence that "+subject+" is a "+predicate+".";
    return "I don't know yet whether "+subject+" is a "+predicate+".";
}

std::string BrainOrchestrator::reason(const std::string& narsese,int cycles){
    std::lock_guard<std::mutex> lock(mutex_);
    if(!nars_ready_||narsese.empty())return {};
    nars::add_narsese(narsese.c_str());nars::cycles(std::clamp(cycles,1,10000));record_history(narsese);return narsese;
}

std::string BrainOrchestrator::snapshot_json() const{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream out;out<<"{\"version\":3,\"atoms\":[";
    bool first=true;
    for(const auto& atom:space_.atoms()){
        if(!first)out<<",";first=false;
        if(auto node=std::dynamic_pointer_cast<atomspace::Node>(atom)){
            out<<"{\"id\":"<<node->id()<<",\"kind\":\"node\",\"type\":"<<static_cast<int>(node->type())
               <<",\"name\":\""<<json_escape(node->name())<<"\",\"frequency\":"<<node->truth().frequency
               <<",\"confidence\":"<<node->truth().confidence<<"}";
        }else if(auto link=std::dynamic_pointer_cast<atomspace::Link>(atom)){
            out<<"{\"id\":"<<link->id()<<",\"kind\":\"link\",\"type\":"<<static_cast<int>(link->type())<<",\"outgoing\":[";
            for(std::size_t i=0;i<link->outgoing().size();++i){if(i)out<<",";out<<link->outgoing()[i];}
            out<<"],\"frequency\":"<<link->truth().frequency<<",\"confidence\":"<<link->truth().confidence
               <<",\"source\":\""<<json_escape(link->metadata().source)<<"\",\"timestamp_ms\":"<<link->metadata().timestamp_ms
               <<",\"context\":\""<<json_escape(link->metadata().context)<<"\"}";
        }
    }
    out<<"],\"narsese\":[";
    for(std::size_t i=0;i<narsese_history_.size();++i){if(i)out<<",";out<<"\""<<json_escape(narsese_history_[i])<<"\"";}
    out<<"]}";
    return out.str();
}
std::size_t BrainOrchestrator::atom_count() const noexcept{std::lock_guard<std::mutex> lock(mutex_);return space_.size();}
std::size_t BrainOrchestrator::history_count() const noexcept{std::lock_guard<std::mutex> lock(mutex_);return narsese_history_.size();}
bool BrainOrchestrator::has_explicit_contradiction() const noexcept{std::lock_guard<std::mutex> lock(mutex_);for(const auto& a:space_.atoms())if(representation::is_explicit_contradiction(space_,a->id()))return true;return false;}
void BrainOrchestrator::set_history_limit(std::size_t limit){std::lock_guard<std::mutex> lock(mutex_);history_limit_=limit;if(narsese_history_.size()>limit)narsese_history_.erase(narsese_history_.begin(),narsese_history_.begin()+(narsese_history_.size()-limit));}
}
