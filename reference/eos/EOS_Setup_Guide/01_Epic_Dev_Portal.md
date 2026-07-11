# 01 — Epic Developer Portal Setup

**Goal:** Collect the five EOS identifiers + client secret your game needs, and make sure your two test accounts are allowed to log in.

---

## Concept — why this step matters

EOS needs to know *which* product a login belongs to. That identity is a bundle of IDs (Product / Sandbox / Deployment / Client) plus a secret, which you'll paste into config in file 02. **Wrong or foreign IDs are the #1 cause of silent login failure**, so this whole file is about getting them right and verifying their format before you build anything.

One gotcha to understand up front: your app has **not** passed Epic's "Brand Review" (you won't submit it — this is a hobby project). Until it does, **only members of your organization can log in.** So both your test accounts must be org members. That's why step 1 is inviting the second account.

Portal: **https://dev.epicgames.com/portal** — sign in with your primary Epic account.

> **Evidence protocol:** after each step, note the value (or screenshot it). Verify format *before* moving on. `ProductId / SandboxId / DeploymentId / ClientId` are **32-char lowercase hex** strings (ClientId conventionally starts with `xyza`). `ClientSecret` is a generated string — treat it as a secret (it will live in `DefaultEngine.ini`, fine for dev, but note it ships in packaged builds).

---

## Steps

1. **Organization** — create one if prompted (any name). Then **Organization → Members → invite the 2nd Epic account** (the one the phone will use) and any friends. They must **accept the email invite**. *Non-members cannot log in until brand review — which you are not doing.*

2. **Create Product** — name it `GTFOClone`.

3. **Product Settings → General** — copy **ProductId**.

4. **Product Settings → Sandboxes / Deployments** — use the default **`Live`** sandbox and its default deployment. Copy **SandboxId** and **DeploymentId**.

5. **Product Settings → Clients → Add New Client:**
   - Name: `GTFOCloneClient`.
   - Client Policy: create new, name `GTFOClonePolicy`, policy type **`Peer2Peer`** (grants player-hosted Sessions/Lobbies + P2P relay).
   - **Voice:** on the policy's feature list, also enable the **Voice** feature (RTC / `voice:createRoomToken`) if it isn't already included. If the UI won't let you add Voice to the Peer2Peer preset, switch the policy to **Custom** and enable: Sessions (player-owned), Lobbies, P2P relays, **Voice**.
   - Copy **ClientId** and **ClientSecret**.

6. **Epic Account Services (left sidebar) → your application** (may be auto-created for the product — then just configure it):
   - **Permissions tab:** enable **Basic Profile** only. (This must match `AuthScopeFlags=BasicProfile` in file 02 — requested scopes must equal portal permissions or login fails with a scope error.) Leave Online Presence and Friends **OFF**.
   - **Linked Clients tab:** link `GTFOCloneClient`.
   - **Brand Settings:** leave unsubmitted (dev mode).

7. Voice needs nothing else portal-side — RTC is switched on by the client policy (step 5) plus `bUseEOSRTC` in config (file 02).

---

## Collect your values

Fill these in — you'll paste them into file 02:

```
ProductId    = ________________________________   (32 hex)
SandboxId    = ________________________________   (32 hex)
DeploymentId = ________________________________   (32 hex)
ClientId     = ________________________________   (32 hex, starts xyza)
ClientSecret = ____________________________________________  (secret)
```

---

## ✅ Verify before continuing

- [ ] Org **Members** list shows your 2nd account (and friends) with status **Accepted**.
- [ ] All four IDs are **32-char lowercase hex**; `ClientId` starts with `xyza`. (Paste-check the length — a truncated copy is a classic silent failure.)
- [ ] `ClientSecret` copied and stored somewhere safe.
- [ ] Client policy screenshot shows **Sessions/Lobbies + P2P + Voice** enabled.
- [ ] EAS application **Permissions** = **Basic Profile only**, and `GTFOCloneClient` appears under **Linked Clients**.

All five boxes ticked → open **`02_Project_Config.md`**.

If login later fails, most causes trace back to this file — see the top rows of **`99_Troubleshooting.md`**.
