# 06 — Local Multiplayer Test (Dev Auth Tool)

**Goal:** Get **two instances on your PC** to log in as two different Epic accounts, host + join, and see each other. This is the first proof the whole stack works — before you package or touch Android.

---

## Concept — why the Dev Auth Tool, and why not PIE

Every host/join needs **two real EOS logins**. Two problems make plain PIE unsuitable:

- **PIE shares one process → one EOS login.** Multi-client PIE can't give you two distinct accounts.
- Logging in via the browser portal twice, every iteration, is painful.

The **EOS Dev Auth Tool** solves both: you log two Epic accounts into it *once*, name them `Cred1` / `Cred2`, and then launch instances with `-AUTH_TYPE=developer` command-line flags. Your `Login()` code already detects `-AUTH_TYPE` and calls `AutoLogin` automatically (file 03). Fast, repeatable, no browser per run.

---

## Steps

### 6.1 — Get and run the Dev Auth Tool

1. Dev Portal → your product → **SDK → C SDK** — download the **EOS SDK** zip.
2. Inside: `SDK\Tools\EOS_DevAuthTool-win32-x64-*.zip`. Unzip and run it.
3. In the tool: set **Port = 8081**, then **log in** with your **two different Epic accounts**, naming their credentials **`Cred1`** and **`Cred2`**. Leave the tool running.

> Both accounts must be **accepted org members** (file 01) or their login will fail the brand-review gate.

### 6.2 — Launch two standalone instances

You need two **separate processes** (not multi-client-in-one). Two options:

**A) From the editor** — Play dropdown → **Advanced Settings**:
- Net Mode: **Play Standalone**, Number of Players: **1**.
- **Additional Launch Parameters** for the run. (The editor launches extra instances as separate processes when you set "Number of Players" > 1 with "Run Under One Process" **unchecked** — but the cleanest control is two packaged/-game launches, option B.)

**B) Two command-line launches** (most reliable) — build a Development target, then:

- **Instance A (host):**
  ```
  -AUTH_TYPE=developer -AUTH_LOGIN=localhost:8081 -AUTH_PASSWORD=Cred1
  ```
- **Instance B (client):**
  ```
  -AUTH_TYPE=developer -AUTH_LOGIN=localhost:8081 -AUTH_PASSWORD=Cred2
  ```

### 6.3 — Drive the flow

- **Instance A:** click **1. Epic Login** → wait for `[EOS] LOGIN OK` → click **2. Host**. It should travel to the map as a listen server.
- **Instance B:** click **1. Epic Login** → `LOGIN OK` → click **3. Join**. Watch for `Found 1 session(s)` → `Joined - traveling to EOS:...`.

Use **Status** on either instance any time to print login/session/netmode.

---

## ✅ Verify — the first real multiplayer gate

- [ ] Both instances show **`[EOS] LOGIN OK: <nickname>`** (two *different* nicknames).
- [ ] Instance A reaches the map as host (`Session created - starting listen server`).
- [ ] Instance B logs **`Found N session(s)`** then **`Joined - traveling to EOS:...`** (the address starts with `EOS:` — that's P2P, not an IP).
- [ ] **You see the other player's character** moving in each window (walk around to confirm).
- [ ] The **test cube rotates / bobs / color-cycles identically** in both windows — this is your replication proof.

If all five pass, EOS multiplayer genuinely works on your machine.

### Common failures → `99_Troubleshooting.md`
- `Found 0 sessions` → host not hosting, or **bucket string mismatch** between the two builds.
- Join succeeds but travel times out → `NetDriverDefinitions` block missing/wrong in `DefaultEngine.ini`.
- Login fails → account not an org member, or wrong ClientId/Secret.

Two instances connected → open **`07_Packaged_Windows.md`**.
