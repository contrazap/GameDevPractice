# 02 — Project Configuration

**Goal:** Enable the EOS plugins and write the config that makes EOS your default online service — with your real credentials from file 01.

---

## Concept — what config actually does here

UE's online layer is pluggable. By default a template has **no** online subsystem. Three things flip it to EOS:

1. **Plugins** (`.uproject`) — compile the EOS code into your game.
2. **`DefaultPlatformService=EOS`** — tells the engine "when someone asks for the online subsystem, hand them EOS."
3. **The `NetDriverDefinitions` swap** — replaces the default IP net driver with **`NetDriverEOS`**, so gameplay traffic goes over EOS P2P. (It still falls through to IP behavior for non-EOS URLs, so LAN/IP travel keeps working.)

The `Artifacts=` line is where your file-01 credentials live. The `ArtifactName` ties the config block to `DefaultArtifactName`.

> **Substitution:** every `GTFOClone` below assumes that's your module name. If yours differs, change `DefaultArtifactName`, the `Artifacts` `ArtifactName`, and `PackageName` accordingly.

---

## Steps

### 2.1 — Enable plugins in `<Project>.uproject`

**Append** to the existing `Plugins` array (keep what's already there):

```json
"Plugins": [
    { "Name": "OnlineSubsystem", "Enabled": true },
    { "Name": "OnlineSubsystemUtils", "Enabled": true },
    { "Name": "OnlineSubsystemEOS", "Enabled": true }
]
```

`EOSShared`, `SocketSubsystemEOS`, `EOSVoiceChat` auto-enable as dependencies.

### 2.2 — Append to `Config/DefaultEngine.ini`

Paste your real file-01 values over each `REPLACE_ME`:

```ini
; ---------------- EOS multiplayer ----------------
[OnlineSubsystem]
DefaultPlatformService=EOS

[OnlineSubsystemEOS]
bEnabled=true

[/Script/Engine.GameEngine]
!NetDriverDefinitions=ClearArray
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/SocketSubsystemEOS.NetDriverEOS",DriverClassNameFallback="/Script/OnlineSubsystemUtils.IpNetDriver")

[/Script/OnlineSubsystemEOS.EOSSettings]
DefaultArtifactName=GTFOClone
bUseEAS=True
bUseEOSConnect=True
bPreferPersistentAuth=True
bUseEOSRTC=True
!AuthScopeFlags=ClearArray
+AuthScopeFlags=BasicProfile
+Artifacts=(ArtifactName="GTFOClone",ClientId="REPLACE_ME",ClientSecret="REPLACE_ME",ProductId="REPLACE_ME",SandboxId="REPLACE_ME",DeploymentId="REPLACE_ME",ClientEncryptionKey="REPLACE_ME")

; ---------------- Android ----------------
[/Script/AndroidRuntimeSettings.AndroidRuntimeSettings]
PackageName=com.contrazap.gtfoclone
bPackageDataInsideApk=True
+ExtraPermissions=android.permission.RECORD_AUDIO
```

### Watch out for these (from the 5.8 source)

- **`!AuthScopeFlags=ClearArray` is required, not optional.** UE 5.8's `BaseEngine.ini` pre-seeds `AuthScopeFlags` with `BasicProfile`+`FriendsList`+`Presence`. A bare `+AuthScopeFlags=BasicProfile` *appends* to that, so login requests `friends_list`/`presence` and Epic rejects it (`Invalid Client — scope: friends_list`) because the EAS app is Basic Profile only. Clear first, then add `BasicProfile`.
- The key is **`ClientEncryptionKey`**, NOT `EncryptionKey` (renamed in 5.8; the old name gets stripped from packaged builds). Any 64-hex-char string works — the one above is fine.
- **Do NOT** add `bIsUsingP2PSockets` — deprecated in 5.6, now always-on.
- `bUseEOSRTC=True` + `RECORD_AUDIO` are for **voice**. Deferring voice? Set RTC `False` and drop the permission.
- Leave the template's existing `GameMapsSettings` / input sections **untouched**.
- Optional while debugging (noisy): under `[Core.Log]` add `LogEOSSDK=Verbose`, `LogOnlineSession=Verbose`, `LogNet=Log`.

---

## ✅ Verify before continuing

1. **Open the project** in the UE editor (it may prompt to rebuild — allow it).
2. **Window → Developer Tools → Output Log**, then filter for `EOS`:
   - [ ] You see EOS subsystem lines (e.g. `LogEOSSDK` initializing / `OnlineSubsystemEOS` loading) and **no** "Artifact not found" or malformed-config errors.
3. Open the console (backtick `` ` ``) and run:
   ```
   Online.GetSubsystemName
   ```
   or check the log at startup —
   - [ ] the active subsystem resolves to **EOS**, not NULL.
4. **Edit → Plugins** — [ ] `OnlineSubsystemEOS` shows **Enabled**.

> You are **not** logging in yet — there's no UI or code for that until files 03–04. This gate only proves EOS is loaded and configured. A login test comes in file 04.

All boxes ticked → open **`03_Session_Subsystem.md`**.

Config errors (bad artifact, wrong key name) → **`99_Troubleshooting.md`**.
