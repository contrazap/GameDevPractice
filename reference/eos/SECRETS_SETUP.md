# Secrets setup (EOS credentials)

This repo is **public**, so the real EOS credentials are kept out of version
control. This page explains where they go and why the split works the way it does.

## What is secret

Only the EOS **`+Artifacts`** line is sensitive. It carries:

| Field | Sensitivity |
|-------|-------------|
| `ClientSecret` | secret |
| `ClientEncryptionKey` | secret |
| `ClientId` | identifier (ships in client) |
| `ProductId` / `SandboxId` / `DeploymentId` | identifiers |

> Note: EOS **GameClient** credentials are baked into every packaged build by
> design — anyone with your APK/EXE can extract them. Keeping them out of a
> public repo prevents *trivial* scraping; your real protection is the
> **Client Policy** in the Epic Dev Portal (lock it to the minimum permissions).

## How the split works

`Config/DefaultEngine.ini` is committed and holds **all non-secret** EOS config
(`bUseEOSRTC`, auth scopes, net driver, etc.) but **not** the `+Artifacts` line.

The `+Artifacts` line lives in two **gitignored** per-platform config files:

| File | Used by |
|------|---------|
| `Config/Windows/WindowsEngine.ini` | the editor + packaged **Windows** builds |
| `Config/Android/AndroidEngine.ini` | packaged **Android** builds |

Unreal automatically layers `Config/<Platform>/<Platform>Engine.ini` on top of
`DefaultEngine.ini`, and — unlike the engine's `Restricted/NoRedist` config —
these platform files **do** get cooked into packaged builds. So the credentials
reach the shipped Windows/Android clients while never entering git.

The committed `*.template` files are placeholders you copy from.

## Setup on a fresh clone

```sh
# from the project root (MultiplayerTestEOS/)
cp Config/Windows/WindowsEngine.ini.template Config/Windows/WindowsEngine.ini
cp Config/Android/AndroidEngine.ini.template Config/Android/AndroidEngine.ini
```

Then open each copy and replace the `<YOUR_EOS_*>` placeholders with the values
from your Epic Dev Portal product (Product Settings → Clients / Sandboxes /
Deployments, and the Client's ID + secret + encryption key).

`.gitignore` already excludes the two non-`.template` files, so `git status`
should never show them.

## Rotating a leaked key

If a `ClientSecret` / `ClientEncryptionKey` is ever exposed, rotate it in the
Epic Dev Portal (regenerate the client), update your two local platform inis,
and repackage. Nothing in git history needs changing because the real values
were never committed.
