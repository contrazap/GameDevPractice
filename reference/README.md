# reference/

In-repo snapshots of external source material that feature plans depend on. Anything a plan needs
to be *executable* gets copied here — external projects and folders may move or vanish, and plans in
this repo must never break when they do. External originals may still be cited as provenance.

The plan generator reads relevant `reference/` docs during plan generation (see PLAN-GENERATOR.md).

| Folder | Contents |
|---|---|
| [eos/](eos/) | EOS multiplayer material from the `MultiplayerTestEOS` test project: setup guide, verified-facts reference, secrets pattern, and the source files P0.2 ports. |
