# 10 — In-Game Menu, Session Leave & Voice HUD

**Goal:** Replace the always-on corner buttons from file 04 with a proper **Escape/Back pause-less menu**, add **state guards** (can't Host while hosting, etc.), a **Leave Session** action, and an on-screen **voice HUD** to mute/unmute mic and speaker — all touch-friendly for Android.

> **Substitution:** this file documents the code as it actually ships in this project, so it uses the real names `AMultiplayerCoursePlayerController` and `UEOSSessionSubsystem`. If you followed the generic guide, that's `AGTFOClonePlayerController` / same subsystem.

---

## What changed vs file 04

File 04 built four buttons pinned top-left, always visible, with the mouse cursor permanently shown. That was fine to smoke-test EOS but wrong for actual play:

- the cursor floated over the game, so **camera look needed click-drag** (no direct mouse-look);
- there was no way to **leave** a session, and Host/Join could be pressed in nonsensical states;
- **voice** started silently with no indicator and no mute control.

This step reworks the same `BeginPlay` Slate menu into a single **`SOverlay`** with three parts — a centered menu, a top-right toggle button, and a voice HUD — and moves cursor handling so gameplay feels normal.

The old LAN test UI was also removed (see [Cleanup](#cleanup-removed-legacy-bits) at the bottom).

---

## 1 — The menu overlay

The overlay (added once in `BeginPlay`, torn down in `EndPlay`) contains:

| Piece | Where | Visible when |
|---|---|---|
| **Centered menu** — Resume · Login/Logout · Host · Join · Leave Session · Mic · Speaker · Quit | screen center, dim backdrop | only while the menu is **open** |
| **Toggle button** | top-right corner | always |
| **Voice HUD** — Mic / Speaker toggles | top-right, under the toggle | only while **voice is active** |

Buttons are large (420×84 in the menu) so they're easy to tap on a phone. The menu is **collapsed** (not just hidden) when closed, so it doesn't eat input.

### Toggling — Escape / Back, and the platform label

`Escape` (desktop) and `Android_Back` (phone) are bound directly on the PlayerController's input component in `SetupInputComponent` — they aren't in any Input Mapping Context, so a plain `BindKey` is simplest:

```cpp
InputComponent->BindKey(EKeys::Escape,      IE_Pressed, this, &AMultiplayerCoursePlayerController::ToggleMainMenu);
InputComponent->BindKey(EKeys::Android_Back, IE_Pressed, this, &AMultiplayerCoursePlayerController::ToggleMainMenu);
```

The **toggle button's label is platform-specific** (compile-time `#if PLATFORM_ANDROID`):

- **Windows:** text `Menu (Esc)` — a hint. While playing, the cursor is hidden, so on desktop this is mostly a reminder that Escape opens the menu.
- **Android:** a **drawn hamburger** (three bars built from `SImage` + the `WhiteBrush`), so it never depends on a font actually having a ☰ glyph. It's directly tappable.

### No pause

`ToggleMainMenu` never calls `SetPause`. This is multiplayer — pausing one client must not affect anyone else, and other players may already be in the level.

---

## 2 — Input mode (the mouse-look fix)

The cursor/input-mode swap lives in `SetMainMenuOpen(bool)` and is the fix for "camera needs click-drag":

| Menu state | Input mode | Cursor | Result |
|---|---|---|---|
| **Closed** (playing) | `FInputModeGameOnly` | hidden | Direct mouse-look, standard TPS feel |
| **Open** | `FInputModeGameAndUI` | shown | Buttons clickable; **Escape still toggles it shut** |

`BeginPlay` finishes by calling `SetMainMenuOpen(false)`, i.e. it starts in the gameplay state.

> **Why `GameAndUI`, not `UIOnly`, when open?** In `UIOnly` the PlayerController's input component stops receiving keys, so `Escape` could no longer close the menu. `GameAndUI` keeps the key bind alive.

> **Mobile note:** touch reaches on-screen Slate buttons regardless of input mode (same as the template's virtual joystick), so the hamburger and voice HUD stay tappable on Android even in `GameOnly`.

---

## 3 — State guards

The menu buttons enable/disable themselves live via Slate `IsEnabled` attribute lambdas that read the subsystem each frame:

| Button | Label / behaviour | Enabled when |
|---|---|---|
| **Login / Logout** | one slot that flips: `Epic Login` when logged out (→ `Login()`), `Epic Logout` when logged in (→ `Logout()`) | logged out → always; logged in → **only when not in a session** (leave first) |
| **Host** | — | logged in **and** not already in a session |
| **Join** | — | logged in **and** not already in a session |
| **Leave Session** | — | currently in a session |
| **Mic / Speaker** | — | voice is active |
| **Resume / Quit** | — | always |

This is backed by three subsystem queries: `IsLoggedIn()`, `IsInSession()`, and `IsVoiceActive()`.

**Login / Logout as one button.** Rather than a permanently-greyed "Epic Login" once you're signed in, the slot's label, click action, and enabled state are all driven by `IsLoggedIn()`, so it reads `Epic Logout` while signed in — a clear signal you're already logged in. `Logout()` tears down the voice workaround and calls `Identity->Logout(0)`; the live `IsLoggedIn()` attribute flips the button back to `Epic Login` when logout completes (no completion delegate needed). It's disabled while in a session so you don't strand a live lobby/voice room — **Leave Session first, then Logout.**

> **Note — login survives PIE restarts.** EOS identity lives on the OnlineSubsystem, which is owned by the editor process, not the PIE world. So logging in once keeps you logged in across PIE stop/start for the whole editor session — which is exactly why a plain disabled "Epic Login" was confusing and the flip button is clearer. To force a fresh login without the Logout button, restart the whole editor.

---

## 4 — Leave Session

`UEOSSessionSubsystem::LeaveSession()`:

1. no-ops with a message if we're not in a session;
2. `DestroySession(NAME_GameSession)`;
3. on completion, clears the tracked role and returns the player to a **solo** copy of the map via `UGameplayStatics::OpenLevel(World, "Lvl_ThirdPerson")`.

The same path works for a former **host** (tears down the listen server) and a **client** (drops the P2P connection) — everyone ends up back in the level standalone. The EOS RTC voice room is attached to the lobby, so it goes away automatically when the session is destroyed.

A small role enum (`None / Host / Client`) is set on host-create success and join success, and cleared on destroy, so the subsystem knows how it got into the session.

---

## 5 — Voice HUD + mute

EOS RTC exposes device-level mute on the per-user voice interface (`IVoiceChatUser`), confirmed present in UE 5.8:

| Subsystem method | Wraps | Meaning |
|---|---|---|
| `SetMicMuted(b)` / `IsMicMuted()` | `SetAudioInputDeviceMuted` / `GetAudioInputDeviceMuted` | stop / resume **sending my voice** |
| `SetSpeakerMuted(b)` / `IsSpeakerMuted()` | `SetAudioOutputDeviceMuted` / `GetAudioOutputDeviceMuted` | stop / resume **hearing everyone** |
| `IsVoiceActive()` | `GetChannels().Num() > 0` | are we in a voice room yet |

The HUD shows two buttons whose text flips with state — `Mic: ON` / `Mic: MUTED`, `Speaker: ON` / `Speaker: MUTED` — and is only visible while `IsVoiceActive()`. The same two toggles are mirrored inside the menu, so desktop players (cursor hidden while playing) can mute from the Escape menu; mobile players tap the HUD directly.

> The device-mute is global for our EOS user (the whole app), which is exactly what "mute my mic" / "mute everyone" should mean.

### Echo / feedback when two devices share a room

If you test with the PC and the phone side by side, you'll hear an echo — or a rising howl if both mics are open. This is **acoustic coupling**, not a bug and not a missing setting:

1. You speak on the PC → the phone's speaker plays your voice out loud;
2. the PC's mic (right next to the phone) re-captures it and sends it back → you hear yourself delayed, and the loop can build into feedback.

**EOS RTC's acoustic echo cancellation is already on by default, but it can't fix this.** AEC only subtracts a device's *own* speaker output from its *own* mic; it has no reference signal for a *different physical device's* speaker sitting next to it. No EOS setting changes that — cross-device coupling is a property of the room, not the software.

Practical fixes, in order:

- **Use headphones on at least one device.** This breaks the acoustic loop completely and is the definitive fix — standard practice for any co-located voice test.
- **Or mute one device's speaker** with the HUD **Speaker** button (§5 above) while you test the other direction.
- **Put the two devices in separate rooms.**

> A **push-to-talk** mode (mic stays muted unless a key/on-screen button is held) would also stop the open-mic feedback loop, since closed mics can't couple. It's a deferred option, intentionally **not built here** — headphones remain the correct fix for genuine co-located audio.

---

## Cleanup — removed legacy bits

Applying this step also removed the file-04-era LAN test scaffolding:

- **`WBP_UiActions`** widget (the top-right LAN **Join** button) — deleted.
- **`Lvl_ThirdPerson` Level Blueprint** — the `BeginPlay → CreateWidget(WBP_UiActions) → AddToViewport`, the `H`/`J` LAN host/join key events, and the `Escape → QuitGame` node were all deleted (Quit now lives in the menu; Escape now opens the menu).
- **`AMultiplayerCourseGameMode::HostLANGame` / `JoinLANGame`** (hardcoded LAN IP) — deleted.

---

## ✅ Verify

Rebuild (editor **closed** — this adds new `UFUNCTION`s and member variables, which Live Coding can't hot-reload), then PIE:

1. [ ] On start you can **look around with the mouse directly** (no click-drag); cursor is hidden.
2. [ ] **Escape** (or the top-right button) opens a centered menu; the cursor appears; Escape/**Resume** closes it and look-control returns.
3. [ ] Before login: **Host/Join/Leave** are greyed out. After **Epic Login**: Login greys out, Host/Join enable.
4. [ ] After **Host** (or **Join**): Host/Join grey out, **Leave Session** enables.
5. [ ] While in a voice session, the **Mic/Speaker HUD** appears top-right and the labels flip when toggled.
6. [ ] **Leave Session** drops you back into the map solo (Host/Join enabled again, Leave greyed).
7. [ ] On Android: the top-right shows a **hamburger** you can tap to open the menu.

Back to the walkthrough index: **`00_START_HERE.md`** · voice details: **`09_Voice_Chat.md`** · problems: **`99_Troubleshooting.md`**.
