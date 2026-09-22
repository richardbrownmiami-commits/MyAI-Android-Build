#include "brain/atomspace/atomspace.h"
#include "brain/representation/bridge.h"
#include "brain/persistence/sqlite_store.h"
#include <cassert>
#include <iostream>
int main(){
    brain::atomspace::AtomSpace as;
    auto id=brain::representation::inheritance(as,"Cat","Animal");
    assert(as.size()==3);
    assert(brain::representation::atom_to_narsese(as,id)=="<Cat --> Animal>.");
    brain::persistence::SqliteStore db;
    assert(db.open("brain_smoke.db")); assert(db.save(as));
    brain::atomspace::AtomSpace restored; assert(db.load(restored)); assert(restored.size()==3);
    std::cout<<"brain smoke ok\n"; return 0;
}
