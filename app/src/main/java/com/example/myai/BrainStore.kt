package com.example.myai

import android.content.Context
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import java.io.File
import java.util.zip.ZipEntry
import java.util.zip.ZipFile
import java.util.zip.ZipOutputStream

class BrainStore(context: Context) : SQLiteOpenHelper(context, "brain.db", null, 2) {
    companion object {
        const val FORMAT_VERSION = 2
        const val MAX_EVENTS = 4096
    }
    override fun onCreate(db: SQLiteDatabase) {
        db.execSQL("CREATE TABLE brain_state (id INTEGER PRIMARY KEY CHECK(id=1), snapshot TEXT NOT NULL, updated_ms INTEGER NOT NULL)")
        db.execSQL("CREATE TABLE brain_events (id INTEGER PRIMARY KEY AUTOINCREMENT, narsese TEXT NOT NULL, created_ms INTEGER NOT NULL)")
    }
    override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) {
        if (oldVersion < 2) {
            db.execSQL("CREATE TABLE IF NOT EXISTS brain_meta (key TEXT PRIMARY KEY, value TEXT NOT NULL)")
            db.execSQL("INSERT OR REPLACE INTO brain_meta(key,value) VALUES('format_version','2')")
        }
    }
    fun saveSnapshot(snapshot: String) {
        writableDatabase.execSQL("INSERT OR REPLACE INTO brain_state(id,snapshot,updated_ms) VALUES(1,?,?)", arrayOf(snapshot, System.currentTimeMillis()))
    }
    fun addEvent(narsese: String) {
        writableDatabase.beginTransaction()
        try {
            writableDatabase.execSQL("INSERT INTO brain_events(narsese,created_ms) VALUES(?,?)", arrayOf(narsese, System.currentTimeMillis()))
            writableDatabase.execSQL("DELETE FROM brain_events WHERE id NOT IN (SELECT id FROM brain_events ORDER BY id DESC LIMIT $MAX_EVENTS)")
            writableDatabase.setTransactionSuccessful()
        } finally { writableDatabase.endTransaction() }
    }
    fun exportBrain(target: File) {
        val snapshot = writableDatabase.rawQuery("SELECT snapshot FROM brain_state WHERE id=1", null).use { if (it.moveToFirst()) it.getString(0) else "{\"version\":$FORMAT_VERSION,\"atoms\":[]}" }
        val events = buildString { writableDatabase.rawQuery("SELECT narsese FROM brain_events ORDER BY id", null).use { c -> while(c.moveToNext()) append(c.getString(0)).append('\n') } }
        ZipOutputStream(target.outputStream().buffered()).use { zip ->
            zip.putNextEntry(ZipEntry("manifest.json"))
            zip.write("""{"format":"brain.brain","version":$FORMAT_VERSION,"architecture":"armv7-symbolic","persistence":"sqlite+event-replay"}""".toByteArray()); zip.closeEntry()
            zip.putNextEntry(ZipEntry("atomspace.json")); zip.write(snapshot.toByteArray()); zip.closeEntry()
            val dbFile=File(writableDatabase.path)
            if(dbFile.exists()){zip.putNextEntry(ZipEntry("brain.db"));dbFile.inputStream().use{it.copyTo(zip)};zip.closeEntry()}
            zip.putNextEntry(ZipEntry("narsese.log"));zip.write(events.toByteArray());zip.closeEntry()
        }
    }
    private fun contextDatabaseFile(): File = File(writableDatabase.path)
    fun importBrain(source: File): List<String> {
        val events=mutableListOf<String>()
        ZipFile(source).use { zip ->
            val manifest=zip.getEntry("manifest.json") ?: error("Invalid brain.brain: missing manifest")
            val json=zip.getInputStream(manifest).bufferedReader().use{it.readText()}
            val version=Regex("\\\"version\\\"\\\\s*:\\s*(\\\\d+)").find(json)?.groupValues?.get(1)?.toIntOrNull() ?: error("Invalid brain.brain: missing version")
            require(version in 1..FORMAT_VERSION) {"Unsupported brain.brain version $version"}
            val entry=zip.getEntry("narsese.log") ?: return@use
            zip.getInputStream(entry).bufferedReader().useLines { lines -> lines.map{it.trim()}.filter{it.isNotBlank()}.takeLast(MAX_EVENTS).forEach(events::add) }
        }
        writableDatabase.transaction {
            delete("brain_events", null, null)
            events.forEach { execSQL("INSERT INTO brain_events(narsese,created_ms) VALUES(?,?)", arrayOf(it,System.currentTimeMillis())) }
        }
        return events
    }
    private inline fun <T> SQLiteDatabase.transaction(block: SQLiteDatabase.() -> T): T {
        beginTransaction(); return try { val result=block(); setTransactionSuccessful(); result } finally { endTransaction() }
    }
}
