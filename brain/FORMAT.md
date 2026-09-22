# Brain State Format

The runtime brain is state, not model weights.

Initial logical layout:
    manifest.json
    brain.db
    nars/
    atomspace/
    skills/
    knowledge/
    memories/

The manifest should contain:
- format version
- brain identifier
- schema version
- ONA version
- AtomSpace compatibility version
- architecture-independent state format version
- enabled capabilities
- creation/update timestamps

The format must be deterministic enough for backup, restore, and migration.

The Android runtime should never require web access to open an existing brain state.
