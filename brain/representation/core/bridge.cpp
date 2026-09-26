#include "brain/representation/bridge.h"
#include "brain/representation/contradiction.h"
#include <sstream>
namespace brain::representation {
using namespace atomspace;
static AtomId binary(AtomSpace& as, AtomType type, const std::string& a, const std::string& b, const AtomMetadata& m, TruthValue tv={1.0,0.9}) {
    auto x=as.add_node(AtomType::ConceptNode,a,tv), y=as.add_node(AtomType::ConceptNode,b,tv);
    auto l=as.add_link(type,{x->id(),y->id()},tv); l->set_metadata(m); return l->id();
}
std::string atom_to_narsese(const AtomSpace& as, AtomId id) {
    auto a=as.get(id); if(!a) return {};
    if(auto n=std::dynamic_pointer_cast<Node>(a)) return n->name();
    auto l=std::dynamic_pointer_cast<Link>(a); if(!l || l->outgoing().size()!=2) return {};
    auto left=atom_to_narsese(as,l->outgoing()[0]), right=atom_to_narsese(as,l->outgoing()[1]);
    if(left.empty()||right.empty()) return {};
    switch(l->type()){
      case AtomType::InheritanceLink: return "<"+left+" --> "+right+">.";
      case AtomType::SimilarityLink: return "<"+left+" <-> "+right+">.";
      case AtomType::ImplicationLink: return "<"+left+" ==> "+right+">.";
      case AtomType::EvaluationLink: return "<"+left+" --> "+right+">.";
      case AtomType::NegationLink: return "<"+left+" --> (--)"+right+">.";
      default: return {};
    }
}
AtomId inheritance(AtomSpace& as,const std::string&a,const std::string&b,double f,double c,const AtomMetadata&m){return binary(as,AtomType::InheritanceLink,a,b,m,{f,c});}
AtomId similarity(AtomSpace& as,const std::string&a,const std::string&b,const AtomMetadata&m){return binary(as,AtomType::SimilarityLink,a,b,m);}
AtomId implication(AtomSpace& as,const std::string&a,const std::string&b,const AtomMetadata&m){return binary(as,AtomType::ImplicationLink,a,b,m);}
AtomId negation(AtomSpace& as,const std::string&a,const std::string&b,const AtomMetadata&m){return binary(as,AtomType::NegationLink,a,b,m);}
bool ingest_narsese(AtomSpace& as,const std::string&s,AtomId*created) {
    auto t=s; while(!t.empty()&&(t.back()=='.'||t.back()==' '||t.back()=='\n'||t.back()=='\r')) t.pop_back();
    if(t.size()<7||t.front()!='<'||t.back()!='>') return false;
    const auto p=t.find(" --> "); const auto sim=t.find(" <-> "); const auto imp=t.find(" ==> ");
    const auto pos=p!=std::string::npos?p:(sim!=std::string::npos?sim:imp);
    if(pos==std::string::npos) return false;
    const auto op=sim==pos?AtomType::SimilarityLink:(imp==pos?AtomType::ImplicationLink:AtomType::InheritanceLink);
    const auto left=t.substr(1,pos-1), right=t.substr(pos+5,t.size()-pos-6);
    if(left.empty()||right.empty()) return false;
    AtomId id=binary(as,op,left,right,{}, {1.0,0.9}); if(created)*created=id; return true;
}
} // namespace brain::representation
