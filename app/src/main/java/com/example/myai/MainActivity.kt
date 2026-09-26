package com.example.myai

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.example.myai.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File

class MainActivity:AppCompatActivity(){
    private lateinit var binding:ActivityMainBinding
    private lateinit var brainStore:BrainStore
    private lateinit var conversation:ConversationEngine
    private val importRequest=42

    override fun onCreate(savedInstanceState:Bundle?){
        super.onCreate(savedInstanceState)
        AppLogger.init(this)
        binding=ActivityMainBinding.inflate(layoutInflater);setContentView(binding.root)
        try{
            brainStore=BrainStore(this);conversation=ConversationEngine(brainStore)
            binding.statusText.text="MyAI ${BrainNative.version()} · ONA/NARS · atoms ${BrainNative.atomCount()}"
            AppLogger.info("ui","started")
        }catch(t:Throwable){
            AppLogger.error("ui","startup failed",t);binding.statusText.text="Startup error — see myai.log";return
        }
        binding.sendButton.setOnClickListener{
            val input=binding.inputText.text.toString().trim()
            if(input.isEmpty())return@setOnClickListener
            binding.inputText.text?.clear()
            lifecycleScope.launch{
                binding.statusText.text="Thinking..."
                val response=conversation.reply(input)
                binding.aiResponseText.text=""
                conversation.stream(response){chunk->binding.aiResponseText.text=chunk}
                binding.statusText.text="Local brain · ${BrainNative.atomCount()} atoms"
            }
        }
        binding.micButton.setOnClickListener{binding.statusText.text="Voice input is optional; text mode is fully offline."}
        binding.exportButton.setOnClickListener{
            runCatching{
                val target=File(filesDir,"brain.brain")
                brainStore.saveSnapshot(BrainNative.snapshot());brainStore.exportBrain(target)
                binding.statusText.text="Saved ${target.name} (${target.length()} bytes)"
                AppLogger.info("persistence","exported ${target.length()} bytes")
            }.onFailure{AppLogger.error("persistence","export failed",it);binding.statusText.text="Export error — see myai.log"}
        }
        binding.importButton.setOnClickListener{
            startActivityForResult(Intent(Intent.ACTION_OPEN_DOCUMENT).apply{type="application/octet-stream";addCategory(Intent.CATEGORY_OPENABLE)},importRequest)
        }
    }

    override fun onActivityResult(requestCode:Int,resultCode:Int,data:Intent?){
        super.onActivityResult(requestCode,resultCode,data)
        if(requestCode!=importRequest||resultCode!=RESULT_OK)return
        lifecycleScope.launch(Dispatchers.Default){
            runCatching{
                val uri=data?.data?:error("No import file selected")
                val temp=File(cacheDir,"import.brain")
                contentResolver.openInputStream(uri)?.use{input->temp.outputStream().use{output->input.copyTo(output)}}?:error("Unable to open import file")
                val events=brainStore.importBrain(temp)
                for(event in events)BrainNative.processText(event,1)
                brainStore.saveSnapshot(BrainNative.snapshot());temp.delete()
                withContext(Dispatchers.Main){
                    binding.statusText.text="Imported ${events.size} brain events"
                    binding.aiResponseText.text="Brain restored. Atoms: ${BrainNative.atomCount()}"
                }
                AppLogger.info("persistence","imported ${events.size} events")
            }.onFailure{
                AppLogger.error("persistence","import failed",it)
                withContext(Dispatchers.Main){binding.statusText.text="Import error — see myai.log"}
            }
        }
    }

    override fun onDestroy(){
        runCatching{if(::brainStore.isInitialized){brainStore.saveSnapshot(BrainNative.snapshot());brainStore.close()}}
            .onFailure{AppLogger.error("ui","shutdown save failed",it)}
        super.onDestroy()
    }
}
