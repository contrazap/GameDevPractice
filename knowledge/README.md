# knowledge/

Your personal wiki — one note per finished feature, closed while it's fresh (template in [CONVENTIONS.md](../CONVENTIONS.md)): Claude drafts the mechanics sections from the code; you write decisions, gotchas, would-do-differently. This folder exists because implementations fade from memory; the notes + the code are what you return to, and the plan generator reads them to avoid re-teaching what you already know.

Naming: `KN-<short-topic>.md`, e.g. `KN-eos-sessions.md`, `KN-weapon-replication.md`.

Seed suggestion: before P0.2, write `KN-eos-sessions.md` from the *existing* EOS test while it's still fresh — what the pieces are (login, session hosting/search, voice), the issues you hit (the voice chat problems and their fixes), and where the relevant code lives. It becomes the reference for the plugin port.
