#include "brain/persistence/sqlite_store.h"
#include <sqlite3.h>
#include <sstream>
#include <vector>
namespace brain::persistence {
using namespace atomspace;
static std::vector<AtomId> parse_ids(const char*s){std::vector<AtomId>v;if(!s||!*s)return v;std::stringstream ss(s);std::string x;while(std::getline(ss,x,','))if(!x.empty())v.push_back((AtomId)std::stoull(x));return v;}
static bool exec(sqlite3*db,const char*sql){char*e=nullptr;const int rc=sqlite3_exec(db,sql,nullptr,nullptr,&e);if(e)sqlite3_free(e);return rc==SQLITE_OK;}
bool SqliteStore::open(const std::string&path){close();return sqlite3_open(path.c_str(),reinterpret_cast<sqlite3**>(&db_))==SQLITE_OK;}
bool SqliteStore::save(const AtomSpace&as){
 if(!db_)return false; auto*db=(sqlite3*)db_; if(!exec(db,"PRAGMA user_version=2; CREATE TABLE IF NOT EXISTS atoms(id INTEGER PRIMARY KEY,type INTEGER,name TEXT,outgoing TEXT,f REAL,c REAL,source TEXT DEFAULT '',timestamp_ms INTEGER DEFAULT 0,context TEXT DEFAULT '');"))return false;
 exec(db,"ALTER TABLE atoms ADD COLUMN source TEXT DEFAULT '';"); exec(db,"ALTER TABLE atoms ADD COLUMN timestamp_ms INTEGER DEFAULT 0;"); exec(db,"ALTER TABLE atoms ADD COLUMN context TEXT DEFAULT '';");
 if(!exec(db,"BEGIN; DELETE FROM atoms;"))return false;
 sqlite3_stmt*st=nullptr; if(sqlite3_prepare_v2(db,"INSERT INTO atoms(id,type,name,outgoing,f,c,source,timestamp_ms,context) VALUES(?,?,?,?,?,?,?,?,?)",-1,&st,nullptr)!=SQLITE_OK)return false;
 for(const auto&a:as.atoms()){sqlite3_bind_int64(st,1,(sqlite3_int64)a->id());sqlite3_bind_int(st,2,(int)a->type());std::string n,o;if(auto x=std::dynamic_pointer_cast<Node>(a))n=x->name();if(auto x=std::dynamic_pointer_cast<Link>(a))for(size_t i=0;i<x->outgoing().size();++i){if(i)o+=',';o+=std::to_string(x->outgoing()[i]);}
  sqlite3_bind_text(st,3,n.c_str(),-1,SQLITE_TRANSIENT);sqlite3_bind_text(st,4,o.c_str(),-1,SQLITE_TRANSIENT);sqlite3_bind_double(st,5,a->truth().frequency);sqlite3_bind_double(st,6,a->truth().confidence);sqlite3_bind_text(st,7,a->metadata().source.c_str(),-1,SQLITE_TRANSIENT);sqlite3_bind_int64(st,8,a->metadata().timestamp_ms);sqlite3_bind_text(st,9,a->metadata().context.c_str(),-1,SQLITE_TRANSIENT);
  if(sqlite3_step(st)!=SQLITE_DONE){sqlite3_finalize(st);exec(db,"ROLLBACK;");return false;}sqlite3_reset(st);sqlite3_clear_bindings(st);}
 sqlite3_finalize(st);return exec(db,"COMMIT;");
}
bool SqliteStore::load(AtomSpace&as){
 if(!db_)return false; auto*db=(sqlite3*)db_;as.clear();sqlite3_stmt*st=nullptr;
 if(sqlite3_prepare_v2(db,"SELECT id,type,name,outgoing,f,c,source,timestamp_ms,context FROM atoms ORDER BY id",-1,&st,nullptr)!=SQLITE_OK)return false;
 struct Row{AtomId id;AtomType type;std::string name,out,source,context;TruthValue tv;std::int64_t ts;};std::vector<Row>links;
 while(sqlite3_step(st)==SQLITE_ROW){Row r{(AtomId)sqlite3_column_int64(st,0),(AtomType)sqlite3_column_int(st,1),(const char*)sqlite3_column_text(st,2),(const char*)sqlite3_column_text(st,3),{}, {},{sqlite3_column_double(st,4),sqlite3_column_double(st,5)},sqlite3_column_int64(st,7)};const char*s=(const char*)sqlite3_column_text(st,6);const char*c=(const char*)sqlite3_column_text(st,8);r.source=s?s:"";r.context=c?c:"";if(!r.name.empty())as.add_node_with_id(r.id,r.type,r.name,r.tv,{r.source,r.ts,r.context});else links.push_back(std::move(r));}
 sqlite3_finalize(st);for(const auto&r:links)as.add_link_with_id(r.id,r.type,parse_ids(r.out.c_str()),r.tv,{r.source,r.ts,r.context});return true;
}
void SqliteStore::close(){if(db_){sqlite3_close((sqlite3*)db_);db_=nullptr;}}
}