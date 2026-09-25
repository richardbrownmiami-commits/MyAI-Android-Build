package com.example.myai

import android.net.Uri
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.OkHttpClient
import okhttp3.Request
import org.json.JSONObject

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

    suspend fun search(query: String, endpoint: String = DEFAULT_SEARCH_ENDPOINT): Result<String> =
        fetch(endpoint.replace("{query}", Uri.encode(query)))

    suspend fun searchSummary(query: String): Result<String> =
        search(query).mapCatching { raw ->
            val json = JSONObject(raw)
            val abstractText = json.optString("AbstractText").trim()
            if (abstractText.isNotEmpty()) {
                val source = json.optString("AbstractSource").trim()
                if (source.isEmpty()) abstractText else "$abstractText (source: $source)"
            } else {
                val heading = json.optString("Heading").trim()
                if (heading.isNotEmpty()) "Search result: $heading" else "No direct web answer was returned."
            }
        }

    companion object {
        // DuckDuckGo provides Instant Answers and traditional web search results.
        // This endpoint is only a fallback knowledge lookup; fetched pages remain separate.
        const val DEFAULT_SEARCH_ENDPOINT =
            "https://api.duckduckgo.com/?format=json&no_html=1&skip_disambig=1&q={query}"
    }
}
