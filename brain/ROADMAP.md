# Roadmap

## Phase 1 — Native foundation
- [x] Create ARMv7 brain branch
- [x] Pin upstream ONA source as a native dependency
- [x] Trace AtomSpace source dependency closure
- [x] Define minimal AtomSpace Android target
- [x] Build native smoke test

## Phase 2 — Cognitive bridge
- [ ] Define CR data structures
- [ ] ONA <-> CR adapter
- [ ] AtomSpace <-> CR adapter
- [ ] contradiction/provenance handling

## Phase 3 — Persistence
- [x] SQLite schema
- [x] Atom serialization
- [ ] ONA state persistence
- [ ] brain import/export

## Phase 4 — Android
- [ ] JNI boundary
- [ ] Kotlin Brain API
- [ ] chat UI
- [ ] offline operation

## Phase 5 — Learning and tools
- [ ] topic/dataset ingestion
- [ ] coding knowledge module
- [ ] language/translation module
- [ ] general knowledge module
- [ ] optional search/fetch tools

## Acceptance target
The first meaningful milestone is an ARMv7 APK/native test that can:
1. create symbolic concepts,
2. persist them,
3. load them,
4. submit reasoning input to ONA,
5. exchange a defined representation between ONA and AtomSpace,
6. run with no network connection.
