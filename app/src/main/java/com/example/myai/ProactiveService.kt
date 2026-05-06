package com.example.myai

import android.app.*
import android.content.Context
import android.content.Intent
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.os.Build
import android.os.IBinder
import androidx.core.app.NotificationCompat
import org.json.JSONObject
import kotlinx.coroutines.*

class ProactiveService : Service(), GeminiClient.GeminiListener {
    private val CHANNEL_ID = "ProactiveServiceChannel"
    private lateinit var geminiClient: GeminiClient
    private lateinit var audioTrack: AudioTrack
    private val serviceScope = CoroutineScope(Dispatchers.Main + Job())

    override fun onCreate() {
        super.onCreate()
        createNotificationChannel()
        initAudioTrack()
        geminiClient = GeminiClient("AIzaSyCTmvXRgUm_4IDUnpJzFzXMf8JukP3rB-Y", this)
        geminiClient.connect()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        val prompt = intent?.getStringExtra("prompt") ?: "Hey! Ready to talk?"
        
        val notification = NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("MyAI is active")
            .setContentText(prompt)
            .setSmallIcon(android.R.drawable.ic_btn_speak_now)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setCategory(NotificationCompat.CATEGORY_CALL)
            .setFullScreenIntent(getFullScreenIntent(), true)
            .build()

        startForeground(1, notification)
        
        // Once connected, we could send the prompt to Gemini to start speaking
        // For now, we wait for the WebSocket to open in GeminiClient
        
        return START_STICKY
    }

    private fun getFullScreenIntent(): PendingIntent {
        val intent = Intent(this, MainActivity::class.java).apply {
            flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP
            putExtra("is_proactive", true)
        }
        return PendingIntent.getActivity(this, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE)
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val serviceChannel = NotificationChannel(
                CHANNEL_ID,
                "Proactive AI Service",
                NotificationManager.IMPORTANCE_HIGH
            )
            val manager = getSystemService(NotificationManager::class.java)
            manager.createNotificationChannel(serviceChannel)
        }
    }

    private fun initAudioTrack() {
        val bufferSize = AudioTrack.getMinBufferSize(16000, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT)
        audioTrack = AudioTrack.Builder()
            .setAudioAttributes(AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_ASSISTANT)
                .setContentType(AudioAttributes.CONTENT_TYPE_SPEECH)
                .build())
            .setAudioFormat(AudioFormat.Builder()
                .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                .setSampleRate(16000)
                .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                .build())
            .setBufferSizeInBytes(bufferSize)
            .setTransferMode(AudioTrack.MODE_STREAM)
            .build()
        audioTrack.play()
    }

    override fun onMessage(text: String) {
        // Handle text message if needed
    }

    override fun onAudioData(data: ByteArray) {
        audioTrack.write(data, 0, data.size)
    }

    override fun onToolCall(name: String, args: JSONObject) {
        serviceScope.launch {
            when (name) {
                "save_memory" -> {
                    val fact = args.getString("fact")
                    val category = args.getString("category")
                    MemoryManager(this@ProactiveService).saveMemory(fact, category)
                }
                "batch_schedule_tasks" -> {
                    val tasksJson = args.getJSONArray("tasks")
                    val tasks = mutableListOf<Task>()
                    for (i in 0 until tasksJson.length()) {
                        val taskObj = tasksJson.getJSONObject(i)
                        tasks.add(Task(taskObj.getLong("time_ms"), taskObj.getString("prompt")))
                    }
                    TaskManager(this@ProactiveService).batchScheduleTasks(tasks)
                }
            }
        }
    }

    override fun onError(message: String) {
        // Handle error
    }

    override fun onDestroy() {
        super.onDestroy()
        geminiClient.disconnect()
        audioTrack.stop()
        audioTrack.release()
        serviceScope.cancel()
    }

    override fun onBind(intent: Intent?): IBinder? = null
}
