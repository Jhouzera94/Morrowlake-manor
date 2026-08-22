# Performance optimization log

## Baseline — 2026-08-20

- Engine: Unreal Engine 5.6.1, Development Editor, DX12/SM6 project.
- Hardware reported by Unreal: Intel Core i7-12700K, 64 GB system RAM, 10 GB-class GPU (10,184 MB reported); 7,129 MB default texture pool.
- Content inventory: 4,950 files, 8.33 GB; 4,935 `.uasset` files and 11 maps.
- Largest observed files: `Showcase_BuiltData.uasset` 328.5 MB, `PlaneCutOtherPart_2A3261FC.uasset` 316.2 MB, `SM_ENERGY_TOWER.uasset` 295.7 MB, `T_Cheminee_Normal.uasset` 218.0 MB, `SM_tower_energy.uasset` 199.5 MB, and one bird-song asset 71.6 MB.
- Latest editor log: 2,601 warnings, including 674 missing-package load warnings and many static-mesh warnings. One audio-device critical warning was also present.
- Current source builds successfully before optimization.
- Starting CSV capture before RHI initialization causes an Unreal assertion (`IsRayTracingAllowed() may only be called once RHI is initialized`), so subsequent runs omit the early CSV flag.
- First repeatable rendered pass: 60.83 seconds to load the main map, about 700 unprecached PSO hitches, repeated 100–200 ms PSO waits, 650 missing-package warnings, five SM6 material compilation failures, and two handled delegate-binding ensures. The benchmark rendered 901 frames in its fixed 30-second window; because `-benchmark` uses a deterministic timestep, this is not treated as a measured 30 FPS result.

## Batch 1 — event-driven CPU work

- Disabled empty actor ticks in `AInteractableComponent`, `AInventoryComponent`, and `AMyActor`.
- Disabled `ADestinationTerminal` polling because its overlap-scoped input bindings already handle the same interaction; this also prevents duplicate invocation in a frame.
- Cached energy-tower display values so unchanged progress/text/status no longer invalidates Slate every frame.

## Batch 2 — frame pacing

- Enabled component/resource PSO precaching and deferred proxy creation until required PSOs are ready.
- These settings retain the same shaders and materials while moving compilation away from first visible use.
- Unreal 5.6 ignored the DX12 disk-cache switches from project-level configuration and kept them disabled, so those ineffective settings were removed rather than left as misleading configuration. They can still be evaluated later through engine-level configuration or packaged launch arguments.

## Verification

- Both optimization batches compile and link successfully as an Unreal Development Editor target.
- A 30-second rendered DX12/SM6 run loaded and exited normally after the changes.
- A second rendered warm run reduced map load from 60.83 seconds to 23.40 seconds (61.5%) and logged zero first-use PSO hitches or PSO waits, versus about 700 hitches and repeated 100–200 ms waits on the first run. This is explicitly a warm-cache comparison; derived-data warmth contributes to the improvement.
- Remaining runtime errors are pre-existing content/delegate issues: 650 missing packages, five failed SM6 materials, two Energy Tower overlap delegate ensures, invalid Nanite/translucent combinations, and a material-instance request on an invalid slot.

## Intentionally deferred

- No assets removed: the working tree contains active additions/deletions and apparently unused assets may be progression-triggered.
- No global quality reductions beyond settings already present before this pass.
- Missing furniture dependencies were not guessed or redirected; they require reference-aware repair in the editor.
- Nanite was not bulk-disabled on translucent meshes because that changes stored binary assets and needs visual review; the affected assets are listed in runtime logs.
- Five invalid materials were not automatically rewired because changing texture sampler/material-function connections without inspecting the graphs could alter appearance.
