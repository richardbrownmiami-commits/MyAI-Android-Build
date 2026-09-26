package com.example.myai

import android.content.Context
import android.util.Log
import java.io.File
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

object AppLogger {
    private const val TAG = "MyAI"
    private const val MAX_BYTES = 512 * 1024L
    private val lock = Any()
    @Volatile private var logFile: File? = null

    fun init(context: Context) {
        synchronized(lock) {
            logFile = File(context.applicationContext.filesDir, "myai.log")
            installCrashHandler()
            info("logger", "initialized")
        }
    }

    fun info(component: String, message: String) = write(Log.INFO, component, message, null)
    fun warn(component: String, message: String) = write(Log.WARN, component, message, null)
    fun error(component: String, message: String, throwable: Throwable? = null) =
        write(Log.ERROR, component, message, throwable)

    fun file(): File? = logFile

    private fun write(priority: Int, component: String, message: String, throwable: Throwable?) {
        val safe = message.replace('\n', ' ').take(4000)
        Log.println(priority, TAG, "[$component] $safe")
        synchronized(lock) {
            val file = logFile ?: return
            try {
                if (file.exists() && file.length() > MAX_BYTES) {
                    val backup = File(file.parentFile, "myai.log.1")
                    if (backup.exists()) backup.delete()
                    file.renameTo(backup)
                }
                val stamp = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.US).format(Date())
                file.appendText("\$stamp \${level(priority)} [\$component] \$safe\n")
                if (throwable != null) {
                    file.appendText("\${throwable::class.java.name}: \${throwable.message}\n")
                    throwable.stackTrace.take(20).forEach { file.appendText("  at \$it\n") }
                }
            } catch (_: Throwable) {
                // Logging must never crash the app.
            }
        }
    }

    private fun level(priority: Int) = when (priority) {
        Log.ERROR -> "ERROR"
        Log.WARN -> "WARN"
        Log.DEBUG -> "DEBUG"
        else -> "INFO"
    }

    private fun installCrashHandler() {
        val previous = Thread.getDefaultUncaughtExceptionHandler()
        Thread.setDefaultUncaughtExceptionHandler { thread, throwable ->
            error("crash", "uncaught exception on \${thread.name}", throwable)
            previous?.uncaughtException(thread, throwable)
        }
    }
}
