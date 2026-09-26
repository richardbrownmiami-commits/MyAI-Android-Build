# MyAI Brain Build Plan

## Goal

Build a standalone local cognitive AI for Android ARMv7a (32-bit), optimized for a 4-thread CPU.

The first milestone is **not** internet search, large datasets, personality, or self-improvement.

The first milestone is a complete local **talking + reasoning loop**:

```
User text
  ↓
Tokenizer / language analysis
  ↓
Meaning representation
  ↓
AtomSpace
  ↓
Memory retrieval
  ↓
ONA / NARS reasoning
  ↓
Evidence / conclusion
  ↓
Response planning
  ↓
Natural-language realization
  ↓
Token stream
  ↓
Android chat display
```

The system is symbolic/neuro-symbolic. It does not use GGUF, llama.cpp, transformer LLM inference, or neural model weights.

---

## Architecture

```
                         Android Chat UI
                              │
                              ▼
                     Conversation Controller
                              │
                              ▼
                    Language / Token Layer
                              │
                    ┌─────────┴─────────┐
                    ▼                   ▼
              Understanding        Response
                    │                 Planner
                    ▼                   │
                 AtomSpace             │
                    │                   │
                    ├──── Memory ───────┤
                    │                   │
                    ▼                   ▼
                CR Adapter       Language Realizer
                    │                   │
                    ▼                   ▼
                 ONA/NARS          Token Stream
                    │                   │
                    └───────┬───────────┘
                            ▼
                       Chat Display

             SQLite = durable storage
             brain.brain = portable state
             JNI = Android/native adapter only
```

### Native core

- C/C++ brain core independent of Android.
- ONA/NARS remains the reasoning engine.
- Minimal Android AtomSpace remains the structured knowledge representation.
- Cognitive Representation (CR) connects AtomSpace structures to Narsese.
- SQLite stores durable state and history.
- JNI exposes the native brain to Kotlin.
- Android UI is a host, not the brain.

### Target platform

- Android
- ARMv7a / `armeabi-v7a`
- 32-bit
- 4-thread CPU
- Offline-first
- Low-memory aware
- No dependency on a remote LLM

---

# Phase 0 — Build Foundation and Cleanup

## Objective

Get the existing project into a clean, reproducible state before adding another capability.

## Work

- [ ] Fix malformed Kotlin source.
- [ ] Fix malformed C++ JNI source.
- [ ] Remove dead/duplicate conversation paths.
- [ ] Verify CMake/NDK ARMv7a build.
- [ ] Verify JNI load and native initialization.
- [ ] Verify ONA initialization.
- [ ] Verify AtomSpace initialization.
- [ ] Verify SQLite initialization.
- [ ] Add native and Kotlin smoke tests.
- [ ] Establish timing instrumentation.
- [ ] Establish memory/CPU instrumentation.
- [ ] Confirm 4-thread behavior without unsafe shared-state access.
- [ ] Document the exact build toolchain.

## Definition of done

The APK builds cleanly, installs on ARMv7a Android, starts the native brain, and can execute a minimal native reasoning operation.

**No new feature work starts until this phase is stable.**

---

# Phase 1 — COMPLETE TALKING + REASONING BRAIN

## Objective

Make the brain actually understand a supported subset of human language, reason over it, and answer the user.

This is the most important phase.

## 1. Input/token layer

Build a lightweight deterministic tokenizer/parser suitable for 32-bit Android.

Responsibilities:

- [ ] Normalize input.
- [ ] Tokenize words and punctuation.
- [ ] Detect statements.
- [ ] Detect questions.
- [ ] Detect yes/no questions.
- [ ] Detect simple WH questions.
- [ ] Detect subject/relation/object structures.
- [ ] Detect negation.
- [ ] Detect references such as "it", "this", and "that" where unambiguous.
- [ ] Preserve original text for response generation.

This is a symbolic language layer, not an LLM tokenizer.

## 2. Language → meaning representation

Convert supported language into structured concepts and relations.

Examples:

```
"A cat is an animal."
        ↓
ConceptNode(cat)
ConceptNode(animal)
InheritanceLink(cat, animal)
```

```
"A dog is similar to a wolf."
        ↓
SimilarityLink(dog, wolf)
```

```
"If something is a cat, it is an animal."
        ↓
ImplicationLink(cat-condition, animal-conclusion)
```

Implement:

- [ ] noun/concept extraction
- [ ] predicate/relation extraction
- [ ] inheritance
- [ ] similarity
- [ ] implication
- [ ] basic evaluation
- [ ] explicit negation
- [ ] confidence/truth values
- [ ] source/provenance
- [ ] timestamps
- [ ] conversation context

