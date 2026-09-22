package com.example.myai

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.OkHttpClient
import okhttp3.Request

class WebTools {
    private val client = OkHttpClient()

    suspend fun fetch(url: String): Result<String> = withContext(Dispatchers.IO) {
        runCatching {
            val request = Request.Builder().url(url).get().build()
            client.newCall(request).execute().use { response ->
                if (!response.isSuccessful) error("HTTP ${response.code}")
                response.body?.string() ?: ""
            }
        }
    }

    suspend fun search(query: String, endpoint: String): Result<String> =
        fetch(endpoint.replace("{query}", java.net.URLEncoder.encode(query, "UTF-8")))
}
