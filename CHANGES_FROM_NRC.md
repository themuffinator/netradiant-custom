# CHANGES_FROM_NRC.md

This document captures significant, *documented* changes in VibeRadiant relative to the
NetRadiant-Custom (NRC) baseline. It is intentionally scoped to items that have explicit
documentation in this repository. If you need a full, audited diff against NRC, see the
"Open items" section at the end.

Sources used:
- `README.md`
- `RELEASING.md`
- `docs/changelog-custom.txt`
- `docs/Additional_map_editor_features.htm`
- `docs/Additional_map_compiler_features.htm`
- `docs/auto-updater.md`
- `docs/fsr_readme.txt`

## Local changes (this repo)

- Entities: added a "Drop Entities to Floor" command to drop selected point entities onto brush geometry.
- Filters: idTech2 brush filtering now honors surface/content flags for clip, hint/skip, liquids, sky, translucent, and areaportals.
- Linked Duplicates: ported TrenchBroom-style linked group duplication and syncing (create/select/separate, transformation tracking, and linked group update propagation).
- Z-bar view: added the GtkRadiant-style vertical Z ruler alongside the 2D XY view.
- Z-bar view: completed GtkRadiant parity for mouse-driven selection/texture edits, Z-constrained drags, and resize minimum sizing.
- Build fixes: exported linked group module access for plugins, adjusted Qt mouse event handling for Qt5, and cleaned up a build warning in `libs/gtkutil/image.cpp`.
- Texture browser: refreshed layout with a unified filter bar, name search, and surface/content flag filtering.
- Camera view: added a real-time lighting preview mode using point lights, surface lights, and sky/worldspawn sun keys to approximate map lighting.
- Gamepack model types: added `md5mesh` and `iqm` to explicit `modeltypes` lists so MD5/IQM models are available in the editor for non-wildcard game configs.
- Releases/updates: added a `VERSION` file, release packaging workflow with update manifest generation, and an in-app auto-updater that checks GitHub releases and installs updates (Windows zip, Linux AppImage).
- Documentation: added `RELEASING.md` for versioning/packaging/release details and `docs/auto-updater.md` for user-facing update instructions.
- Clipper tool: added a visual style option (GTK/NRC/VIBE) for clipped volume previewing, including a VIBE mode with a red dashed cut line and striped fill.
- Asset browser: added entity and sound browser tabs alongside textures, with drag-and-drop into 2D/3D views to create entities or assign `noise`/`target_speaker` sounds.
- Branding: replaced the splash screen artwork with a new 1536x1024 `splash.png`.
- Documentation/branding: refreshed `README.md` with a new `docs/viberadiant-banner.png` social banner and updated project overview/links.

## Map editor changes (documented)

From `docs/Additional_map_editor_features.htm`:
- OBJ model support with `.mtl` texture association.
- Ctrl-Alt-E expands selection to whole entities.
- Targeting lines support `target2`, `target3`, `target4`, `killtarget`.
- Rotate/Scale dialogs are non-modal.
- Four-pane view: Ctrl-Tab centers all 2D views to selection.
- Configurable strafe mode behavior.
- Regrouping entities (move brushes in/out of entities without retyping keys).
- Clone selection no longer rewrites `targetname`/`target` (Shift+Space keeps old behavior).
- Linked duplicates for synchronized group copies (create/select/separate).
- Clip tool shows a direction line indicating which half is deleted.
- Automatic game configuration when launched inside known game installs.
- Keyboard shortcuts editor (editable bindings).
- Portable mode by creating a `settings/` directory in the install.

From `docs/changelog-custom.txt`:
- `.webp` image format support.
- Rotate dialog non-modal (explicitly called out).
- `func_static` included in world filter (not hidden by entity filter).
- Texture browser text legibility improvements.
- `misc_model` supports `model2` key.
- Light style number display.
- Group entities: force arrow drawing for `func_door` and `func_button`.
- "Select Touching Tall" command (2D-touching select ignoring height).
- Option to disable dots on selected faces outside face mode.

From `README.md` (high-level editor highlights):
- WASD camera binds and 3D view editing workflow improvements.
- UV Tool, autocaulk, texture painting by drag, and texture lock support.
- MeshTex plugin, patch thicken, patch prefab alignment to active projection.
- Expanded selection/transform tools (skew, affine bbox manipulator, custom pivot).
- Extended filters toolbar and viewport navigation tweaks.
- Texture browser search improvements and transparency view option.

## Map compiler / q3map2 changes (documented)

From `docs/Additional_map_compiler_features.htm`:
- Floodlighting via `_floodlight` worldspawn key.
- `-exposure` light compile parameter.
- `q3map_alphagen dotProductScale` and `dotProduct2scale`.
- BSP stage `-minsamplesize`.
- `-convert -format ase -shadersasbitmap` for ASE prefabs.
- `-celshader` support.
- Minimap generator (Nexuiz-style).

From `docs/fsr_readme.txt` (FS-R Q3Map2 modifications):
- `-gridscale` / `-gridambientscale` for grid lighting (R5).
- Light spawnflags `unnormalized` (32) and `distance_falloff` (64) (R4).
- `_deviance` implies `_samples` (R4).
- Deluxemap fixes and `-keeplights` behavior (R3).
- Floodlight behavior adjustments and new `q3map_floodlight` parameters (R3).
- Entity normal smoothing keys `_smoothnormals`/`_sn`/`_smooth` (R2).
- `-deluxemode` and per-game defaults (R2).
- `q3map_deprecateShader` keyword (R1).
- Entity `_patchMeta`, `_patchQuality`, `_patchSubdivide` (R1).
- `MAX_TW_VERTS` increase for complex curves (R1).
- Game-type defaults and negation switches for `-deluxe`, `-subdivisions`, `-nostyles`, `-patchshadows` (R1).
- `-samplesize` global lightmap sample scaling (R1).
- `_ls` short key for `_lightmapscale` (R1).

From `README.md` (Q3Map2 feature summary):
- Shader remap improvements, lightmap brightness/contrast/saturation controls.
- `-nolm`, `-novertex`, `-vertexscale`, `-extlmhacksize`.
- Area light “backsplash” and other light pipeline updates.
- Valve220 mapformat detection and support.
- Assimp-based model loading (40+ formats).
- `-json` BSP export/import, `-mergebsp`, and shader discovery without `shaderlist.txt`.

## Open items / needs verification

This repository does not contain a direct NRC version pin or an authoritative
"baseline diff" list. To make this document exhaustive, the following inputs are needed:
- The exact NRC commit/tag this fork was based on.
- A curated list of VibeRadiant-specific commits after the fork point.

If you can provide the NRC base reference (tag/commit), I can expand this file with a
verified, diff-driven change list.
