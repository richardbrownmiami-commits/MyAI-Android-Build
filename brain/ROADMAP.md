# Roadmap

## Phase 1 — Native foundation
- [x] ARMv7 brain branch
- [x] Pin upstream ONA source
- [x] Trace AtomSpace dependency closure
- [x] Minimal Android AtomSpace core
- [x] Native SQLite persistence layer
- [x] Native smoke test
- [x] Android Gradle/NDK/CMake/JNI target

## Phase 2 — Cognitive bridge
- [x] Cognitive Representation data structures
- [x] AtomSpace -> Narsese projection
- [x] ONA bridge API
- [x] Provenance/source/timestamp/context fields
- [x] Explicit negation/contradiction marker support
- [x] Full bidirectional structural AtomSpace <-> ONA projection for every supported Atom type

## Phase 3 — Persistence
- [x] SQLite schema
- [x] Atom serialization
- [x] Replayable Narsese event persistence
- [x] brain.brain export/import package
- [ ] Exact opaque ONA internal-memory snapshot/restore

## Phase 4 — Android
- [x] JNI boundary
- [x] Kotlin Brain API
- [x] Standalone local chat UI
- [x] Offline-first text reasoning path
- [x] ARMv7 native packaging
- [ ] Device-level install/runtime verification

## Phase 5 — Learning and tools
- [x] General/coding/language knowledge ingestion interface
- [x] Optional web fetch tool
- [x] Configurable search tool interface
- [ ] Production search provider integration
- [ ] Robust natural-language parsing beyond simple symbolic statements
- [ ] Translation model/ruleset
- [ ] Larger general-knowledge datasets
- [ ] Coding knowledge corpus

## Phase 6 — Hardening
- [x] Complete ARMv7 APK CI pass with artifact
- [x] Native ONA regression coverage in the native build; device execution remains open
- [x] Brain format migration/versioning
- [x] Memory limits and eviction policy
- [x] Provenance and contradiction query APIs
- [x] Thread-safety around JNI brain state

## Acceptance target

The target is an ARMv7 APK that can create symbolic concepts, run ONA reasoning, persist durable state in SQLite, export/import brain.brain, operate offline, and optionally use web tools when explicitly enabled.

## Verification boundary

The remaining unchecked items require either an actual Android device/runtime, an upstream ONA opaque-memory serialization API, or external data/provider dependencies. They are intentionally not represented as complete merely by adding placeholders.
