# 99 — Troubleshooting Reference

Keep this open while you work. Find your symptom, apply the cause. Ordered roughly by where in the guide you'd hit it.

---

## Symptom → cause table

| Symptom | Likely cause | Fix / which file |
|---|---|---|
| Editor log shows `OSS: none` / subsystem is NULL, not EOS | `DefaultPlatformService=EOS` missing, or plugins not enabled | File 02 — config block + plugins |
| "Artifact not found" / malformed EOS settings at startup | `Artifacts=` line typo, or `DefaultArtifactName` ≠ the `ArtifactName` | File 02 |
| Build error: `SEARCH_LOBBIES` unresolved | `OnlineBase` missing from `Build.cs` public deps | File 03 §3.1 |
| Build error: unresolved online symbols | `OnlineSubsystem` / `OnlineSubsystemUtils` missing from `Build.cs` | File 03 §3.1 |
| On-screen buttons don't appear in PIE | GameMode not using `GTFOClonePlayerController` | File 04 §4.3 + verify step 2 |
| Cube animates but stays grey (no color cycle) | Assigned material has no `Color` vector parameter | File 05 §5.2 note |
| **Login fails instantly**, log `EOS_InvalidCredentials` / `EOS_Auth_*` | Wrong ClientId/Secret, or client not linked to the EAS application | Files 01 §6 + 02 |
| Login fails with a **scope/consent** error | Portal Permissions ≠ `AuthScopeFlags` — both must be **Basic Profile only** | File 01 §6 |
| **Login works for you, fails for a friend/2nd account** | That account isn't an **accepted org member** (brand-review gate) | File 01 §1 |
| Browser opens but app never gets the token **(Android)** | Missing/wrong `eos.<clientid>` intent filter — `Artifacts=` line broken at package time | File 08 §8.3 |
| `CreateSession` fails, `EOS_Permission` denied | Client policy missing Lobbies/Sessions actions — use **Peer2Peer/Custom** | File 01 §5 |
| **Search finds 0 sessions** | Host not logged in/hosting; **bucket string mismatch**; different `SessionBucket` between builds | Files 03 (`SessionBucket`) + 06 |
| Join OK but travel fails / times out | `NetDriverDefinitions` block missing/wrong on one side | File 02 (both builds must match) |
| Works on LAN but not across networks | Not actually using EOS P2P relay — check `NetDriverEOS` shipped in both configs; grep `LogEOSSDK` for relay/P2P errors | Files 02 + 07 |
| **No voice at all** | Policy Voice missing, `bUseEOSRTC=False`, or mic permission denied | File 09 |
| One person can't be heard | Mic permission denied on their device, or wrong input device | File 09 |
| **Echo / rising howl when two devices are physically close** | Acoustic coupling — each device's speaker feeds the other's nearby mic; EOS AEC can't cancel a *different* device's speaker | Headphones on one device, or mute one device's **Speaker** — File 10 §5 |

---

## First things to check for any networking failure

1. **Both builds have the identical `SessionBucket`** string (`GTFOClone_v1` in file 03). A mismatch = client never finds host.
2. **Both builds shipped the `NetDriverDefinitions` block** (file 02). One side missing it = travel fails after a successful join.
3. **Both accounts are accepted org members** (file 01). The brand-review gate silently blocks non-members.
4. **The `Artifacts=` line is present and correct in the packaged config** — not just your source `DefaultEngine.ini`.

---

## Turning on verbose logs

Add under `[Core.Log]` in `DefaultEngine.ini` while debugging (noisy — remove after):

```ini
[Core.Log]
LogEOSSDK=Verbose
LogOnlineSession=Verbose
LogEOSVoiceChat=Verbose
LogNet=Log
```

- **PC:** Window → Developer Tools → Output Log, filter by the category.
- **Android:** `adb logcat -s UE`.

---

## What "good" looks like in the log

- Startup: EOS subsystem initializes, artifact loads, `OSS: EOS`.
- Login: `[EOS] LOGIN OK: <nickname>`.
- Host: `Session created - starting listen server`.
- Client: `Found N session(s)` → `Joined - traveling to EOS:<PUID>:GameNetDriver:...` (the `EOS:` prefix = P2P).
- Voice: `LogEOSVoiceChat` login + channel join on both ends.

---

If a symptom isn't here, cross-reference the deeper facts table and the source-verified notes in `../EOS_MULTIPLAYER_SETUP.md` (§1 facts table, §7.5 troubleshooting).
