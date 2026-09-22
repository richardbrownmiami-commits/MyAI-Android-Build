#include <jni.h>
#include "brain/brain_core.h"
#include "brain/atomspace/atomspace.h"
#include "brain/representation/bridge.h"
static brain::atomspace::AtomSpace g_space;
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myai_BrainNative_version(JNIEnv* env, jclass) {
    return env->NewStringUTF(brain::version());
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myai_BrainNative_addInheritance(JNIEnv* env, jclass, jstring a, jstring b) {
    const char* ca=env->GetStringUTFChars(a,nullptr), *cb=env->GetStringUTFChars(b,nullptr);
    auto id=brain::representation::inheritance(g_space,ca,cb);
    auto s=brain::representation::atom_to_narsese(g_space,id);
    env->ReleaseStringUTFChars(a,ca); env->ReleaseStringUTFChars(b,cb);
    return env->NewStringUTF(s.c_str());
}