## 3. AtomSpace integration

The language layer must write actual structured atoms.

- [ ] No answer-by-string-matching as the primary mechanism.
- [ ] Every accepted fact becomes structured knowledge.
- [ ] Duplicate knowledge is merged appropriately.
- [ ] Provenance is retained.
- [ ] Truth/confidence is retained.
- [ ] Contradictory evidence is retained rather than silently overwritten.
- [ ] Relevant atoms can be retrieved by query.

## 4. Memory retrieval

Before reasoning, retrieve relevant knowledge.

```
Question
  ↓
Query representation
  ↓
AtomSpace lookup
  ↓
Relevant concepts
  ↓
Relevant relations
  ↓
Relevant prior conclusions
  ↓
NARS
```

Implement:

- [ ] exact concept lookup
- [ ] relation lookup
- [ ] bounded graph traversal
- [ ] relevance filtering
- [ ] context filtering
- [ ] recent conversation context
- [ ] provenance-aware retrieval

## 5. ONA/NARS reasoning

ONA is responsible for inference.

- [ ] Convert supported structures to Narsese.
- [ ] Submit evidence to ONA.
- [ ] Run bounded reasoning cycles.
- [ ] Collect conclusions.
- [ ] Preserve frequency/confidence.
- [ ] Handle contradiction.
- [ ] Handle transitive inheritance.
- [ ] Handle implication.
- [ ] Support multi-step inference.
- [ ] Bound reasoning so Android cannot hang indefinitely.
- [ ] Return a structured reasoning result rather than only text.

Example:

```
cat → mammal
mammal → animal

Question:
cat → animal?

NARS:
cat → mammal → animal

Result:
supported
confidence = ...
evidence = ...
```

## 6. Answer formation

Do not directly print raw Narsese to the user.

Create an intermediate answer object:

```
Answer {
    status
    conclusion
    confidence
    evidence
    provenance
    uncertainty
    explanation
}
```

Implement:

- [ ] yes/no answers
- [ ] definition answers
- [ ] relationship answers
- [ ] explanation answers
- [ ] "I don't know" when evidence is insufficient
- [ ] uncertainty statements
- [ ] contradiction explanations
- [ ] evidence-aware wording

## 7. Natural-language response

Convert the structured answer into readable language.

Example:

```
NARS conclusion:
<cat --> animal>

↓
"Yes. Based on the knowledge available to me,
a cat is an animal."
```

Implement:

- [ ] deterministic response templates
- [ ] context-aware wording
- [ ] concise response mode
- [ ] explanation mode
- [ ] uncertainty wording
- [ ] contradiction wording

## 8. Token streaming

The response must appear progressively in the Android chat.

```
Answer object
     ↓
Response text
     ↓
Tokenizer
     ↓
Token/chunk stream
     ↓
Android UI
```

Implement:

- [ ] incremental response chunks
- [ ] UI-safe main-thread updates
- [ ] cancellation
- [ ] backpressure
- [ ] final-answer event
- [ ] no UI freeze
- [ ] timing instrumentation

The UI should show the answer while it is being produced instead of waiting unnecessarily for the entire response.

## 9. Phase 1 test conversation

The following must work end-to-end:

```
User: A cat is an animal.
Brain: I understand and remember that.

User: All animals are living things.
Brain: I understand and remember that.

User: Is a cat a living thing?
Brain: Yes. A cat is a living thing, based on the reasoning chain:
      cat → animal → living thing.
```

Also:

```
User: A cat is not a vehicle.
User: Is a cat a vehicle?
Brain: No ...
```

And:

```
User: Is a unicorn a vehicle?
Brain: I don't have enough evidence to determine that.
```

## Phase 1 performance target

Target on the 4-thread ARMv7a Android device:

| Operation | Target |
|---|---:|
| Simple local answer | < 1 sec |
| Normal local reasoning | 0.2–3 sec |
| Multi-step local reasoning | 1–5 sec |
| First response chunk | < 1 sec where possible |
| No-network operation | Fully offline |

These are engineering targets, not guaranteed measurements until tested on the target device.

## Definition of done

Phase 1 is DONE only when:

- [ ] User can type natural-language input.
- [ ] Input becomes structured internal meaning.
- [ ] Meaning enters AtomSpace.
- [ ] Relevant memory is retrieved.
- [ ] ONA/NARS performs reasoning.
- [ ] Structured conclusions are returned.
- [ ] Conclusions become natural language.
- [ ] Response streams into chat.
- [ ] Unknown questions are honestly reported.
- [ ] Contradictions are represented.
- [ ] The complete loop works on ARMv7a Android.
- [ ] Automated tests cover the complete loop.
- [ ] Device test demonstrates the complete loop.
- [ ] Timing and memory are measured.

