# ROADMAP-FLEX

Goal: a complete, spec-faithful CSS flexbox layout system for versatile GUI development, integrated into the existing `Core` / `Input` / `System` / `Graphics` architecture. Font/text rendering is explicitly deferred.

---

## Phase 0 — Types & Data Model
- [x] `docs/discussion/flex-design.md` review
- [x] `src/Layout/Rect.h` — `Rect` and `Size` value types
- [x] `src/Layout/BoxModel.h` — margin/padding/border in rect form
- [x] `src/Layout/FlexStyle.h` — typed flex properties, including `gap`, `min/max`, `basis`
- [x] `src/Layout/LayoutNode.h` — tree node combining `BoxModel`, `FlexStyle`, children, and computed output

## Phase 1 — Layout Engine Core
- [x] `src/Layout/FlexLayoutEngine.h/.cpp`
  - [x] `computeLayout(LayoutNode&, float availableWidth, float availableHeight)`
  - [x] Row/column main-axis resolution
  - [x] Flex grow/shrink/basis distribution
  - [ ] Wrap handling
  - [x] Justify content alignment
  - [ ] Align items / align content (cross-axis stretch is partial)
  - [x] Min/max constraint clamping
  - [x] Gap insertion
  - [ ] Intrinsic size estimation for unknown-size children

## Phase 2 — Render Integration & Debugging
- [x] `src/Graphics/RenderCommand.h` — command types for rects and later primitives
- [x] `src/Graphics/RenderCommandBuilder.h/.cpp` — `buildRenderTree()` flattens computed layout into draw commands
- [x] `src/Layout/LayoutDumper.h/.cpp` — full tree dump with issue detection (negative size, box bleed, width/height over max, grow zero result)
- [x] `main.cpp` demo that computes layout and prints computed rects + debug dump

## Phase 3 — Validation
- [ ] Unit-style tests or self-check demo cases:
  - [ ] Row with flex-grow split
  - [ ] Column reverse
  - [ ] Wrap + gap
  - [ ] Center alignment
  - [ ] Min/max constraints
- [ ] Compare computed outputs against expected CSS flexbox behavior

## Phase 4 — Future (out of scope for now)
- [ ] Vulkan renderer integration for colored rects
- [ ] CSS cascade / selector matching
- [ ] Dirty layout / incremental recompute
- [ ] Animation / transition hooks
- [ ] Grid, block, inline layout modes

---

## Notes
- Layout is headless: engine knows nothing about `Window`, `Vulkan`, or `Event`.
- `LayoutNode` owns computed output fields (`computedRect`, `measuredSize`) so the engine mutates the tree in place.
- `gap` is treated as first-class spacing, not margin hacks.
- All units are float pixels; no DPI scaling yet.
- `LayoutDumper` supports full tree inspection with `LayoutDumpOptions` to include/exclude box model, flex style, computed rect, measured size, children, and detected issues.
