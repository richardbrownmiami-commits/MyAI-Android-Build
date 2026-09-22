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
- [ ] Full bidirectional structural AtomSpace <-> ONA projection for every supported Atom type

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
- [ ] Native ONA regression tests on Android
- [ ] Brain format migration/versioning
- [ ] Memory limits and eviction policy
- [ ] Provenance and contradiction query APIs
- [ ] Thread-safety around JNI brain state

## Acceptance target

The target is an ARMv7 APK that can create symbolic concepts, run ONA reasoning, persist durable state in SQLite, export/import brain.brain, operate offline, and optionally use web tools when explicitly enabled.
