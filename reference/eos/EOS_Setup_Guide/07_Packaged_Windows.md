# 07 — Packaged Windows ↔ Windows (over the internet)

**Goal:** Prove two **separately packaged** builds connect over the real internet with no port forwarding — using actual Epic account logins, not the Dev Auth Tool.

---

## Concept — what changes vs file 06

File 06 proved the code path on one machine with dev logins. This step proves the two things that only a packaged, cross-network test can:

1. **Real login** — the **Epic Account Portal** (browser) instead of `-AUTH_TYPE=developer`. `bPreferPersistentAuth=True` means it caches after the first success, so subsequent launches log in silently.
2. **Real NAT traversal** — two machines on **different networks** connecting via EOS P2P relay. If it works from a laptop on a phone hotspot, you've proven "no port forwarding" for real. Same-LAN also works but doesn't prove the internet path.

---

## Steps

### 7.1 — Package a Development build

Editor → **Platforms → Windows → Package Project** (Build Configuration: **Development**, so the console and on-screen buttons are available; Shipping strips the console).

> Sanity-check the packaged config: open the packaged `…/GTFOClone/Config/DefaultEngine.ini` (or the project one you shipped from) and confirm the `Artifacts=` line with your real IDs made it in. A missing/blank artifact at package time = instant login failure.

### 7.2 — Run on two machines

- Copy the packaged build to a **second PC** (ideally on a **different network** — e.g. a laptop on a phone hotspot — to prove non-LAN).
- Launch on both. On each: **1. Epic Login** → complete the **browser** account-portal login (each machine with a *different* Epic account, both org members).

### 7.3 — Host and join

- PC 1: **2. Host**.
- PC 2: **3. Join**.

---

## ✅ Verify before continuing

- [ ] Both PCs complete the **browser** Epic login and show `LOGIN OK` (different accounts).
- [ ] Relaunching a build logs in **without** the browser (persistent auth cache working).
- [ ] PC 2 finds and joins PC 1's session; connect string starts with **`EOS:`**.
- [ ] **Ideally on two different networks:** characters visible on both, **cube in sync** on both.
- [ ] No router/port-forwarding changes were needed on either side.

> If it only works on the same LAN but not across networks, the connection isn't actually using EOS P2P relay — check both builds shipped the `NetDriverEOS` `NetDriverDefinitions` block, and look for relay/P2P errors in the log (`LogEOSSDK`). See **`99_Troubleshooting.md`**.

Cross-network PC↔PC works → open **`08_Android_Test.md`**.
