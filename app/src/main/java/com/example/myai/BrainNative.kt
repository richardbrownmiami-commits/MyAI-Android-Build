package com.example.myai
object BrainNative {
    init { System.loadLibrary("myai_brain") }
    @JvmStatic external fun version(): String
    @JvmStatic external fun addInheritance(subject: String, predicate: String): String
}
