# Model-Jarvis

Native Android local AI assistant using llama.cpp and GGUF models.

## Features

- Import and load local `.gguf` models.
- Uses embedded GGUF chat-template metadata when available.
- llama.cpp native inference.
- ARMv7 32-bit (`armeabi-v7a`) Android build.
- GitHub Actions build produces a debug APK artifact.

## Build locally

```bash
cd jarvis
./gradlew assembleDebug -PjarvisAbi=armeabi-v7a
```