**Nothing from later phases is required to call Phase 1 complete.**

---

# Phase 2 — Learning

## Objective

Make the brain reliably learn from conversation instead of merely answering the current turn.

```
Conversation
  ↓
Candidate knowledge
  ↓
Validation
  ↓
AtomSpace
  ↓
ONA evidence
  ↓
Durable memory
```

Build:

- [ ] fact extraction
- [ ] learning validation
- [ ] confidence assignment
- [ ] provenance
- [ ] contradiction handling
- [ ] knowledge revision
- [ ] durable SQLite persistence
- [ ] retrieval after restart
- [ ] forgetting/eviction policy

Definition of done:

A fact learned in one conversation can be correctly retrieved and reasoned over in a later conversation after restarting the app.

---

# Phase 3 — Real Multi-Turn Conversation

## Objective

Make the brain use previous conversation context as cognition, not merely as a chat transcript.

Build:

- [ ] dialogue state
- [ ] reference resolution
- [ ] topic tracking
- [ ] context-aware retrieval
- [ ] follow-up questions
- [ ] conversation memory
- [ ] distinction between temporary context and durable knowledge
- [ ] correction of previously learned information

Example:

```
User: My dog is Max.
Brain: Understood.

User: He is three years old.
Brain: Understood that Max is three years old.

User: How old is my dog?
Brain: Max is three years old.
```

Definition of done:

The brain can sustain a meaningful multi-turn conversation using its internal knowledge and context.

---

# Phase 4 — Web Cognition

## Objective

Only after local cognition works, add the internet as an external knowledge tool.

The web must become part of the cognitive loop, not a separate chatbot fallback.

```
Question
  ↓
Local knowledge check
  ↓
Can local brain answer?
  ├── yes → reason → answer
  │
  └── no
       ↓
    Knowledge gap
       ↓
    Web search
       ↓
    Retrieve sources
       ↓
    Evaluate source
       ↓
    Extract facts
       ↓
    AtomSpace
       ↓
    NARS reasoning
       ↓
    Answer + provenance
       ↓
    Decide whether to remember
```

Build:

- [ ] search provider interface
- [ ] web fetch
- [ ] source metadata
- [ ] source quality signals
- [ ] source comparison
- [ ] web content extraction
- [ ] fact extraction
- [ ] provenance
- [ ] uncertainty
- [ ] web-derived memory policy
- [ ] stale-information handling
- [ ] network timeout/cancellation

Internet is an optional tool. The brain must continue working offline.

Definition of done:

A question unknown locally can trigger web retrieval, turn useful information into structured knowledge, reason over it, answer the user, and identify the source.

---

# Phase 5 — Datasets and Knowledge Expansion

## Objective

Increase knowledge without changing the core cognitive architecture.

Possible sources:

- [ ] curated general-knowledge datasets
- [ ] structured knowledge bases
- [ ] language resources
- [ ] technical/coding knowledge
- [ ] dictionaries
- [ ] domain-specific datasets
- [ ] user-approved knowledge packages

Requirements:

- [ ] dataset importer
- [ ] schema mapping
- [ ] provenance
- [ ] versioning
- [ ] deduplication
- [ ] contradiction detection
- [ ] confidence handling
- [ ] incremental installation
- [ ] ARMv7 memory limits

Large datasets must not be dumped blindly into AtomSpace.

---

# Phase 6 — Language Expansion

## Objective

Expand beyond the initial deterministic language subset.

Build progressively:

- [ ] more grammatical structures
- [ ] richer questions
- [ ] temporal expressions
- [ ] quantities
- [ ] comparisons
- [ ] conditionals
- [ ] pronouns
- [ ] coreference
- [ ] paraphrases
- [ ] multilingual rules/resources
- [ ] translation support

The language system remains a separate layer from reasoning.

---

# Phase 7 — Personality and Self-Model

## Objective

Add personality only after cognition is useful.

Build:

- [ ] persistent identity
- [ ] personality traits
- [ ] communication preferences
- [ ] self-model
- [ ] capabilities/limitations model
- [ ] versioned profile
- [ ] reversible changes

Personality must influence presentation, not override facts, evidence, or immutable core rules.

---

# Phase 8 — Controlled Self-Improvement

## Objective

Allow the system to propose improvements without allowing uncontrolled rewriting of its core.

