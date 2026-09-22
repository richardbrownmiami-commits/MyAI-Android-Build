# Brain Architecture

## Runtime layers
1. Chat/UI — Android presentation only.
2. Brain Orchestrator — routes queries, learning, reasoning, memory, and tools.
3. Cognitive Representation (CR) — stable internal representation shared by the subsystems.
4. ONA — inference engine and cognitive memory dynamics.
5. AtomSpace core — symbolic hypergraph/state representation.
6. Persistence — SQLite serialization of durable brain state.
7. Tools — optional web search/fetch adapters; never required for offline reasoning.

## ONA / AtomSpace boundary
ONA and AtomSpace remain separate engines. They communicate through the CR adapter.

The adapter must preserve, where applicable:
- term structure
- truth/confidence
- source/provenance
- timestamps
- context
- contradiction information
- NARS stamps

Do not stringify the entire AtomSpace into Narsese. Only project structures that have a meaningful NAL representation.

## Portable brain
The planned portable artifact is a state package, for example:
    brain.brain/
      manifest.json
      brain.db
      nars/
      atomspace/
      skills/
      knowledge/
      memories/

A single-file container can be added later without changing the runtime API.

## ARMv7 strategy
Build the smallest native core first. Avoid pulling the full OpenCog application stack into Android.

Excluded from the core target unless later required:
- Guile/Scheme bindings
- Python bindings
- OCaml bindings
- desktop packaging
- documentation generators
- unrelated OpenCog services
- neural inference libraries
