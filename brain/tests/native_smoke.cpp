#include "brain/atomspace/atomspace.h"
#include "brain/representation/bridge.h"
#include "brain/representation/contradiction.h"
#include "brain/persistence/sqlite_store.h"
#include <cassert>
#include <iostream>
int main(){
    brain::atomspace::AtomSpace as;
    brain::atomspace::AtomMetadata meta{"smoke",123,"test"};
    auto id=brain::representation::inheritance(as,"Cat","Animal",0.8,0.7,meta);
    assert(as.size()==3);
    assert(brain::representation::atom_to_narsese(as,id)=="<Cat --> Animal>.");
    auto sim=brain::representation::similarity(as,"Cat","Feline",meta);
    assert(brain::representation::atom_to_narsese(as,sim)=="<Cat <-> Feline>.");
    auto imp=brain::representation::implication(as,"Cat","Mammal",meta);
    assert(brain::representation::atom_to_narsese(as,imp)=="<Cat ==> Mammal>.");
    brain::atomspace::AtomId parsed=0;
    assert(brain::representation::ingest_narsese(as,"<Dog --> Animal>.",&parsed));
    assert(brain::representation::atom_to_narsese(as,parsed)=="<Dog --> Animal>.");
    assert(as.get(id)->metadata().source=="smoke");
    brain::persistence::SqliteStore db;
    assert(db.open("brain_smoke.db")); assert(db.save(as));
    brain::atomspace::AtomSpace restored; assert(db.load(restored)); assert(restored.size()==as.size());
    assert(restored.get(id)->metadata().source=="smoke");
    std::cout<<"brain smoke ok\n"; return 0;
}
