package com.example.myai

object BrainNative {
    init {
        System.loadLibrary("myai_brain")
    }

    @JvmStatic external fun version(): String
    @JvmStatic external fun addInheritance(subject: String, predicate: String): String
    @JvmStatic external fun reason(narsese: String, cycles: Int = 2): String
    @JvmStatic external fun atomCount(): Int
    @JvmStatic external fun processText(text: String, cycles: Int = 2): String
    @JvmStatic external fun snapshot(): String
    @JvmStatic external fun hasExplicitContradiction(): Boolean
    @JvmStatic external fun setHistoryLimit(limit: Int)
}
