# Third-Party Asset Setup

This public repository intentionally excludes source files whose marketplace licenses do not permit standalone redistribution. The C++ and Blueprint gameplay implementation remains available for portfolio review, but the project is not visually complete until licensed users obtain and restore the omitted assets themselves.

Buying or downloading an asset from its original listing does not change this repository's license. Each asset remains governed by its own listing, license, and acquisition entitlement.

## Omitted source assets

Install or migrate each asset into the exact Unreal virtual path shown below so the existing serialized references can resolve.

| Asset | Original listing | Expected Unreal path | Referenced by |
|-------|------------------|----------------------|---------------|
| Cube World Powerup Coin - Proto Series | [Fab](https://www.fab.com/listings/e25090d3-eefe-4e0b-b908-91c14c86daff) | `/Game/Models/Fab/Cube_World_Powerup_Coin_-_Proto_Series/` | Speed and mining-speed power-ups |
| Mojo Barrier | [Fab](https://www.fab.com/listings/2d5ef463-1e87-4309-acef-7bd7406a74a4) | `/Game/Models/Fab/Mojo_Barrier/` | `BP_Barrier` |
| Spike Trap | [Fab](https://www.fab.com/listings/8b8eb6de-9f5a-4f81-b610-79cfd07d1961) | `/Game/Models/Fab/Spike_Trap/` | `BP_SnareTrap` |
| Low Poly Forest | [Fab](https://www.fab.com/listings/f36b3bd7-3e8d-48d1-bfcd-bcc2bc0bb919) | `/Game/Models/LowPolyForest/` | `MainMap` environment and landscape |
| Realistic Starter VFX Pack Vol 2 | [Fab](https://www.fab.com/listings/ac2818b3-7d35-4cf5-a1af-cbf8ff5c61c1) | `/Game/Models/Realistic_Starter_VFX_Pack_Vol2/` | `BP_Smoke` |
| Mine Cart | [Fab / Quixel Megascans](https://www.fab.com/listings/67026011-cc6a-4b2e-a669-2745e6d8c9ca) | `/Game/Models/Fab/Megascans/3D/Mine_Cart_ufmodhpfa/` | `BP_DepotZone` |
| Soil Mud | [Fab / Quixel Megascans](https://www.fab.com/listings/c33cb641-e9bd-4966-adc3-c0b5a937ab12) | `/Game/Models/Fab/Megascans/Surfaces/Soil_Mud_pjuph20/` | `BP_MudTrap` |

## Locally generated coin variants

`/Game/Maps/_GENERATED/dalme/` contained modified meshes derived from the power-up coin pack, so it is also omitted. After installing the original coin pack, recreate suitable Speed and Mining variants locally or assign another properly licensed mesh in `BP_SpeedPowerUp` and `BP_MiningSpeedPowerUp`.

## Included third-party assets

The Miner, Gun Turret, Landmine, and Shield source files remain in the public repository under CC BY 4.0. Their creators, source URLs, license links, and modification notices are recorded in the main [README](README.md#-third-party-assets).

## Other omitted content

`/Game/Sound/` is omitted because the redistribution provenance of the background music and mining sound has not been documented. Supply audio you created or separately licensed for source redistribution, then update the sound assignments in `BP_OreRushPlayerController` and `BP_OreVein`.

`/Game/LevelPrototyping/` contains unused Unreal template/prototyping content and is omitted from the portfolio source export as a precaution.

The local menu image imports at `/Game/Input/MainMenu` and `/Game/Input/MainMenuLogo` are excluded until their source and reuse terms are documented.

Do not commit locally restored Fab, Marketplace, Quixel, or unverified source assets to the public repository. The ignore rules are a safety net, not a substitute for reviewing the license of every newly added asset.
