package com.example.myai

import android.app.AlarmManager
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.os.Build
import android.provider.Settings
import androidx.core.content.ContextCompat

class AlarmReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        val prompt = intent.getStringExtra("prompt") ?: "Time for our check-in!"
        val serviceIntent = Intent(context, ProactiveService::class.java).apply {
            putExtra("prompt", prompt)
        }
        ContextCompat.startForegroundService(context, serviceIntent)
    }
}

class TaskManager(private val context: Context) {
    private val alarmManager = context.getSystemService(Context.ALARM_SERVICE) as AlarmManager

    fun scheduleTask(timeMs: Long, prompt: String) {
        val intent = Intent(context, AlarmReceiver::class.java).apply {
            putExtra("prompt", prompt)
        }
        val pendingIntent = PendingIntent.getBroadcast(
            context,
            timeMs.toInt(), // Use timeMs as request code for uniqueness
            intent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (alarmManager.canScheduleExactAlarms()) {
                alarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, timeMs, pendingIntent)
            } else {
                // Fallback or request permission
                alarmManager.setAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, timeMs, pendingIntent)
            }
        } else {
            alarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, timeMs, pendingIntent)
        }
    }

    fun batchScheduleTasks(tasks: List<Task>) {
        tasks.forEach { scheduleTask(it.timeMs, it.prompt) }
    }
}

data class Task(val timeMs: Long, val prompt: String)
