package com.example.myai

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

class ConversationEngine(
    private val brainStore: BrainStore,
    private val webTools: WebTools = WebTools()
) {
    suspend fun reply(input: String): String = withContext(Dispatchers.Default) {
        val text = input.trim()
        if (text.isEmpty()) return@withContext ""

        val question = BrainNative.answerQuestion(text)
        if (question.isNotEmpty()) {
            brainStore.addEvent(question)
            brainStore.saveSnapshot(BrainNative.snapshot())
            return@withContext question
        }

        val learned = BrainNative.processText(text, 2)
        if (looksLikeKnowledgeStatement(text) && learned.isNotEmpty()) {
            brainStore.addEvent(learned)
            brainStore.saveSnapshot(BrainNative.snapshot())
            return@withContext "I learned: $learned"
        }

        if (looksLikeQuestion(text)) {
            val web = webTools.searchSummary(text)
            if (web.isSuccess) {
                val answer = web.getOrNull().orEmpty()
                if (answer.isNotBlank() && !answer.startsWith("No direct web answer")) {
                    return@withContext "I didn't have that in my local knowledge, so I checked the web.\n\n$answer"
                }
            }
            return@withContext "I don't know that yet. I can check the web when a search result is available."
        }

        if (learned.isNotEmpty()) {
            brainStore.addEvent(learned)
            brainStore.saveSnapshot(BrainNative.snapshot())
            return@withContext "I processed that as knowledge: $learned"
        }

        "I understand the message, but I don't know enough to answer it yet."
    }

    private fun looksLikeKnowledgeStatement(text: String): Boolean {
        val lower = text.lowercase()
        return " is " in lower || lower.startsWith("<") || " are " in lower
    }

    private fun looksLikeQuestion(text: String): Boolean {
        val lower = text.lowercase().trim()
        return text.trim().endsWith("?") ||
            lower.startsWith("what ") ||
            lower.startsWith("who ") ||
            lower.startsWith("where ") ||
            lower.startsWith("when ") ||
            lower.startsWith("why ") ||
            lower.startsWith("how ") ||
            lower.startsWith("is ") ||
            lower.startsWith("are ")
    }
}
