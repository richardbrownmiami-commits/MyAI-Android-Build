# Native Symbolic Brain

This branch replaces the previous GGUF/llama.cpp direction with an Android ARMv7a neuro-symbolic architecture.

## Goals
- Android 32-bit ARMv7a (`armeabi-v7a`)
- OpenNARS-for-Applications (ONA) for inference and cognitive dynamics
- A minimal AtomSpace-compatible symbolic knowledge layer
- SQLite-backed persistence
- A language/cognitive-representation layer between chat input and the symbolic engines
- Optional web tools kept outside the core brain
- Portable brain state export/import

## Architecture
Android UI -> Brain Orchestrator -> Cognitive Representation
                         |-> ONA
                         |-> AtomSpace core
                         |-> Persistence
                         `-> optional tools

## Design rule
GGUF, llama.cpp, and neural model weights are not part of this brain core.
The portable brain state is a future `brain.brain` package, not a neural model file.
