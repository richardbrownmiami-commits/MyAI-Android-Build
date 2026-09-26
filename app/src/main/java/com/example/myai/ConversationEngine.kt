package com.example.myai

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

class ConversationEngine(private val brainStore: BrainStore) {
    suspend fun reply(input: String): String = withContext(Dispatchers.Default) {
        val text=input.trim()
        if(text.isEmpty()) return@withContext ""
        val started=System.nanoTime()
        try {
            AppLogger.info("conversation","input length=${text.length}")
            brainStore.addConversationTurn("user",text)
            if(looksLikeQuestion(text)){
                val answer=BrainNative.answerQuestion(text)
                if(answer.isNotBlank() && !answer.startsWith("ERROR:")){
                    brainStore.addConversationTurn("assistant",answer)
                    AppLogger.info("conversation","local answer ${elapsedMs(started)}ms atoms=${BrainNative.atomCount()}")
                    return@withContext answer
                }
                AppLogger.info("conversation","knowledge gap ${elapsedMs(started)}ms")
            }
            val learned=BrainNative.processText(text,4)
            if(learned.isNotBlank() && !learned.startsWith("ERROR:")){
                brainStore.addEvent(learned)
                brainStore.saveSnapshot(BrainNative.snapshot())
                val response=if(looksLikeKnowledgeStatement(text)) "I learned: $learned" else "I processed that as knowledge: $learned"
                brainStore.addConversationTurn("assistant",response)
                AppLogger.info("conversation","processed ${elapsedMs(started)}ms atoms=${BrainNative.atomCount()}")
                return@withContext response
            }
            val profile=brainStore.loadProfile()
            val response="I understand the message, but I do not have enough grounded knowledge to answer it yet. " +
                "My current reasoning focus is ${profile.selfModel.substringBefore('\n').ifBlank { "symbolic reasoning" }}."
            brainStore.addConversationTurn("assistant",response)
            AppLogger.warn("conversation","no grounded response ${elapsedMs(started)}ms")
            response
        } catch(t:Throwable) {
            AppLogger.error("conversation","reply failed",t)
            val response="I hit an internal error while processing that request. The error was logged locally."
            runCatching { brainStore.addConversationTurn("assistant",response) }
            response
        }
    }

    fun stream(text:String,onChunk:(String)->Unit){
        val tokens=text.split(Regex("(?<=\\s)|(?=[,.!?;:])")).filter{it.isNotEmpty()}
        val buffer=StringBuilder()
        for(token in tokens){buffer.append(token);onChunk(buffer.toString())}
        if(tokens.isEmpty()) onChunk(text)
    }

    private fun looksLikeKnowledgeStatement(text:String):Boolean{
        val lower=text.lowercase()
        return " is " in lower || " are " in lower || lower.startsWith("<")
    }
    private fun looksLikeQuestion(text:String):Boolean{
        val lower=text.lowercase().trim()
        return text.trim().endsWith("?") || lower.startsWith("what ") || lower.startsWith("who ") ||
            lower.startsWith("where ") || lower.startsWith("when ") || lower.startsWith("why ") ||
            lower.startsWith("how ") || lower.startsWith("is ") || lower.startsWith("are ")
    }
    private fun elapsedMs(started:Long):Long=(System.nanoTime()-started)/1_000_000L
}
