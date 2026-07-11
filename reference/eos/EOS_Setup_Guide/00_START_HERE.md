# 00 — Start Here

**Goal:** Understand what you're building and confirm you have everything needed before touching the portal or code.

---

## Concept — the big picture

You're adding **internet multiplayer + voice** to a UE 5.8 Third Person project using **Epic Online Services (EOS)**, the same free stack Lyra uses. No servers to rent, no port forwarding.

The pieces, and the job each one does:

| Piece | What it does | Where it lives |
|---|---|---|
| **OnlineSubsystemEOS** | The engine plugin that talks to EOS. Everything below hangs off it. | Ships with UE |
| **Identity interface** | Logs the player into their **Epic account** (browser portal, or Dev Auth Tool while iterating). | `IOnlineIdentity` |
| **Session / Lobby interface** | Host **creates** a lobby; clients **search** for it and **join**. This is discovery. | `IOnlineSession` |
| **NetDriverEOS (P2P)** | Once joined, this carries actual gameplay traffic peer-to-peer, punching through NAT with an EOS relay fallback. | Net driver |
| **EOS RTC voice** | A voice room that **auto-attaches to the lobby** — no separate voice service. | `bUseLobbiesVoiceChatIfAvailable` |

### The flow you'll build

```
HOST:    Login ─► CreateSession (lobby) ─► ServerTravel(map?listen)
CLIENT:  Login ─► FindSessions ─► JoinSession ─► ClientTravel(resolved EOS address)
                                          └► RTC voice room joins automatically
```

The host is a **listen server** (a player who is also the server). Clients connect to them directly over EOS P2P. That's why there's no port forwarding — EOS handles the NAT traversal.

---

## Roadmap

Read and complete these **in order**. Do not skip a verify gate — each one exists so a failure is caught here instead of three steps later.

| # | File | You'll end with |
|---|------|-----------------|
| 01 | `01_Epic_Dev_Portal.md` | Your 5 EOS IDs + client secret, org invites sent |
| 02 | `02_Project_Config.md` | Plugins + config wired, editor recognizes EOS |
| 03 | `03_Session_Subsystem.md` | The login/host/join C++ that compiles |
| 04 | `04_UI_And_Controller.md` | On-screen Login/Host/Join buttons in PIE |
| 05 | `05_Replication_Cube.md` | A test cube proving replication works |
| 06 | `06_Local_MP_Test.md` | **Two instances connected on your PC** (first real proof) |
| 07 | `07_Packaged_Windows.md` | Two PCs connected over the internet |
| 08 | `08_Android_Test.md` | PC ↔ Android crossplay |
| 09 | `09_Voice_Chat.md` | Two-way voice audible |
| 10 | `10_InGame_Menu_Voice_HUD.md` | Escape/Back menu, guards, Leave Session, voice mute HUD |
| 99 | `99_Troubleshooting.md` | Reference — symptom → cause table |

Whenever a step fails, jump to **`99_Troubleshooting.md`** before retrying blindly.

---

## Prerequisites checklist

- [ ] **UE 5.8** installed (this guide is verified against `C:\Program Files\Epic Games\UE_5.8`). Other versions may differ — config keys changed in 5.6/5.8.
- [ ] A **C++ Third Person template** project created in `C:\MyFiles\Projects\UE5Projects\GTFOClone` (you create this yourself; these guides configure on top of it). If your project/module name is **not** `GTFOClone`, note it — you'll substitute it (and the `GTFOCLONE_API` macro) everywhere.
- [ ] **Visual Studio** set up so the project compiles (you've built a UE C++ project before).
- [ ] **Two different Epic accounts** you control (one for the PC, one for the phone). Multiplayer needs two real logins.
- [ ] An **Android device** + working `adb` (only needed by file 08).
- [ ] A browser signed into your primary Epic account for the dev portal.

> **Unreal MCP:** you already have the MCP server running, so the optional MCP-plugin setup from the original spec is skipped. You can use it later (file 05) to place the test cube from the editor if you like.

---

## ✅ Verify before continuing

- [ ] Every prerequisite box above is ticked.
- [ ] Your project opens in the UE editor and compiles **before** any changes (so later build errors are clearly *your* new code, not a broken baseline).
- [ ] You know your **module name** (default `GTFOClone`) — you'll type it a lot.

When all three are true, open **`01_Epic_Dev_Portal.md`**.
