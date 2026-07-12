# features/

Detailed feature plans land here, generated on demand — see [PLAN-GENERATOR.md](../PLAN-GENERATOR.md) for the invocation.

One file per feature, generated in **two stages**: the *container* up front (orientation, mental model, prerequisites, session **outlines**, DoD, variation, KN prompts, verify list), then each session's *detail* (**Teach → Steps**) generated just-in-time at the start of the session that runs it, statically verified against the repo as it exists then. Naming: `<ID>-<short-slug>.md`, e.g. `P0.2-eos-plugin-port.md`, `P1.6a-sleeper-ai.md`.

Every plan carries its own progress record — `**Next action:**` at the top, append-only `## Session log` at the bottom, both maintained by Claude (CONVENTIONS rule 10). Resuming work in any session is: *"read `features/<ID>-*.md` and continue."*

When a plan is generated, the feature's status in [ROADMAP.md](../ROADMAP.md) moves ⬜ → 📋 (🔨 when build starts). Variation specs live *inside* each plan (`## Variation`, with a `Do after:` trigger); later plans pick up overdue ones under "Pending practice".
