#include "brain/persistence/sqlite_store.h"
#include <sqlite3.h>
namespace brain::persistence {
using namespace atomspace;
bool SqliteStore::open(const std::string& path) {
    close(); return sqlite3_open(path.c_str(),reinterpret_cast<sqlite3**>(&db_))==SQLITE_OK;
}
bool SqliteStore::save(const AtomSpace& as) {
    if(!db_) return false;
    char* err=nullptr;
    const char* sql="BEGIN; CREATE TABLE IF NOT EXISTS atoms(id INTEGER PRIMARY KEY,type INTEGER,name TEXT,outgoing TEXT,f REAL,c REAL); DELETE FROM atoms;";
    if(sqlite3_exec(static_cast<sqlite3*>(db_),sql,nullptr,nullptr,&err)!=SQLITE_OK){sqlite3_free(err);return false;}
    sqlite3_stmt* st=nullptr;
    sqlite3_prepare_v2(static_cast<sqlite3*>(db_), "INSERT INTO atoms(id,type,name,outgoing,f,c) VALUES(?,?,?,?,?,?)",-1,&st,nullptr);
    for(const auto& a:as.atoms()){
        sqlite3_bind_int64(st,1,(sqlite3_int64)a->id());
        sqlite3_bind_int(st,2,(int)a->type());
        std::string name,out;
        if(auto n=std::dynamic_pointer_cast<Node>(a)) name=n->name();
        if(auto l=std::dynamic_pointer_cast<Link>(a)){ for(size_t i=0;i<l->outgoing().size();++i){if(i)out+=',';out+=std::to_string(l->outgoing()[i]);}}
        sqlite3_bind_text(st,3,name.c_str(),-1,SQLITE_TRANSIENT); sqlite3_bind_text(st,4,out.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_double(st,5,a->truth().frequency); sqlite3_bind_double(st,6,a->truth().confidence);
        sqlite3_step(st); sqlite3_reset(st); sqlite3_clear_bindings(st);
    }
    sqlite3_finalize(st); return sqlite3_exec(static_cast<sqlite3*>(db_),"COMMIT;",nullptr,nullptr,&err)==SQLITE_OK;
}
bool SqliteStore::load(AtomSpace& as) {
    if(!db_) return false; as.clear(); sqlite3_stmt* st=nullptr;
    if(sqlite3_prepare_v2(static_cast<sqlite3*>(db_),"SELECT type,name,outgoing,f,c FROM atoms ORDER BY id",-1,&st,nullptr)!=SQLITE_OK)return false;
    while(sqlite3_step(st)==SQLITE_ROW){
        auto t=(AtomType)sqlite3_column_int(st,0); const char* n=reinterpret_cast<const char*>(sqlite3_column_text(st,1));
        double f=sqlite3_column_double(st,3), c=sqlite3_column_double(st,4);
        if(n && *n){as.add_node(t,n,{f,c});}
    }
    sqlite3_finalize(st); return true;
}
void SqliteStore::close(){if(db_){sqlite3_close(static_cast<sqlite3*>(db_));db_=nullptr;}}
} // namespace brain::persistence
