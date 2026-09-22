#include "brain/atomspace/atomspace.h"
#include <algorithm>
#include <utility>
namespace brain::atomspace {
std::shared_ptr<Node> AtomSpace::add_node(AtomType t,const std::string& n,TruthValue tv,AtomMetadata m){auto p=Node(0,t,n);auto i=key_index_.find(p.key());if(i!=key_index_.end())return std::static_pointer_cast<Node>(atoms_.at(i->second));return add_node_with_id(next_id_,t,n,tv,std::move(m));}
std::shared_ptr<Node> AtomSpace::add_node_with_id(AtomId id,AtomType t,const std::string& n,TruthValue tv,AtomMetadata m){auto p=Node(0,t,n);auto i=key_index_.find(p.key());if(i!=key_index_.end())return std::static_pointer_cast<Node>(atoms_.at(i->second));auto x=std::make_shared<Node>(id,t,n,tv,std::move(m));key_index_[x->key()]=id;atoms_[id]=x;if(id>=next_id_)next_id_=id+1;return x;}
std::shared_ptr<Link> AtomSpace::add_link(AtomType t,const std::vector<AtomId>& o,TruthValue tv,AtomMetadata m){auto p=Link(0,t,o);auto i=key_index_.find(p.key());if(i!=key_index_.end())return std::static_pointer_cast<Link>(atoms_.at(i->second));return add_link_with_id(next_id_,t,o,tv,std::move(m));}
std::shared_ptr<Link> AtomSpace::add_link_with_id(AtomId id,AtomType t,const std::vector<AtomId>& o,TruthValue tv,AtomMetadata m){auto p=Link(0,t,o);auto i=key_index_.find(p.key());if(i!=key_index_.end())return std::static_pointer_cast<Link>(atoms_.at(i->second));auto x=std::make_shared<Link>(id,t,o,tv,std::move(m));key_index_[x->key()]=id;atoms_[id]=x;if(id>=next_id_)next_id_=id+1;return x;}
std::shared_ptr<Atom> AtomSpace::get(AtomId id)const{auto i=atoms_.find(id);return i==atoms_.end()?nullptr:i->second;}
std::shared_ptr<Node> AtomSpace::find_node(AtomType t,const std::string& n)const{Node p(0,t,n);auto i=key_index_.find(p.key());if(i==key_index_.end())return nullptr;return std::static_pointer_cast<Node>(atoms_.at(i->second));}
std::vector<std::shared_ptr<Atom>> AtomSpace::atoms()const{std::vector<std::shared_ptr<Atom>>o;o.reserve(atoms_.size());for(const auto&p:atoms_)o.push_back(p.second);std::sort(o.begin(),o.end(),[](const auto&a,const auto&b){return a->id()<b->id();});return o;}
void AtomSpace::clear(){atoms_.clear();key_index_.clear();next_id_=1;}
}