package com.example.myai

import android.content.Context
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import java.io.File
import java.util.zip.ZipEntry
import java.util.zip.ZipFile
import java.util.zip.ZipOutputStream

class BrainStore(context: Context) : SQLiteOpenHelper(context, "brain.db", null, 1) {
    override fun onCreate(db: SQLiteDatabase) {
        db.execSQL("CREATE TABLE brain_state (id INTEGER PRIMARY KEY CHECK(id=1), snapshot TEXT NOT NULL, updated_ms INTEGER NOT NULL)")
        db.execSQL("CREATE TABLE brain_events (id INTEGER PRIMARY KEY AUTOINCREMENT, narsese TEXT NOT NULL, created_ms INTEGER NOT NULL)")
    }

    override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) = Unit

    fun saveSnapshot(snapshot: String) {
        writableDatabase.execSQL(
            "INSERT OR REPLACE INTO brain_state(id,snapshot,updated_ms) VALUES(1,?,?)",
            arrayOf(snapshot, System.currentTimeMillis())
        )
    }

    fun addEvent(narsese: String) {
        writableDatabase.execSQL(
            "INSERT INTO brain_events(narsese,created_ms) VALUES(?,?)",
            arrayOf(narsese, System.currentTimeMillis())
        )
    }

    fun exportBrain(target: File) {
        val snapshot = writableDatabase.rawQuery("SELECT snapshot FROM brain_state WHERE id=1", null).use {
            if (it.moveToFirst()) it.getString(0) else "{\"version\":1,\"atoms\":[]}"
        }
        val events = buildString {
            writableDatabase.rawQuery("SELECT narsese FROM brain_events ORDER BY id", null).use { c ->
                while (c.moveToNext()) append(c.getString(0)).append('\n')
            }
        }
        ZipOutputStream(target.outputStream().buffered()).use { zip ->
            zip.putNextEntry(ZipEntry("manifest.json"))
            zip.write("""{"format":"brain.brain","version":1,"architecture":"armv7-symbolic"}""".toByteArray())
            zip.closeEntry()
            zip.putNextEntry(ZipEntry("atomspace.json"))
            zip.write(snapshot.toByteArray())
            zip.closeEntry()
            zip.putNextEntry(ZipEntry("narsese.log"))
            zip.write(events.toByteArray())
            zip.closeEntry()
        }
    }

    fun importBrain(source: File): List<String> {
        val events = mutableListOf<String>()
        ZipFile(source).use { zip ->
            val entry = zip.getEntry("narsese.log") ?: return@use
            zip.getInputStream(entry).bufferedReader().useLines { lines ->
                lines.filter { it.isNotBlank() }.forEach(events::add)
            }
        }
        writableDatabase.delete("brain_events", null, null)
        for (event in events) addEvent(event)
        return events
    }
}
