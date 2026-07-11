# 09 — Voice Chat Verification

**Goal:** Confirm two-way lobby voice is audible between two devices — completing the milestone.

> **Muting / HUD:** this file only verifies audio flows. For the on-screen **mic/speaker mute
> HUD** and the `IVoiceChatUser` device-mute API behind it, see **`10_InGame_Menu_Voice_HUD.md`** §5.

---

## Concept — why there's almost no "voice code"

You may notice this file has no new C++. That's intentional: **EOS RTC voice attaches automatically to the lobby.** You already enabled every required piece across earlier files:

- **Portal** (file 01): the client policy has the **Voice** feature.
- **Config** (file 02): `bUseEOSRTC=True` + `RECORD_AUDIO` permission.
- **Session settings** (file 03): `bUseLobbiesVoiceChatIfAvailable = true` → the engine sets `bEnableRTCRoom` on the lobby and every member **auto-joins** the RTC room (`EOS_LRRJAT_AutomaticJoin`).
- **Mic permission** (file 04): requested at startup on Android.

So voice "just works" once players are in the same lobby — this file is pure **verification** that the plumbing is correct. Mute/volume UI is a later milestone (`IVoiceChatUser`), not needed now.

---

## Steps

1. **Confirm the portal policy** (file 01 evidence) shows **Voice** enabled on `GTFOClonePolicy`. If it doesn't, voice will fail with a permission error no matter what — fix it first.
2. **Host + join from two devices** (PC ↔ Android, or two PCs). Put them **in different rooms or use headphones** — echo makes it impossible to judge otherwise. (Why it echoes and how to tame it: `10_InGame_Menu_Voice_HUD.md` §5, "Echo / feedback when two devices share a room".)
3. Once both are in the session, **just talk.** No button to press — the RTC room joined automatically on join.
4. Watch the logs for voice activity:
   - PC: Output Log, filter `LogEOSVoiceChat`.
   - Android: `adb logcat -s UE` and look for `LogEOSVoiceChat`.
   Expect lines showing the user logging into voice and **joining the channel**.

---

## ✅ Verify — final gate

- [ ] Portal client policy shows **Voice** enabled.
- [ ] After joining, `LogEOSVoiceChat` shows the local user **login + channel join** on both devices.
- [ ] **You can hear the other person speak** (both directions — test each mic).
- [ ] On Android, Settings → Apps → your app → Permissions shows **Microphone = Allowed**.

### If there's no voice → `99_Troubleshooting.md`
- `EOS_Permission` / `EOS_MissingPermissions` in logs → client **policy lacks Voice** (file 01).
- RTC init errors → `bUseEOSRTC` is `False` in some build's config (file 02).
- One side can't be heard → mic permission denied on that device, or wrong input device selected.

---

## 🎉 Definition of done (the whole milestone)

- [ ] PC (host) + Android (client) connect over the internet via EOS — **no IP/port config**.
- [ ] Both see each other's third-person characters moving.
- [ ] `ReplicatedTestCube` rotates / bobs / color-cycles in sync on both.
- [ ] **Two-way voice audible.**
- [ ] Evidence collected: portal screenshots, login/host/join log excerpts, a short screen recording.

When all five are true, EOS multiplayer + voice is done. Next steps (session browser UI, friends/invites, mute UI, then GTFO gameplay per the project brief) are deferred milestones — see §9 of `EOS_MULTIPLAYER_SETUP.md`.
