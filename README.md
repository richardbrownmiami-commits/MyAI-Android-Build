# MyAI – Native Android Proactive Agent

MyAI is a native Android application (Kotlin) that acts as a proactive, witty digital peer using Gemini 3.1 Flash Live via Multimodal WebSockets.

## Features

- **Intelligence & Identity**: Powered by Gemini 3.1 Flash Live. It's witty, helpful, and remembers user context.
- **Multilingual Support**: Full real-time support for English, Hindi, and Punjabi (Voice and Text).
- **Memory Engine**: Uses a local Room Database to store and retrieve user facts.
- **Proactive Wake-up**: Uses WorkManager and AlarmManager for background scheduling and foreground service for proactive check-ins.
- **Zero-Touch**: Automatically wakes the screen and starts speaking when a task triggers.

## Setup

1. Clone the repository.
2. Open in Android Studio.
3. Ensure you have a Gemini API key.
4. Build and run.

## Build

The project includes a GitHub Actions workflow. Every push to the `main` branch will automatically trigger a build and produce a downloadable APK as an artifact.
