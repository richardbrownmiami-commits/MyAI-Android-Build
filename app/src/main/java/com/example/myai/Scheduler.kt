package com.example.myai

import android.app.AlarmManager
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.os.Build
import androidx.core.app.NotificationCompat

class AlarmReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        val prompt = intent.getStringExtra("prompt") ?: "Scheduled brain task"
        val manager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            manager.createNotificationChannel(
                NotificationChannel("brain_tasks", "Brain tasks", NotificationManager.IMPORTANCE_DEFAULT)
            )
        }
        val notification = NotificationCompat.Builder(context, "brain_tasks")
            .setSmallIcon(android.R.drawable.ic_dialog_info)
            .setContentTitle("MyAI local brain")
            .setContentText(prompt)
            .setAutoCancel(true)
            .build()
        manager.notify(prompt.hashCode(), notification)
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
            timeMs.toInt(),
            intent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            if (alarmManager.canScheduleExactAlarms()) {
                alarmManager.setExactAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, timeMs, pendingIntent)
            } else {
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
