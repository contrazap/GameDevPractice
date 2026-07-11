# 08 — Android Build + PC ↔ Android Crossplay

**Goal:** Package for Android, log in on the phone via the Epic portal, and connect PC ↔ phone over the internet — the crossplay milestone.

---

## Concept — the one Android-specific gotcha

Almost everything is identical to Windows because EOS is cross-platform. The single Android-specific mechanism to understand is the **login redirect**:

When the phone opens the Epic Account Portal in a browser and you finish logging in, the browser has to hand the token *back to your app*. Android does that with an **intent filter** using a custom URL scheme `eos.<clientid-lowercase>`. The engine's EOS UPL script **auto-injects** this into your `AndroidManifest.xml` at package time — **no manual manifest editing** — but only if the `Artifacts=` line (with your real ClientId) is correct at package time. If that line is wrong/missing, the browser logs in but the app never receives the token.

The mic permission (`RECORD_AUDIO`) you added in file 02 + the runtime request in file 04 is the other Android piece — that's for voice (file 09).

---

## Steps

### 8.1 — Prerequisites

- Android SDK/NDK set up for UE (Android Studio + `SetupAndroid` done previously — you've packaged Android before).
- Phone in **developer mode**, USB debugging on, `adb devices` lists it.
- The phone's Epic account is an **accepted org member** (file 01).

### 8.2 — Package Development for Android

Editor → **Platforms → Android → Package Project**, Build Configuration **Development** (console via 4-finger tap; on-screen buttons work regardless).

### 8.3 — Verify the manifest BEFORE installing

Open the generated manifest:

```
Intermediate\Android\arm64\gradle\app\src\main\AndroidManifest.xml
```

- [ ] It contains `<data android:scheme="eos.<your-clientid-lowercase>"/>`.
- [ ] It contains the `RECORD_AUDIO` permission.

If the `eos.` scheme is missing, the `Artifacts=` line was wrong/absent at package time — fix file 02 and repackage. **Don't skip this check** — it's the difference between login working and silently hanging.

### 8.4 — Install, run, connect

1. Install the APK (`adb install -r <apk>` or the editor's Launch).
2. Start `adb logcat -s UE` on the PC to watch logs (look for `LogEOSSDK`, `[EOS]`).
3. On the phone: **1. Epic Login** → the browser opens → complete login → app returns to `LOGIN OK`. Accept the **microphone** permission dialog if it appears.
4. PC hosts (**2. Host**), phone joins (**3. Join**) — or vice versa.

---

## ✅ Verify before continuing

- [ ] Manifest contained the `eos.<clientid>` intent filter and `RECORD_AUDIO` (§8.3).
- [ ] Phone completes the browser login and returns to the app with **`LOGIN OK`** (watch `adb logcat`).
- [ ] **PC ↔ phone connect** over the internet: each sees the other's character, cube in sync.
- [ ] The OS **microphone permission** dialog appeared on first run (needed for file 09).

> This satisfies the core milestone: **PC (host) + Android (client) over the internet via EOS, no IP/port config, both seeing each other and the replicated cube.**

### Common Android failures → `99_Troubleshooting.md`
- Browser logs in but app never proceeds → missing/wrong intent filter (§8.3).
- Login fails for the phone account only → not an accepted org member.

Crossplay works → open **`09_Voice_Chat.md`** for the final piece.
