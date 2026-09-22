# Minimal AtomSpace Port

This directory is reserved for the Android-specific AtomSpace core.

The port will be derived from upstream AtomSpace source, but the dependency closure must be verified before copying source files.

Required investigation targets:
- AtomSpace
- Atom
- Node
- Link
- Handle
- type system
- atom indexes
- name/type registration

The Android target should not depend on Guile, Python, OCaml, Cython, tests, packaging, or unrelated OpenCog services unless source inspection proves a dependency is required.

Upstream reference: https://github.com/opencog/atomspace
