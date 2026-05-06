package com.example.myai

import android.content.Context
import androidx.room.Room

class MemoryManager(context: Context) {
    private val db = Room.databaseBuilder(
        context.applicationContext,
        AppDatabase::class.java, "myai-database"
    ).build()

    private val memoryDao = db.memoryDao()

    suspend fun saveMemory(fact: String, category: String) {
        memoryDao.insert(Memory(fact = fact, category = category))
    }

    suspend fun retrieveMemories(query: String): List<Memory> {
        return if (query.isEmpty()) {
            memoryDao.getAllMemories()
        } else {
            memoryDao.searchMemories(query)
        }
    }
}
