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

        brainStore.addConversationTurn("user", text)
        learnConversationPreferences(text)

        val question = BrainNative.answerQuestion(text)
        if (question.isNotEmpty()) {
            brainStore.addConversationTurn("assistant", question)
            return@withContext question
        }

        if (looksLikeQuestion(text)) {
            val web = webTools.searchSummary(text)
            if (web.isSuccess) {
                val answer = web.getOrNull().orEmpty()
                if (answer.isNotBlank() && !answer.startsWith("No direct web answer")) {
                    val response = "I did not have that in my local knowledge, so I checked the web.\n\n$answer"
                    brainStore.addConversationTurn("assistant", response)
                    return@withContext response
                }
            }
            val response = "I do not know that yet, and the web lookup did not return a direct answer."
            brainStore.addConversationTurn("assistant", response)
            return@withContext response
        }

        val learned = BrainNative.processText(text, 2)
        if (learned.isNotEmpty()) {
            brainStore.addEvent(learned)
            brainStore.saveSnapshot(BrainNative.snapshot())
            val response = if (looksLikeKnowledgeStatement(text)) {
                "I learned: $learned"
            } else {
                "I processed that as knowledge: $learned"
            }
            brainStore.addConversationTurn("assistant", response)
            return@withContext response
        }

        val profile = brainStore.loadProfile()
        val response = "I understand the message, but I do not know enough to answer it yet. " +
            "My current reasoning focus is \${profile.selfModel.substringBefore('\n').ifBlank { "symbolic reasoning" }}."
        brainStore.addConversationTurn("assistant", response)
        response
    }

    private fun learnConversationPreferences(text: String) {
        val lower = text.lowercase()
        when {
            lower.contains("be concise") || lower.contains("keep answers short") -> {
                val current = brainStore.loadProfile()
                brainStore.updatePersonality(
                    current.personality + "\nCommunication preference learned from conversation: prefer concise answers when the user asks for brevity."
                )
            }
            lower.contains("explain in detail") || lower.contains("be detailed") -> {
                val current = brainStore.loadProfile()
                brainStore.updatePersonality(
                    current.personality + "\nCommunication preference learned from conversation: provide more detailed explanations when requested."
                )
            }
            lower.startsWith("you should ") -> {
                val instruction = text.removePrefix("You should ").trim()
                if (instruction.isNotBlank()) {
                    runCatching {
                        val id = brainStore.addImprovementProposal(
                            "Conversation-derived self-improvement suggestion",
                            "When appropriate, $instruction"
                        )
                        brainStore.validateImprovement(id, true)
                    }
                }
            }
        }
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
