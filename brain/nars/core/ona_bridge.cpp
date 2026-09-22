#include "ona_bridge.h"

extern "C" {
#include "NAR.h"
}

namespace brain::nars {
void init() {
    NAR_INIT();
}

void cycles(int count) {
    if (count > 0) NAR_Cycles(count);
}

void add_narsese(const char* sentence) {
    if (sentence == nullptr) return;
    NAR_AddInputNarsese(const_cast<char*>(sentence));
}
}
