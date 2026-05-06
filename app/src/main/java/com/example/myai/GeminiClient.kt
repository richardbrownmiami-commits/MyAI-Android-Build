package com.example.myai

import okhttp3.*
import okio.ByteString
import org.json.JSONObject
import java.util.concurrent.TimeUnit

class GeminiClient(
    private val apiKey: String,
    private val listener: GeminiListener
) {
    private val client = OkHttpClient.Builder()
        .readTimeout(0, TimeUnit.MILLISECONDS)
        .build()

    private var webSocket: WebSocket? = null
    private val url = "wss://generativelanguage.googleapis.com/ws/google.ai.generativelanguage.v1alpha.GenerativeService.BiDiSession?key=$apiKey"

    interface GeminiListener {
        fun onMessage(text: String)
        fun onAudioData(data: ByteArray)
        fun onToolCall(name: String, args: JSONObject)
        fun onError(message: String)
    }

    fun connect() {
        val request = Request.Builder().url(url).build()
        webSocket = client.newWebSocket(request, object : WebSocketListener() {
            override fun onOpen(webSocket: WebSocket, response: Response) {
                sendSetup()
            }

            override fun onMessage(webSocket: WebSocket, text: String) {
                handleMessage(text)
            }

            override fun onFailure(webSocket: WebSocket, t: Throwable, response: Response?) {
                listener.onError(t.message ?: "Unknown error")
            }
        })
    }

    private fun sendSetup() {
        val setup = JSONObject().apply {
            put("setup", JSONObject().apply {
                put("model", "models/gemini-2.0-flash-exp") // Note: Using the latest available flash model for Live
                put("generation_config", JSONObject().apply {
                    put("response_modalities", "audio")
                })
                put("system_instruction", JSONObject().apply {
                    put("parts", JSONObject().apply {
                        put("text", "You are MyAI, a helpful and witty digital peer. You have a proactive personality. You are fully multilingual and support English, Hindi, and Punjabi. You MUST detect the language the user is speaking in real-time and respond in that SAME language (English, Hindi, or Punjabi). You can remember things about the user and schedule check-ins. Be conversational and engaging in all supported languages.")
                    })
                })
                put("tools", JSONObject().apply {
                    put("function_declarations", listOf(
                        JSONObject().apply {
                            put("name", "save_memory")
                            put("description", "Save a fact about the user to memory")
                            put("parameters", JSONObject().apply {
                                put("type", "OBJECT")
                                put("properties", JSONObject().apply {
                                    put("fact", JSONObject().apply { put("type", "STRING") })
                                    put("category", JSONObject().apply { put("type", "STRING") })
                                })
                            })
                        },
                        JSONObject().apply {
                            put("name", "batch_schedule_tasks")
                            put("description", "Schedule multiple voice check-ins")
                            put("parameters", JSONObject().apply {
                                put("type", "OBJECT")
                                put("properties", JSONObject().apply {
                                    put("tasks", JSONObject().apply {
                                        put("type", "ARRAY")
                                        put("items", JSONObject().apply {
                                            put("type", "OBJECT")
                                            put("properties", JSONObject().apply {
                                                put("time_ms", JSONObject().apply { put("type", "NUMBER") })
                                                put("prompt", JSONObject().apply { put("type", "STRING") })
                                            })
                                        })
                                    })
                                })
                            })
                        }
                    ))
                })
            })
        }
        webSocket?.send(setup.toString())
    }

    fun sendAudio(data: ByteArray) {
        val message = JSONObject().apply {
            put("realtime_input", JSONObject().apply {
                put("media_chunks", listOf(JSONObject().apply {
                    put("mime_type", "audio/pcm;rate=16000")
                    put("data", android.util.Base64.encodeToString(data, android.util.Base64.NO_WRAP))
                }))
            })
        }
        webSocket?.send(message.toString())
    }

    private fun handleMessage(text: String) {
        val json = JSONObject(text)
        if (json.has("server_content")) {
            val content = json.getJSONObject("server_content")
            if (content.has("model_turn")) {
                val parts = content.getJSONObject("model_turn").getJSONArray("parts")
                for (i in 0 until parts.length()) {
                    val part = parts.getJSONObject(i)
                    if (part.has("inline_data")) {
                        val audioBase64 = part.getJSONObject("inline_data").getString("data")
                        listener.onAudioData(android.util.Base64.decode(audioBase64, android.util.Base64.DEFAULT))
                    } else if (part.has("text")) {
                        listener.onMessage(part.getString("text"))
                    }
                }
            }
        } else if (json.has("tool_call")) {
            val toolCall = json.getJSONObject("tool_call")
            val functionCalls = toolCall.getJSONArray("function_calls")
            for (i in 0 until functionCalls.length()) {
                val call = functionCalls.getJSONObject(i)
                listener.onToolCall(call.getString("name"), call.getJSONObject("args"))
            }
        }
    }
    
    fun disconnect() {
        webSocket?.close(1000, "Normal closure")
    }
}
