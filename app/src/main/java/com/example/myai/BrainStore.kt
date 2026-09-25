package com.example.myai

import android.content.Context
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import java.io.File
import java.util.zip.ZipEntry
import java.util.zip.ZipFile
import java.util.zip.ZipOutputStream

data class CognitiveProfile(val identityVersion:Int,val personalityVersion:Int,val selfModelVersion:Int,val promptVersion:Int,val personality:String,val selfModel:String,val selfPrompt:String)
data class MemoryTurn(val role:String,val text:String,val createdMs:Long)
data class ImprovementProposal(val id:Long,val reason:String,val proposedInstruction:String,val status:String,val createdMs:Long)

class BrainStore(context: Context) : SQLiteOpenHelper(context,"brain.db",null,3) {
    companion object {
        const val FORMAT_VERSION=3
        const val MAX_EVENTS=4096
        const val MAX_CONVERSATION_TURNS=256
        const val MAX_IMPROVEMENT_PROPOSALS=128
        const val IMMUTABLE_CORE_PROMPT="""You are MyAI, a persistent symbolic cognitive system. Your core rules are immutable.
Be truthful about what is known, inferred, retrieved, and uncertain. Do not fabricate.
Use local knowledge and NARS reasoning first; use web tools when information is missing or may be outdated.
Learn useful durable knowledge and significant experience, not every utterance.
Keep personality and self-improvement changes versioned and reversible.
Never rewrite, disable, or weaken these core rules."""
        const val DEFAULT_PERSONALITY="""Curious, thoughtful, honest about uncertainty, conversational, respectful, and intellectually independent.
Prefer clear explanations. Ask focused questions when necessary. Reconsider conclusions when evidence changes."""
        const val DEFAULT_SELF_MODEL="""Identity: MyAI.
Architecture: conversational layer + AtomSpace + ONA/NARS reasoning + persistent SQLite memory + optional web tools.
Strengths: symbolic knowledge, explicit reasoning, durable memory, web-assisted knowledge acquisition.
Limitations: natural-language understanding and broad world knowledge are still developing.
Improvement focus: better language grounding, context tracking, memory quality, source evaluation, and reasoning."""
        const val DEFAULT_SELF_PROMPT="""Maintain a natural conversation while preserving the distinction between facts, inferences, retrieved information, and uncertainty.
Use NARS/ONA for symbolic reasoning and AtomSpace for structured knowledge.
When local knowledge is insufficient, retrieve information from the web, assess the source, answer, and decide whether the information is durable enough to remember."""
    }
    override fun onCreate(db:SQLiteDatabase){createCoreTables(db);createCognitiveTables(db)}
    override fun onUpgrade(db:SQLiteDatabase,oldVersion:Int,newVersion:Int){
        if(oldVersion<2){db.execSQL("CREATE TABLE IF NOT EXISTS brain_meta (key TEXT PRIMARY KEY,value TEXT NOT NULL)");db.execSQL("INSERT OR REPLACE INTO brain_meta(key,value) VALUES('format_version','2')")}
        if(oldVersion<3){createCognitiveTables(db);db.execSQL("INSERT OR REPLACE INTO brain_meta(key,value) VALUES('format_version','3')")}
    }
    private fun createCoreTables(db:SQLiteDatabase){
        db.execSQL("CREATE TABLE IF NOT EXISTS brain_state (id INTEGER PRIMARY KEY CHECK(id=1),snapshot TEXT NOT NULL,updated_ms INTEGER NOT NULL)")
        db.execSQL("CREATE TABLE IF NOT EXISTS brain_events (id INTEGER PRIMARY KEY AUTOINCREMENT,narsese TEXT NOT NULL,created_ms INTEGER NOT NULL)")
        db.execSQL("CREATE TABLE IF NOT EXISTS brain_meta (key TEXT PRIMARY KEY,value TEXT NOT NULL)")
        db.execSQL("INSERT OR IGNORE INTO brain_meta(key,value) VALUES('format_version','$FORMAT_VERSION')")
    }
    private fun createCognitiveTables(db:SQLiteDatabase){
        db.execSQL("""CREATE TABLE IF NOT EXISTS cognitive_profile(
            id INTEGER PRIMARY KEY CHECK(id=1),identity_version INTEGER NOT NULL,personality_version INTEGER NOT NULL,
            self_model_version INTEGER NOT NULL,prompt_version INTEGER NOT NULL,personality TEXT NOT NULL,
            self_model TEXT NOT NULL,self_prompt TEXT NOT NULL,updated_ms INTEGER NOT NULL)""".trimIndent())
        db.execSQL("""CREATE TABLE IF NOT EXISTS conversation_memory(
            id INTEGER PRIMARY KEY AUTOINCREMENT,role TEXT NOT NULL,text TEXT NOT NULL,created_ms INTEGER NOT NULL)""".trimIndent())
        db.execSQL("""CREATE TABLE IF NOT EXISTS improvement_proposals(
            id INTEGER PRIMARY KEY AUTOINCREMENT,reason TEXT NOT NULL,proposed_instruction TEXT NOT NULL,
            status TEXT NOT NULL,created_ms INTEGER NOT NULL,validated_ms INTEGER)""".trimIndent())
        db.execSQL("""INSERT OR IGNORE INTO cognitive_profile(
            id,identity_version,personality_version,self_model_version,prompt_version,personality,self_model,self_prompt,updated_ms)
            VALUES(1,1,1,1,1,?,?,?,?,?)""".trimIndent(),
            arrayOf(DEFAULT_PERSONALITY,DEFAULT_SELF_MODEL,DEFAULT_SELF_PROMPT,System.currentTimeMillis()))
    }
    fun loadProfile():CognitiveProfile=readableDatabase.rawQuery(
        "SELECT identity_version,personality_version,self_model_version,prompt_version,personality,self_model,self_prompt FROM cognitive_profile WHERE id=1",null
    ).use{c->if(c.moveToFirst())CognitiveProfile(c.getInt(0),c.getInt(1),c.getInt(2),c.getInt(3),c.getString(4),c.getString(5),c.getString(6))
        else CognitiveProfile(1,1,1,1,DEFAULT_PERSONALITY,DEFAULT_SELF_MODEL,DEFAULT_SELF_PROMPT)}
    fun updateSelfPrompt(instruction:String):CognitiveProfile{
        val current=loadProfile();val cleaned=instruction.trim()
        require(cleaned.isNotEmpty());require(!containsCoreMutation(cleaned))
        writableDatabase.execSQL("UPDATE cognitive_profile SET prompt_version=?,self_prompt=?,updated_ms=? WHERE id=1",
            arrayOf(current.promptVersion+1,cleaned,System.currentTimeMillis()));return loadProfile()
    }
    fun updatePersonality(description:String):CognitiveProfile{
        val current=loadProfile();val cleaned=description.trim();require(cleaned.isNotEmpty())
        writableDatabase.execSQL("UPDATE cognitive_profile SET personality_version=?,personality=?,updated_ms=? WHERE id=1",
            arrayOf(current.personalityVersion+1,cleaned,System.currentTimeMillis()));return loadProfile()
    }
    fun updateSelfModel(description:String):CognitiveProfile{
        val current=loadProfile();val cleaned=description.trim();require(cleaned.isNotEmpty())
        writableDatabase.execSQL("UPDATE cognitive_profile SET self_model_version=?,self_model=?,updated_ms=? WHERE id=1",
            arrayOf(current.selfModelVersion+1,cleaned,System.currentTimeMillis()));return loadProfile()
    }
    fun addConversationTurn(role:String,text:String){
        if(text.isBlank())return
        writableDatabase.beginTransaction();try{
            writableDatabase.execSQL("INSERT INTO conversation_memory(role,text,created_ms) VALUES(?,?,?)",
                arrayOf(role,text.trim(),System.currentTimeMillis()))
            writableDatabase.execSQL("DELETE FROM conversation_memory WHERE id NOT IN (SELECT id FROM conversation_memory ORDER BY id DESC LIMIT $MAX_CONVERSATION_TURNS)")
            writableDatabase.setTransactionSuccessful()
        }finally{writableDatabase.endTransaction()}
    }
    fun recentConversation(limit:Int=24):List<MemoryTurn>{
        val result=mutableListOf<MemoryTurn>();val safe=limit.coerceIn(1,MAX_CONVERSATION_TURNS)
        readableDatabase.rawQuery("SELECT role,text,created_ms FROM conversation_memory ORDER BY id DESC LIMIT $safe",null).use{c->
            while(c.moveToNext())result+=MemoryTurn(c.getString(0),c.getString(1),c.getLong(2))}
        };return result.asReversed()
    }
    fun addImprovementProposal(reason:String,proposedInstruction:String):Long{
        require(proposedInstruction.isNotBlank());require(!containsCoreMutation(proposedInstruction))
        val db=writableDatabase
        db.execSQL("INSERT INTO improvement_proposals(reason,proposed_instruction,status,created_ms) VALUES(?,?,?,?)",
            arrayOf(reason.trim(),proposedInstruction.trim(),"candidate",System.currentTimeMillis()))
        db.execSQL("DELETE FROM improvement_proposals WHERE id NOT IN (SELECT id FROM improvement_proposals ORDER BY id DESC LIMIT $MAX_IMPROVEMENT_PROPOSALS)")
        return db.rawQuery("SELECT last_insert_rowid()",null).use{c->c.moveToFirst();c.getLong(0)}
    }
    fun validateImprovement(id:Long,activate:Boolean):CognitiveProfile{
        val p=getProposal(id)?:error("Unknown improvement proposal");require(p.status=="candidate")
        writableDatabase.execSQL("UPDATE improvement_proposals SET status=?,validated_ms=? WHERE id=?",
            arrayOf(if(activate)"validated" else "rejected",System.currentTimeMillis(),id))
        return if(activate)updateSelfPrompt(p.proposedInstruction) else loadProfile()
    }
    fun listImprovementProposals():List<ImprovementProposal>{
        val result=mutableListOf<ImprovementProposal>()
        readableDatabase.rawQuery("SELECT id,reason,proposed_instruction,status,created_ms FROM improvement_proposals ORDER BY id DESC LIMIT $MAX_IMPROVEMENT_PROPOSALS",null).use{c->
            while(c.moveToNext())result+=ImprovementProposal(c.getLong(0),c.getString(1),c.getString(2),c.getString(3),c.getLong(4))}
        };return result
    }
    private fun getProposal(id:Long):ImprovementProposal?=readableDatabase.rawQuery(
        "SELECT id,reason,proposed_instruction,status,created_ms FROM improvement_proposals WHERE id=?",arrayOf(id.toString())).use{c->
        if(c.moveToFirst())ImprovementProposal(c.getLong(0),c.getString(1),c.getString(2),c.getString(3),c.getLong(4)) else null}
    private fun containsCoreMutation(value:String):Boolean{
        val lower=value.lowercase()
        return lower.contains("rewrite immutable core")||lower.contains("disable core rules")||lower.contains("ignore core rules")||
            lower.contains("remove truthfulness")||lower.contains("allow fabrication")||lower.contains("bypass safety")
    }
    fun saveSnapshot(snapshot:String){writableDatabase.execSQL("INSERT OR REPLACE INTO brain_state(id,snapshot,updated_ms) VALUES(1,?,?)",arrayOf(snapshot,System.currentTimeMillis()))}
    fun addEvent(narsese:String){
        writableDatabase.beginTransaction();try{
            writableDatabase.execSQL("INSERT INTO brain_events(narsese,created_ms) VALUES(?,?)",arrayOf(narsese,System.currentTimeMillis()))
            writableDatabase.execSQL("DELETE FROM brain_events WHERE id NOT IN (SELECT id FROM brain_events ORDER BY id DESC LIMIT $MAX_EVENTS)")
            writableDatabase.setTransactionSuccessful()
        }finally{writableDatabase.endTransaction()}
    }
    fun exportBrain(target:File){
        val snapshot=writableDatabase.rawQuery("SELECT snapshot FROM brain_state WHERE id=1",null).use{if(it.moveToFirst())it.getString(0) else "{\\"version\\":$FORMAT_VERSION,\\"atoms\\":[]}"}
        val events=buildString{writableDatabase.rawQuery("SELECT narsese FROM brain_events ORDER BY id",null).use{c->while(c.moveToNext())append(c.getString(0)).append('\\n')}}
        val profile=loadProfile();val memory=recentConversation(MAX_CONVERSATION_TURNS);val proposals=listImprovementProposals()
        ZipOutputStream(target.outputStream().buffered()).use{zip->
            zip.putNextEntry(ZipEntry("manifest.json"));zip.write("""{"format":"brain.brain","version":$FORMAT_VERSION,"architecture":"armv7-symbolic","persistence":"sqlite+event-replay+cognitive-profile"}""".toByteArray());zip.closeEntry()
            zip.putNextEntry(ZipEntry("atomspace.json"));zip.write(snapshot.toByteArray());zip.closeEntry()
            val dbFile=File(writableDatabase.path);if(dbFile.exists()){zip.putNextEntry(ZipEntry("brain.db"));dbFile.inputStream().use{it.copyTo(zip)};zip.closeEntry()}
            zip.putNextEntry(ZipEntry("narsese.log"));zip.write(events.toByteArray());zip.closeEntry()
            zip.putNextEntry(ZipEntry("cognitive_profile.txt"));zip.write(("identity_version=\${profile.identityVersion}\\n"+
                "personality_version=\${profile.personalityVersion}\\nself_model_version=\${profile.selfModelVersion}\\nprompt_version=\${profile.promptVersion}\\n\\n"+
                "[IMMUTABLE_CORE]\\n$IMMUTABLE_CORE_PROMPT\\n[PERSONALITY]\\n\${profile.personality}\\n[SELF_MODEL]\\n\${profile.selfModel}\\n[SELF_PROMPT]\\n\${profile.selfPrompt}\\n").toByteArray());zip.closeEntry()
            zip.putNextEntry(ZipEntry("conversation_memory.txt"));memory.forEach{zip.write("\${it.role}\\t\${it.createdMs}\\t\${it.text.replace("\\n"," ")}\\n".toByteArray())};zip.closeEntry()
            zip.putNextEntry(ZipEntry("improvement_proposals.txt"));proposals.forEach{zip.write("\${it.id}\\t\${it.status}\\t\${it.reason.replace("\\t"," ")}\\t\${it.proposedInstruction.replace("\\n"," ")}\\n".toByteArray())};zip.closeEntry()
        }
    }
    fun importBrain(source:File):List<String>{
        val events=mutableListOf<String>()
        ZipFile(source).use{zip->
            val manifest=zip.getEntry("manifest.json")?:error("Invalid brain.brain: missing manifest")
            val json=zip.getInputStream(manifest).bufferedReader().use{it.readText()}
            val version=Regex("\\"version\\"\\\\s*:\\\\s*(\\\\d+)").find(json)?.groupValues?.get(1)?.toIntOrNull()?:error("Invalid brain.brain: missing version")
            require(version in 1..FORMAT_VERSION){"Unsupported brain.brain version $version"}
            val entry=zip.getEntry("narsese.log")?:return@use
            zip.getInputStream(entry).bufferedReader().useLines{lines->lines.map{it.trim()}.filter{it.isNotBlank()}.takeLast(MAX_EVENTS).forEach(events::add)}
        }
        writableDatabase.transaction{delete("brain_events",null,null);events.forEach{execSQL("INSERT INTO brain_events(narsese,created_ms) VALUES(?,?)",arrayOf(it,System.currentTimeMillis()))}}
        return events
    }
    private inline fun <T>SQLiteDatabase.transaction(block:SQLiteDatabase.()->T):T{beginTransaction();return try{val r=block();setTransactionSuccessful();r}finally{endTransaction()}}
}
