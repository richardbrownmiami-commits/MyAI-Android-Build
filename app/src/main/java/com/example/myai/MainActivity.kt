package com.example.myai

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.example.myai.databinding.ActivityMainBinding
import kotlinx.coroutines.launch
import java.io.File

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private lateinit var brainStore: BrainStore
    private lateinit var conversation: ConversationEngine
    private val importRequest = 42

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        brainStore = BrainStore(this)
        conversation = ConversationEngine(brainStore)
        val profile = brainStore.loadProfile()
        binding.statusText.text = "MyAI ${BrainNative.version()} · NARS/ONA · personality v${profile.personalityVersion} · prompt v${profile.promptVersion}"

        binding.sendButton.setOnClickListener {
            val input = binding.inputText.text.toString().trim()
            if (input.isEmpty()) return@setOnClickListener
            lifecycleScope.launch {
                binding.statusText.text = "Thinking..."
                binding.aiResponseText.text = conversation.reply(input)
                val current = brainStore.loadProfile()
                binding.statusText.text = "NARS/ONA · memory · web fallback · personality v${current.personalityVersion} · prompt v${current.promptVersion}"
            }
            binding.inputText.text?.clear()
        }

        binding.micButton.setOnClickListener {
            binding.statusText.text = "Voice input is optional; use the text box for fully offline operation."
        }

        binding.exportButton.setOnClickListener {
            val target = File(filesDir, "brain.brain")
            brainStore.saveSnapshot(BrainNative.snapshot())
            brainStore.exportBrain(target)
            binding.statusText.text = "Saved ${target.name} (${target.length()} bytes)"
        }

        binding.importButton.setOnClickListener {
            startActivityForResult(
                Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
                    type = "application/octet-stream"
                    addCategory(Intent.CATEGORY_OPENABLE)
                },
                importRequest
            )
        }
    }

    private fun process(input: String) {
        val result = BrainNative.processText(input, 2)
        if (result.isNotEmpty()) brainStore.addEvent(result)
        brainStore.saveSnapshot(BrainNative.snapshot())
        binding.aiResponseText.text = buildString {
            append("Input: ").append(input)
            append("

Brain: ").append(if (result.isEmpty()) "accepted" else result)
            append("
Atoms: ").append(BrainNative.atomCount())
        }
        binding.statusText.text = "Local / offline"
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != importRequest || resultCode != RESULT_OK) return
        val uri = data?.data ?: return
        val temp = File(cacheDir, "import.brain")
        contentResolver.openInputStream(uri)?.use { input ->
            temp.outputStream().use { output -> input.copyTo(output) }
        }
        val events = brainStore.importBrain(temp)
        for (event in events) BrainNative.processText(event, 1)
        brainStore.saveSnapshot(BrainNative.snapshot())
        binding.statusText.text = "Imported ${events.size} brain events"
        binding.aiResponseText.text = "Brain restored. Atoms: ${BrainNative.atomCount()}"
        temp.delete()
    }

    override fun onDestroy() {
        brainStore.saveSnapshot(BrainNative.snapshot())
        brainStore.close()
        super.onDestroy()
    }
}
