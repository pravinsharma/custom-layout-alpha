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
  - [x] Wrap handling with line tracking
  - [x] Justify content alignment
  - [x] Align items / align content (cross-axis stretch, center, flex-end)
  - [x] Min/max constraint clamping
  - [x] Gap insertion
  - [x] Intrinsic size estimation for unknown-size children

## Phase 2 — Render Integration & Debugging
- [x] `src/Graphics/RenderCommand.h` — command types for rects and later primitives
- [x] `src/Graphics/RenderCommandBuilder.h/.cpp` — `buildRenderTree()` flattens computed layout into draw commands
- [x] `src/Layout/LayoutDumper.h/.cpp` — full tree dump with issue detection (negative size, box bleed, width/height over max, grow zero result)
- [x] `main.cpp` demo that computes layout and prints computed rects + debug dump

## Phase 3 — Validation
- [x] Validation harness in `src/Layout/FlexTests.h/.cpp` using `LayoutDumper` output to assert pass/fail
- [x] Each test case:
  - [x] Builds a `LayoutNode` tree
  - [x] Runs `FlexLayoutEngine::computeLayout`
  - [x] Dumps layout with `LayoutDumper`
  - [x] Asserts expected rects, order, and no issues
  - [x] Prints clear PASS/FAIL with dump on failure
- [x] Test cases implemented and passing:
  - [x] Row with flex-grow split (1:2:1)
  - [x] Column reverse
  - [x] Wrap + gap
  - [x] Center alignment
  - [x] Min/max constraints
  - [x] Box bleed / negative content area
  - [x] Zero-size child handling
- [x] Compare computed outputs against expected CSS flexbox behavior

## Phase 3.5 — Positioning, Z-Index, and Opacity
- [x] `src/Layout/Positioning.h` — `Position` enum (`Static`, `Relative`, `Absolute`, `Fixed`, `Sticky`) and `Positioning` struct (`type`, `top/right/bottom/left`, `zIndex`)
- [x] `src/Layout/FlexStyle.h` — added `position`, `zIndex`, `opacity` fields for CSS parsing
- [x] `src/Layout/LayoutNode.h` — added `Positioning positioning`, `float opacity`, and `isInFlexFlow()` helper
- [x] `src/Layout/FlexStyle.cpp` — CSS property parsing for `position`, `z-index`, and `opacity`
- [x] `src/Layout/FlexLayoutEngine.cpp` — copies positioning/opacity from `FlexStyle` to `LayoutNode` in `computeLayout()`; `Absolute`/`Fixed` children excluded from `flexGrow`/`flexShrink` totals and positioned relative to containing block
- [x] `src/Graphics/RenderCommand.h` — added `float opacity` and `int32_t zIndex`
- [x] `src/Graphics/RenderCommandBuilder.cpp` — propagates `opacity` and `zIndex` from `LayoutNode` to `RenderCommand`

## Phase 4 — Future (out of scope for now)
- [x] Vulkan renderer integration for colored rects
  - [x] `src/Graphics/VulkanRenderer.h/.cpp` — Vulkan instance, device, swapchain, render pass, pipeline, framebuffers, command buffers, sync objects
  - [x] `src/Graphics/shaders/quad.vert` / `quad.frag` — vertex/fragment shaders for colored rectangles
  - [x] `buildRenderTree()` outputs `RenderCommand::Type::Rect` consumed by renderer
  - [x] `main.cpp` wired to render computed layout in a window
- [ ] CSS cascade / selector matching
- [ ] Dirty layout / incremental recompute
- [ ] Animation / transition hooks
- [ ] Grid, block, inline layout modes
- [ ] `StackingContext` class for explicit layering and sorting
- [ ] Transform support

---

## Notes
- Layout is headless: engine knows nothing about `Window`, `Vulkan`, or `Event`.
- `LayoutNode` owns computed output fields (`computedRect`, `measuredSize`) so the engine mutates the tree in place.
- `gap` is treated as first-class spacing, not margin hacks.
- All units are float pixels; no DPI scaling yet.
- `LayoutDumper` supports full tree inspection with `LayoutDumpOptions` to include/exclude box model, flex style, computed rect, measured size, children, and detected issues.
