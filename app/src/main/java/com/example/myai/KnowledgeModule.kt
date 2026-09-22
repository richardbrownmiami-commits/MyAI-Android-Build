package com.example.myai

class KnowledgeModule(private val brainStore: BrainStore) {
    enum class Domain { GENERAL, CODING, LANGUAGE }

    fun ingest(domain: Domain, text: String): String {
        val result = BrainNative.processText(text.trim(), 2)
        if (result.isNotEmpty()) brainStore.addEvent(result)
        brainStore.saveSnapshot(BrainNative.snapshot())
        return "[${domain.name.lowercase()}] ${text.trim()}"
    }
}
