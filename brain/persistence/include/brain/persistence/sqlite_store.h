#pragma once
#include "brain/atomspace/atomspace.h"
#include <string>
namespace brain::persistence {
class SqliteStore {
public:
    bool open(const std::string& path);
    bool save(const atomspace::AtomSpace&);
    bool load(atomspace::AtomSpace&);
    void close();
private:
    void* db_{nullptr};
};
}
