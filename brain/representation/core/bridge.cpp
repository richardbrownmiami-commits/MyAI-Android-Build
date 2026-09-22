#include "brain/representation/bridge.h"
#include <sstream>
namespace brain::representation {
using namespace atomspace;
std::string atom_to_narsese(const AtomSpace& as, AtomId id) {
    auto a=as.get(id); if(!a) return {};
    if(auto n=std::dynamic_pointer_cast<Node>(a)) return n->name();
    auto l=std::dynamic_pointer_cast<Link>(a);
    if(!l || l->outgoing().size()!=2) return {};
    auto left=atom_to_narsese(as,l->outgoing()[0]), right=atom_to_narsese(as,l->outgoing()[1]);
    switch(l->type()){
      case AtomType::InheritanceLink: return "<"+left+" --> "+right+">.";
      case AtomType::SimilarityLink: return "<"+left+" <-> "+right+">.";
      case AtomType::ImplicationLink: return "<"+left+" ==> "+right+">.";
      default: return {};
    }
}
AtomId inheritance(AtomSpace& as,const std::string& a,const std::string& b,double f,double c) {
    auto x=as.add_node(AtomType::ConceptNode,a), y=as.add_node(AtomType::ConceptNode,b);
    return as.add_link(AtomType::InheritanceLink,{x->id(),y->id()},{f,c})->id();
}
} // namespace brain::representation
