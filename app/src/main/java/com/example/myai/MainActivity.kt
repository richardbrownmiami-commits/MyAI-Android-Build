package com.example.myai

import android.Manifest
import android.content.pm.PackageManager
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import android.os.Bundle
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.myai.databinding.ActivityMainBinding
import kotlinx.coroutines.*
import org.json.JSONObject

class MainActivity : AppCompatActivity(), GeminiClient.GeminiListener {
    private lateinit var binding: ActivityMainBinding
    private lateinit var geminiClient: GeminiClient
    private var audioRecord: AudioRecord? = null
    private var isRecording = false
    private val activityScope = CoroutineScope(Dispatchers.Main + Job())

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Handle proactive wake-up
        if (intent.getBooleanExtra("is_proactive", false)) {
            window.addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED or
                    WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON or
                    WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        }

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        geminiClient = GeminiClient(BuildConfig.GEMINI_API_KEY, this)
        geminiClient.connect()

        binding.micButton.setOnClickListener {
            if (isRecording) stopRecording() else startRecording()
        }

        requestPermissions()
    }

    private fun requestPermissions() {
        val permissions = arrayOf(
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.POST_NOTIFICATIONS,
            Manifest.permission.SCHEDULE_EXACT_ALARM
        )
        ActivityCompat.requestPermissions(this, permissions, 100)
    }

    private fun startRecording() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
            return
        }
        
        val bufferSize = AudioRecord.getMinBufferSize(16000, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT)
        audioRecord = AudioRecord(MediaRecorder.AudioSource.MIC, 16000, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, bufferSize)
        
        audioRecord?.startRecording()
        isRecording = true
        binding.statusText.text = "Listening..."
        
        activityScope.launch(Dispatchers.IO) {
            val buffer = ByteArray(bufferSize)
            while (isRecording) {
                val read = audioRecord?.read(buffer, 0, buffer.size) ?: 0
                if (read > 0) {
                    geminiClient.sendAudio(buffer.copyOf(read))
                }
            }
        }
    }

    private fun stopRecording() {
        isRecording = false
        audioRecord?.stop()
        audioRecord?.release()
        audioRecord = null
        binding.statusText.text = "MyAI: Ready to talk"
    }

    override fun onMessage(text: String) {
        runOnUiThread {
            binding.aiResponseText.text = text
        }
    }

    override fun onAudioData(data: ByteArray) {
        // Audio is handled by the ProactiveService if it's running, 
        // or we could add a local AudioTrack here if the service isn't used for manual triggers.
        // For simplicity, let's assume the service handles all audio output.
    }

    override fun onToolCall(name: String, args: JSONObject) {
        activityScope.launch {
            when (name) {
                "save_memory" -> {
                    MemoryManager(this@MainActivity).saveMemory(args.getString("fact"), args.getString("category"))
                }
                "batch_schedule_tasks" -> {
                    val tasksJson = args.getJSONArray("tasks")
                    val tasks = mutableListOf<Task>()
                    for (i in 0 until tasksJson.length()) {
                        val obj = tasksJson.getJSONObject(i)
                        tasks.add(Task(obj.getLong("time_ms"), obj.getString("prompt")))
                    }
                    TaskManager(this@MainActivity).batchScheduleTasks(tasks)
                }
            }
        }
    }

    override fun onError(message: String) {
        runOnUiThread {
            binding.statusText.text = "Error: $message"
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        geminiClient.disconnect()
        activityScope.cancel()
    }
}
