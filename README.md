# ⛏️ Ore Rush

> Red vs Blue — an **online 1v1, third-person** competitive mining game set in a procedurally generated mine. Dig ore, haul it to your vault, sabotage your rival; the first to fill the quota wins.

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-313131?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-Core%20Logic-00599C?logo=cplusplus&logoColor=white)
![Multiplayer](https://img.shields.io/badge/Multiplayer-1v1%20Listen%20Server-2ea44f)
![Status](https://img.shields.io/badge/status-in%20development-yellow)

---

## 📖 About

Ore Rush is a warm, competitive game where two miners (**Red** and **Blue**) dig ore in a procedurally generated mining valley and haul it back to their own depots. No killing — just **sabotage and theft**. The first to fill the target quota (e.g. 50 points) into their vault wins.

All core logic is written in **C++**; Blueprint is used only for presentation (mesh, animation, FX, UI). The architecture is **listen-server** and **server-authoritative**.

## ✨ Gameplay

- 🏃 **Movement & Dash** — third-person movement plus a short escape dash (server-authoritative).
- ⛏️ **Mining** — look at a vein and hold; ore goes into your wallet.
- 🎒 **Wallet** — limited capacity; you slow down when full and drop it when trapped.
- 🏦 **Vault & Score** — deposit your wallet at your depot; **score = vault**. The vault is open to raids.
- 💣 **Traps & Defenses** — built by spending ore (snare, mud, decoy, smoke / barrier, turret).
- ⚡ **Power-ups** — speed, capacity, mining speed, shield.
- 🗺️ **Procedural Map** — seed-based; both players see the same map.

## 💎 Ore Economy

| Ore | Value | Availability |
|-----|:-----:|--------------|
| 🪨 Iron | 1× | Unlimited — safe but slow |
| 🥇 Gold | 2× | Limited |
| 💎 Diamond | 5× | Very rare / limited |

## 🎮 Controls

| Action | Key |
|--------|-----|
| Move | `W` `A` `S` `D` |
| Look | `Mouse` |
| Jump | `Space` |
| Dash | `Left Shift` |
| Mine | `Left Mouse` (hold) |

## 🧱 Architecture

- **Engine:** Unreal Engine 5.7 · **Language:** C++ (core) + Blueprint (presentation)
- **Networking:** Listen-server, server-authoritative. Intent → `Server` RPC, FX → `Multicast`, state → `Replicated` / RepNotify.
- **Game module:** `OreRush`

```
Source/OreRush/
├─ Character/    # AOreRushCharacter — movement, Enhanced Input, dash, mining
├─ Components/   # UWalletComponent — wallet (carried ore)
├─ Ore/          # AOreVein — ore vein (3 types)
├─ Game/         # AOreRushGameMode, AOreRushGameState
├─ Player/       # AOreRushPlayerController, AOreRushPlayerState
└─ Core/         # OreRushTypes — ETeam, EOreType
```

## 🚀 Getting Started

- For a quick technical review, browse the C++ gameplay code under [`Source/OreRush`](Source/OreRush/).
- To open the public source project, first follow [ASSET_SETUP.md](ASSET_SETUP.md). Marketplace source assets are intentionally omitted from the public repository.

## 🗺️ Roadmap

- [x] Character + input + dash + replication
- [x] Ore veins (3 types) + mining + wallet
- [x] Depot + vault + score
- [x] GameMode + quota + win condition
- [x] Economy + traps
- [x] Depot raiding + defenses
- [x] Power-ups
- [x] Procedural map generation
- [X] UI (menu / HUD / tutorial / end screen)
- [X] Cosmetics + SFX/VFX + balancing
- [X] Packaging

## 🎨 Third-Party Assets

Ore Rush uses the third-party assets listed below. Each asset remains subject to its original license and is not relicensed by this repository. Attribution does not replace the applicable marketplace license or entitlement.

| Asset | Creator / Publisher | Source | License / Notes |
|-------|---------------------|--------|-----------------|
| **Miner character** | Timsblends | [Miner on Sketchfab](https://sketchfab.com/3d-models/miner-d22a4eb8788d4af887061ce64746c8d9) | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Imported and converted to Unreal Engine format, recolored for the Red/Blue teams, and adapted for Ore Rush. |
| **Power-up coin models** | Bitgem | [Cube World Powerup Coin — Proto Series](https://www.fab.com/listings/e25090d3-eefe-4e0b-b908-91c14c86daff) | [Fab Standard License](https://www.fab.com/eula?lang=en). |
| **Turret model** | EdwinCGStudio | [Gun Turret](https://www.fab.com/listings/1631d645-8217-44ff-9d44-305ad1bdfe2c) | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Imported, converted, and adapted for Ore Rush. |
| **Landmine model** | Mint Chips 3D | [Landmine](https://www.fab.com/listings/5dc45326-7bd4-4c9d-8069-224e4edb3b9d) | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Imported, converted, and adapted for Ore Rush. |
| **Barrier model** | Roy Sousa | [Mojo Barrier](https://www.fab.com/listings/2d5ef463-1e87-4309-acef-7bd7406a74a4) | [Fab Standard License](https://www.fab.com/eula?lang=en). |
| **Shield model** | Artur Pokidko | [Shield](https://www.fab.com/listings/36f9f091-dbbb-483f-8aee-4625f95aa1d0) | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/). Imported, converted, and adapted for Ore Rush. |
| **Spike trap model** | Nichgon | [Spike Trap](https://www.fab.com/listings/8b8eb6de-9f5a-4f81-b610-79cfd07d1961) | [Fab Standard License](https://www.fab.com/eula?lang=en). |
| **Forest environment** | Orasot | [Low Poly Forest](https://www.fab.com/listings/f36b3bd7-3e8d-48d1-bfcd-bcc2bc0bb919) | [Fab Standard License](https://www.fab.com/eula?lang=en). |
| **Smoke VFX** | FX Cat UA | [Realistic Starter VFX Pack Vol 2](https://www.fab.com/listings/ac2818b3-7d35-4cf5-a1af-cbf8ff5c61c1) | [Fab Standard License](https://www.fab.com/eula?lang=en). |
| **Depot mine cart** | Quixel Megascans | [Mine Cart](https://www.fab.com/listings/67026011-cc6a-4b2e-a669-2745e6d8c9ca) | Applicable Fab / Quixel license. |
| **Mud surface** | Quixel Megascans | [Soil Mud](https://www.fab.com/listings/c33cb641-e9bd-4966-adc3-c0b5a937ab12) | Applicable Fab / Quixel license. |

> **Public repository notice:** Standard/Marketplace-licensed source assets and locally generated derivatives of those assets are intentionally omitted from this repository and its Git history. Obtain them from their original listings and follow [ASSET_SETUP.md](ASSET_SETUP.md). This credit section does not grant redistribution rights.

## 📝 Changelog

- **16/06/2026** — Added the third-person miner character: movement, camera, jump, and server-authoritative **dash**.
- **16/06/2026** — Added the ore system: 3 vein types (iron / gold / diamond), look-and-mine mechanic, and the wallet (carrying + slow-down when full).
- **16/06/2026** — Added depots: deposit your wallet into your team vault to score (server-authoritative, replicated team scores).
- **17/06/2026** — Added match flow: automatic team assignment (host Red / joiner Blue), score quota, and win condition — the first team to fill the quota wins.
- **17/06/2026** — Mining now holds the miner in place; movement returns when you release or finish mining.
- **17/06/2026** — Added the economy and traps: spend wallet ore to build snare, mud, decoy, and smoke traps (server-authoritative, with placement cooldown and active limit). Snare stuns and drops your ore as a pickable, mud slows, decoy disguises as ore, smoke leaves a vision-blocking cloud.
- **17/06/2026** — Added depot raiding and defenses: a shared interaction interface routes look-and-hold for both mining and raiding; raid an enemy vault to steal score over time, with a "busted" reversal when the owner defends. Defenses (a blocking barrier wall and a stunning turret with projectile) and nearby-defense reinforcement reduce raid losses.
- **17/06/2026** — Added power-ups: timed pickups that respawn on a cooldown — speed, carry capacity, mining speed, and a shield that grants trap immunity (server-applied, replicated).
- **17/06/2026** — Added the procedural map generator: a per-match random seed deterministically scatters iron/gold/diamond veins, two depots, and power-ups with minimum spacing; the server spawns and replicates so both players share the same map, and each player spawns at their own depot.
- **18/06/2026** — Added the reveal power-up, configurable trap/defense lifetimes, per-player cosmetic color, and HUD data getters (selected trap, active count, buff timers).
- **18/06/2026** — Added Steam sessions (host / find / join with no IP typing, plus Steam invites) with a direct-IP/LAN fallback, and the main-menu framework (menu gamemode + controller that shows the menu and frees the cursor).
- **18/06/2026** — Added a lobby map with a ready zone: while both players wait they can mine for fun, and once both stand in the zone a replicated 5-second countdown sends everyone to the match map.
- **19/06/2026** — Added event-driven HUD hooks: buff icons and the selected-trap label now update via delegates (OnBuffChanged / OnSelectionChanged) instead of polling.
- **19/06/2026** — Fixed online match flow: teams persist across seamless travel, players reliably spawn at their own depot (deferred retry), the lobby countdown cancels if a player leaves, and the win quota is set to 50.
- **19/06/2026** — Fixed replication so clients see correct ore types (gold/diamond no longer look like iron), and turrets now fire on an enemy already standing in range when armed.
- **19/06/2026** — Reworked the economy: carry capacity is now worth-based (diamonds take more space) with a stronger full-load slowdown, and spending refunds iron change instead of overpaying.
- **22/06/2026** — Added ore vein meshes and made vein spawn counts scale with the generated map area.
- **23/06/2026** — Fixed the mining duration, wired miner animations, added the end screen widget, and removed the reveal power-up (4 power-ups remain).
- **02/07/2026** — Added marketplace (Fab) meshes — turret, landmine, barrier, shield, spike trap, coin — and wired them to the traps, defenses and power-ups.
- **02/07/2026** — Added background music to the Player Controller and mining hit sounds to the ore veins.
- **02/07/2026** — Added Niagara smoke VFX to the Smoke Cloud and integrated LowPolyForest environment assets (rocks, trees, etc.) into the Map Generator decoration pool.
- **02/07/2026** — Implemented dynamic team skins and materials (Red vs Blue) applied automatically on character initialization.
- **02/07/2026** — Added height tracing and slope angle constraints (MaxSlopeAngle = 30) to the Map Generator to guarantee all spawned actors spawn on traversable terrain.
- **05/07/2026** — The Map Generator now detects the river from the water mesh and places the two depots on opposite banks (Red one side, Blue the other) with a minimum separation, so teams no longer spawn too close together.
- **05/07/2026** — Added the hosting screen widget and updated the HUD, main menu, and session row widgets.

