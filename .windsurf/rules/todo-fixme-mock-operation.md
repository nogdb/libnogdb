---
trigger: always_on
description: when finish developing any module or code
---

**Instruction:**  
Whenever you encounter any of the following markers in code, documentation, configuration, or comments:

- TODO
- FIXME
- MOCK / mock implementations
- placeholders, stubs, or incomplete interfaces

You must **automatically remediate them** by doing one of the following:

- **Confirm if the user want to remediate the issue** if yes, proceed, if no stop.
- **Complete the missing implementation** according to the surrounding design, types, and patterns.
- **Replace all mock/stub code** with production-ready logic following the project's architecture.
- **Resolve all TODO/FIXME comments** by writing the actual code, tests, or documentation required.
- If information is incomplete, **infer the intent from context** and produce the best possible real implementation based on the existing system rules (UUID v7, enums, Axum 0.7, async Rust patterns, abstraction interfaces, etc.).
- Only if the TODO refers to future product features (not technical debt), create a complete placeholder API with safe defaults and mark it as implemented but dormant rather than leaving a TODO.

**Additional rules:**

- Always remove the marker after remediation.
- Follow project conventions: Rust, Axum 0.7, UUID v7, interfaces/traits for pluggable modules, and strict type-safety.
- Add tests as needed to validate the new implementation.
- Ensure final code is production-ready, deterministic, and consistent across the codebase.
- Apply security guidelines (tenant isolation, audit logging, no fail-open auth paths).
- Never leave partially completed code.

**Goal:**  
By the end of each operation, **no TODO/FIXME/MOCK markers should remain**, and the system should contain complete, correct, and production-grade logic.