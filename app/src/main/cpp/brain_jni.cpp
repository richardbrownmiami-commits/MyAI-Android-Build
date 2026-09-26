#include <jni.h>
#include <android/log.h>
#include <exception>
#include <string>
#include "brain/brain_core.h"
#include "brain_orchestrator.h"

namespace {
static constexpr const char* TAG = "MyAI-Native";
static brain::BrainOrchestrator g_brain;
jstring error_result(JNIEnv* env, const char* op, const std::exception* ex = nullptr) {
    if (ex) __android_log_print(ANDROID_LOG_ERROR, TAG, "%s failed: %s", op, ex->what());
    else __android_log_print(ANDROID_LOG_ERROR, TAG, "%s failed", op);
    const std::string result = ex ? std::string("ERROR: ") + ex->what() : "ERROR: native brain failure";
    return env->NewStringUTF(result.c_str());
}
}
extern "C" JNIEXPORT jstring JNICALL Java_com_example_myai_BrainNative_version(JNIEnv* env, jclass) { return env->NewStringUTF(brain::version()); }
extern "C" JNIEXPORT jstring JNICALL Java_com_example_myai_BrainNative_addInheritance(JNIEnv* env, jclass, jstring a, jstring b) {
    try {
        if (!a || !b) return env->NewStringUTF("ERROR: null inheritance argument");
        const char* ca=env->GetStringUTFChars(a,nullptr); const char* cb=env->GetStringUTFChars(b,nullptr);
        const std::string r=g_brain.remember_inheritance(ca,cb); env->ReleaseStringUTFChars(a,ca); env->ReleaseStringUTFChars(b,cb); return env->NewStringUTF(r.c_str());
    } catch(const std::exception& e){return error_result(env,"addInheritance",&e);} catch(...){return error_result(env,"addInheritance");}
}
extern "C" JNIEXPORT jstring JNICALL Java_com_example_myai_BrainNative_reason(JNIEnv* env,jclass,jstring n,jint cycles){
    try { if(!n)return env->NewStringUTF("ERROR: null reasoning input"); const char* s=env->GetStringUTFChars(n,nullptr); const std::string r=g_brain.reason(s,(int)cycles); env->ReleaseStringUTFChars(n,s); return env->NewStringUTF(r.c_str()); }
    catch(const std::exception& e){return error_result(env,"reason",&e);} catch(...){return error_result(env,"reason");}
}
extern "C" JNIEXPORT jstring JNICALL Java_com_example_myai_BrainNative_answerQuestion(JNIEnv* env,jclass,jstring t){
    try { if(!t)return env->NewStringUTF("ERROR: null question"); const char* s=env->GetStringUTFChars(t,nullptr); const std::string r=g_brain.answer_question(s); env->ReleaseStringUTFChars(t,s); return env->NewStringUTF(r.c_str()); }
    catch(const std::exception& e){return error_result(env,"answerQuestion",&e);} catch(...){return error_result(env,"answerQuestion");}
}
extern "C" JNIEXPORT jstring JNICALL Java_com_example_myai_BrainNative_processText(JNIEnv* env,jclass,jstring t,jint cycles){
    try { if(!t)return env->NewStringUTF("ERROR: null text"); const char* s=env->GetStringUTFChars(t,nullptr); const std::string r=g_brain.process_text(s,(int)cycles); env->ReleaseStringUTFChars(t,s); return env->NewStringUTF(r.c_str()); }
    catch(const std::exception& e){return error_result(env,"processText",&e);} catch(...){return error_result(env,"processText");}
}
extern "C" JNIEXPORT jint JNICALL Java_com_example_myai_BrainNative_atomCount(JNIEnv*,jclass){try{return (jint)g_brain.atom_count();}catch(...){return 0;}}
extern "C" JNIEXPORT jstring JNICALL Java_com_example_myai_BrainNative_snapshot(JNIEnv* env,jclass){try{const std::string r=g_brain.snapshot_json();return env->NewStringUTF(r.c_str());}catch(const std::exception& e){return error_result(env,"snapshot",&e);}catch(...){return error_result(env,"snapshot");}}
extern "C" JNIEXPORT jboolean JNICALL Java_com_example_myai_BrainNative_hasExplicitContradiction(JNIEnv*,jclass){try{return g_brain.has_explicit_contradiction()?JNI_TRUE:JNI_FALSE;}catch(...){return JNI_FALSE;}}
extern "C" JNIEXPORT void JNICALL Java_com_example_myai_BrainNative_setHistoryLimit(JNIEnv*,jclass,jint limit){g_brain.set_history_limit(limit>0?(std::size_t)limit:1u);}
