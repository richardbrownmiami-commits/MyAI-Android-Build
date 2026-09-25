#include <jni.h>
#include "brain/brain_core.h"
#include "brain_orchestrator.h"

static brain::BrainOrchestrator g_brain;

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myai_BrainNative_version(JNIEnv* env, jclass) {
    return env->NewStringUTF(brain::version());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myai_BrainNative_addInheritance(JNIEnv* env, jclass, jstring a, jstring b) {
    const char* ca = env->GetStringUTFChars(a, nullptr);
    const char* cb = env->GetStringUTFChars(b, nullptr);
    const std::string result = g_brain.remember_inheritance(ca, cb);
    env->ReleaseStringUTFChars(a, ca);
    env->ReleaseStringUTFChars(b, cb);
    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myai_BrainNative_reason(JNIEnv* env, jclass, jstring narsese, jint cycles) {
    const char* input = env->GetStringUTFChars(narsese, nullptr);
    const std::string result = g_brain.reason(input, static_cast<int>(cycles));
    env->ReleaseStringUTFChars(narsese, input);
    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myai_BrainNative_answerQuestion(JNIEnv* env, jclass, jstring text) {\n    const char* input = env->GetStringUTFChars(text, nullptr);\n    const std::string result = g_brain.answer_question(input);\n    env->ReleaseStringUTFChars(text, input);\n    return env->NewStringUTF(result.c_str());\n}\n\nextern "C" JNIEXPORT jstring JNICALL\nJava_com_example_myai_BrainNative_processText(JNIEnv* env, jclass, jstring text, jint cycles) {
    const char* input = env->GetStringUTFChars(text, nullptr);
    const std::string result = g_brain.process_text(input, static_cast<int>(cycles));
    env->ReleaseStringUTFChars(text, input);
    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_myai_BrainNative_atomCount(JNIEnv*, jclass) {
    return static_cast<jint>(g_brain.atom_count());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myai_BrainNative_snapshot(JNIEnv* env, jclass) {
    const std::string result = g_brain.snapshot_json();
    return env->NewStringUTF(result.c_str());
}


extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_myai_BrainNative_hasExplicitContradiction(JNIEnv*, jclass) {
    return g_brain.has_explicit_contradiction() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_myai_BrainNative_setHistoryLimit(JNIEnv*, jclass, jint limit) {
    g_brain.set_history_limit(limit > 0 ? static_cast<std::size_t>(limit) : 1u);
}