```
Observed problem
     ↓
Improvement proposal
     ↓
Test
     ↓
Evaluation
     ↓
Versioned change
     ↓
Can rollback
```

Build:

- [ ] improvement proposals
- [ ] behavioral tests
- [ ] sandbox evaluation
- [ ] regression tests
- [ ] versioning
- [ ] rollback
- [ ] immutable core rules
- [ ] explicit activation policy

No automatic instruction such as "you should..." should directly rewrite active behavior.

Definition of done:

Every self-improvement can be identified, tested, versioned, and reversed.

---

# Phase 9 — Complete brain.brain Persistence

## Objective

Make the brain portable as a complete cognitive state.

```
brain.brain
├── manifest
├── AtomSpace
├── SQLite state
├── conversation state
├── cognitive profile
├── Narsese/event history
└── ONA state (when exact serialization is available)
```

Build:

- [ ] atomic export
- [ ] consistent SQLite snapshot
- [ ] AtomSpace serialization
- [ ] conversation state
- [ ] cognitive profile
- [ ] event history
- [ ] version migration
- [ ] integrity checks
- [ ] checksum
- [ ] import validation
- [ ] rollback on failed import
- [ ] exact ONA internal-state restore if supported/implemented

Definition of done:

Export → uninstall/reinstall → import → continue reasoning with equivalent persistent state.

---

# Phase 10 — Performance and Device Hardening

## Objective

Optimize the completed cognitive system for the actual ARMv7a 4-thread device.

Measure:

- [ ] first-token latency
- [ ] complete-response latency
- [ ] NARS cycles/sec
- [ ] AtomSpace lookup time
- [ ] SQLite latency
- [ ] parser latency
- [ ] response-generation latency
- [ ] memory usage
- [ ] peak memory
- [ ] CPU utilization
- [ ] battery impact
- [ ] concurrency
- [ ] UI frame stability

Optimize only after profiling.

Do not add complexity merely to make the architecture look more parallel.

---

# Development Rules

## Rule 1 — Vertical slices

Never build ten disconnected features at once.

Build:

```
Capability
  ↓
Integration
  ↓
Test
  ↓
Real use
  ↓
Persistence if required
  ↓
Performance verification
  ↓
DONE
```

## Rule 2 — No fake completion

A feature is not complete because:

- a class exists
- an API exists
- a database table exists
- a UI button exists
- a fallback string exists
- a web request succeeds

A capability is complete only when the brain actually uses it end-to-end.

## Rule 3 — Core before expansion

Priority:

1. Talking/reasoning loop
2. Learning
3. Conversation memory
4. Web cognition
5. Datasets
6. Language expansion
7. Personality/self-model
8. Self-improvement
9. Complete portable state
10. Optimization

## Rule 4 — Offline-first

The brain must remain useful without internet access.

Internet is an external tool, not the brain itself.

## Rule 5 — Reasoning remains symbolic

Do not replace ONA/NARS with an LLM.

Do not replace AtomSpace with a neural embedding store.

Do not introduce GGUF/llama.cpp unless the architecture is intentionally changed in a future project.

## Rule 6 — Evidence before confidence

The brain must distinguish:

- known
- inferred
- remembered
- retrieved
- uncertain
- contradictory

It must not fabricate an answer merely to keep the conversation flowing.

## Rule 7 — Measure before optimizing

All latency claims must eventually come from measurements on the target ARMv7a hardware.

## Rule 8 — Main stays usable

When a phase is complete, its implementation is integrated into `main`.

Do not accumulate finished capabilities in abandoned branches.

---

# Phase Completion Gate

Before moving from one phase to the next:

- [ ] Code implemented
- [ ] Integrated into the actual brain path
- [ ] Automated tests pass
- [ ] End-to-end test passes
- [ ] ARMv7a build passes
- [ ] Device test passes where applicable
- [ ] Persistence verified where applicable
- [ ] Failure cases tested
- [ ] Performance measured
- [ ] Documentation updated

Only then mark the phase complete.

---

# Immediate Work Order

The next work sequence is deliberately narrow:

1. **Repair the current build.**
2. **Finish Phase 0.**
3. **Build Phase 1 language → AtomSpace → NARS → answer → token stream.**
4. **Test Phase 1 end-to-end on ARMv7a.**
5. **Do not start internet, datasets, personality, or self-improvement until Phase 1 passes its completion gate.**
6. Then begin Phase 2 learning.

The first real product milestone is therefore:

> **A local ARMv7a Android brain that can talk to the user, understand supported natural language, store structured knowledge, reason with ONA/NARS, and stream a grounded answer back into the chat.**
