For a GUI app, the cleanest way to keep **Window, Event, Keyboard, Mouse, Console, Font, etc. classes separate** is to design around **layered responsibilities** and **decoupled subsystems**. Think of it as building a rendering engine with modular input/output channels.

---

## 🏛️ Suggested Architecture

### 1. **Core Layer (Platform Abstraction)**
- **Window**: Abstract interface for window creation, resizing, and lifecycle.
- **Event**: Base event class with type + payload (e.g., `EventType::KeyPress`, `EventType::MouseMove`).
- **Keyboard / Mouse**: Input device abstractions that generate events.
- **Console**: Optional logging/debugging interface, separate from rendering.

👉 These classes don’t know about rendering or fonts. They just expose raw system functionality.

---

### 2. **Event System (Dispatcher)**
- Central **EventDispatcher** or **EventBus**.
- Devices (Keyboard, Mouse, Window) push events into the dispatcher.
- Application components subscribe to events they care about.
- Keeps input handling **decoupled** from rendering logic.

---

### 3. **Rendering Layer**
- **Font**: Encapsulates font loading, glyph rasterization/shaping (via FreeType/HarfBuzz).
- **Renderer**: Knows how to draw primitives, text, images.
- **Surface/Canvas**: Abstraction over GPU context (OpenGL/Vulkan/DirectX).

👉 Fonts are separate because they’re a resource, not a device. Renderer consumes them.

---

### 4. **Application Layer**
- **UI Components** (Button, TextBox, Label, etc.) subscribe to events.
- They use **Renderer + Font** to draw themselves.
- They don’t directly talk to Keyboard/Mouse — they only receive events.

---

### 5. **Utilities**
- **Console**: Logs events, debug info, errors.
- **Resource Manager**: Keeps fonts, textures, shaders organized.
- **Configuration**: Handles app settings.

---

## 🔄 Flow Example
1. **Keyboard** detects a key press → emits `KeyEvent`.
2. **EventDispatcher** routes it to subscribed UI components.
3. **TextBox** receives the event → updates its text buffer.
4. **Renderer** + **Font** draw updated text in the **Window**.
5. **Console** logs `"Key pressed: A"`.

---

## 🎯 Benefits of Separation
- **Testability**: You can unit-test Keyboard without touching Renderer.
- **Extensibility**: Swap out Vulkan for OpenGL without rewriting event code.
- **Maintainability**: Each subsystem evolves independently.
- **Cross-platform**: Window/Input layer can be replaced per OS, while Renderer/UI stays portable.

---

Here’s a clean **ASCII architecture diagram** showing how to keep `Window`, `Event`, `Keyboard`, `Mouse`, `Console`, `Font`, etc. classes separate but connected through a layered design:

```
+---------------------------------------------------+
|                   Application Layer               |
|---------------------------------------------------|
|  + UIComponent (Button, TextBox, Label, etc.)     |
|  + Subscribes to Events                           |
|  + Uses Renderer + Font to draw                   |
+-------------------------+-------------------------+
                          |
                          v
+---------------------------------------------------+
|                 Rendering Layer                   |
|---------------------------------------------------|
|  + Renderer (Vulkan/OpenGL/DirectX)               |
|  + Canvas/Surface (GPU Context)                   |
|  + Font (FreeType/HarfBuzz glyph shaping)         |
+-------------------------+-------------------------+
                          |
                          v
+---------------------------------------------------+
|                 Event System Layer                |
|---------------------------------------------------|
|  + EventDispatcher / EventBus                     |
|  + Routes events from devices to subscribers      |
|  + Event (Base class: type + payload)             |
+-------------------------+-------------------------+
                          |
                          v
+---------------------------------------------------+
|               Core Platform Layer                 |
|---------------------------------------------------|
|  + Window (create, resize, lifecycle)             |
|  + Keyboard (raw key input → KeyEvent)            |
|  + Mouse (raw mouse input → MouseEvent)           |
|  + Console (logging/debugging interface)          |
+---------------------------------------------------+
```

---

### 🔄 Example Flow
```
Keyboard ---> KeyEvent ---> EventDispatcher ---> TextBox ---> Renderer+Font ---> Window
Mouse ------> MouseEvent -> EventDispatcher ---> Button ----> Renderer+Font ---> Window
```

---

### 🧩 Key Separation Principles
- **Window**: Only manages OS-level windowing, no rendering logic.
- **Event**: Abstract base, extended by `KeyEvent`, `MouseEvent`, `WindowEvent`.
- **Keyboard/Mouse**: Input sources, never tied to UI directly.
- **Console**: Debug/logging, independent of rendering.
- **Font**: Resource class, consumed by Renderer/UI, not tied to input.
- **EventDispatcher**: Decouples input sources from UI components.

---

Perfect — here’s a **C++-style ASCII class hierarchy diagram** that shows how the subsystems are separated but connected. I’ve included inheritance arrows (`^`) and composition arrows (`-->`) to make the relationships clear.

---

## 🏛️ Class Hierarchy (ASCII UML-style)

```
                   +-------------------+
                   |      Event        |
                   +-------------------+
                   | type              |
                   | payload           |
                   +-------------------+
                     ^        ^        ^
                     |        |        |
        +------------+   +----+----+   +-------------+
        |                | KeyEvent |   | MouseEvent |
        |                +----------+   +-------------+
        |                | keyCode  |   | x, y, btn  |
        |                +----------+   +-------------+
        |
+-------------------+
|   WindowEvent     |
+-------------------+
| resize, close     |
+-------------------+


+-------------------+        +-------------------+
| Keyboard          |        | Mouse             |
+-------------------+        +-------------------+
| pollInput()       |        | pollInput()       |
+-------------------+        +-------------------+
        |                          |
        | emits KeyEvent            | emits MouseEvent
        |                          |
        v                          v
+-------------------+        +-------------------+
| EventDispatcher   |<-------+-------------------+
+-------------------+        | Window            |
| registerListener()|        +-------------------+
| dispatch(Event&)  |        | create(), resize()|
+-------------------+        +-------------------+
        |
        v
+-------------------+
| UIComponent       |<-----------------------------+
+-------------------+                              |
| virtual handleEvent(Event&) = 0;                 |
| draw(Renderer&)                                  |
+-------------------+                              |
        ^                                          |
        |                                          |
+-------------------+   +-------------------+      |
| Button            |   | TextBox           |      |
+-------------------+   +-------------------+      |
| onClick()         |   | buffer            |      |
+-------------------+   +-------------------+      |
        |                                          |
        | uses Renderer + Font --------------------+
        v
+-------------------+
| Renderer          |
+-------------------+
| drawText(Font&)   |
| drawShape()       |
+-------------------+
        |
        v
+-------------------+
| Font              |
+-------------------+
| loadGlyphs()      |
| shapeText()       |
+-------------------+

+-------------------+
| Console           |
+-------------------+
| log(message)      |
+-------------------+
```

---

### 🔑 Key Points
- **Event** is the abstract base; specialized events (`KeyEvent`, `MouseEvent`, `WindowEvent`) inherit from it.
- **Keyboard/Mouse/Window** generate events → push into **EventDispatcher**.
- **EventDispatcher** routes events to **UIComponents** (e.g., `Button`, `TextBox`).
- **UIComponents** implement `handleEvent(Event&)` and use **Renderer + Font** for drawing.
- **Console** is independent, used for logging/debugging.

---

This diagram doubles as a **header outline** for your C++ classes. For example:

```cpp
class Event { /* base */ };
class KeyEvent : public Event { /* keyCode */ };
class MouseEvent : public Event { /* x,y,btn */ };
class WindowEvent : public Event { /* resize, close */ };

class EventDispatcher {
    void registerListener(UIComponent*);
    void dispatch(Event&);
};

class UIComponent {
    virtual void handleEvent(Event&) = 0;
    virtual void draw(Renderer&) = 0;
};

class Button : public UIComponent { /* onClick */ };
class TextBox : public UIComponent { /* buffer */ };

class Renderer { void drawText(Font&); void drawShape(); };
class Font { void loadGlyphs(); void shapeText(); };

class Console { void log(const std::string&); };
```

---

Here’s a **namespace/module-oriented ASCII diagram** that shows how you could organize your GUI app into separate compilation units. This way, `Window`, `Event`, `Keyboard`, `Mouse`, `Console`, `Font`, etc. live in their own domains but interact cleanly.

---

## 🗂️ Namespaces & Modules Layout

```
Core::Window
+-------------------+
| class Window      |
| - create()        |
| - resize()        |
| - close()         |
+-------------------+

Core::Event
+-------------------+
| class Event       |
| - type            |
| - payload         |
+-------------------+
       ^        ^        ^
       |        |        |
 Input::KeyEvent   Input::MouseEvent   Core::WindowEvent


Input::Keyboard
+-------------------+
| class Keyboard    |
| - pollInput()     |
| - emit KeyEvent   |
+-------------------+

Input::Mouse
+-------------------+
| class Mouse       |
| - pollInput()     |
| - emit MouseEvent |
+-------------------+


System::Console
+-------------------+
| class Console     |
| - log(message)    |
| - warn(message)   |
+-------------------+


Graphics::Font
+-------------------+
| class Font        |
| - loadGlyphs()    |
| - shapeText()     |
+-------------------+

Graphics::Renderer
+-------------------+
| class Renderer    |
| - drawText(Font&) |
| - drawShape()     |
+-------------------+

Graphics::Canvas
+-------------------+
| class Canvas      |
| - GPU context     |
| - swapBuffers()   |
+-------------------+


UI::Component
+-------------------+
| class UIComponent |
| - handleEvent()   |
| - draw(Renderer&) |
+-------------------+
       ^                  ^
       |                  |
UI::Button           UI::TextBox
```

---

## 🔄 Event Flow Across Namespaces

```
Input::Keyboard ---> Core::Event(KeyEvent) ---> Core::EventDispatcher ---> UI::TextBox
Input::Mouse ------> Core::Event(MouseEvent) -> Core::EventDispatcher ---> UI::Button
Core::Window ------> Core::Event(WindowEvent) -> Core::EventDispatcher ---> UI::Component
```

---

## 📦 Compilation Units
- **Core/** → `Window.h`, `Event.h`, `EventDispatcher.h`
- **Input/** → `Keyboard.h`, `Mouse.h`
- **Graphics/** → `Renderer.h`, `Font.h`, `Canvas.h`
- **UI/** → `Component.h`, `Button.h`, `TextBox.h`
- **System/** → `Console.h`

Each namespace maps to a folder, keeping headers and implementations isolated. Dependencies flow **upward** (Input → Core → UI → Graphics), never sideways.

---

This structure makes it easy to swap out subsystems (e.g., replace `Graphics::Renderer` with Vulkan or OpenGL) without touching `Input` or `UI`.  

Here’s a **dependency arrow diagram (ASCII)** showing how the modules/namespaces depend on each other. This illustrates compile-time coupling and makes it clear which layers can be swapped or extended without breaking others.

---

## 📐 Dependency Diagram

```
   +-------------------+
   |   Input::Keyboard |
   |   Input::Mouse    |
   +-------------------+
            |
            v
   +-------------------+
   |   Core::Event     |
   |   Core::Window    |
   +-------------------+
            |
            v
   +-------------------+
   | Core::Dispatcher  |
   +-------------------+
            |
            v
   +-------------------+
   |   UI::Component   |
   |   UI::Button      |
   |   UI::TextBox     |
   +-------------------+
            |
            v
   +-------------------+
   | Graphics::Renderer|
   | Graphics::Font    |
   | Graphics::Canvas  |
   +-------------------+
            |
            v
   +-------------------+
   | System::Console   |
   +-------------------+
```

---

### 🔄 Explanation of Arrows
- **Input → Core**: Input devices generate events that depend on `Core::Event`.
- **Core → Dispatcher**: Events are routed by the dispatcher.
- **Dispatcher → UI**: UI components subscribe to events.
- **UI → Graphics**: UI components use Renderer + Font to draw.
- **Graphics → Console**: Renderer/Font log debug info to Console (optional).

---

### ⚖️ Compile-Time Coupling Rules
- **Input** depends only on **Core::Event** (not Renderer/UI).
- **Core** depends only on OS/platform APIs (Window system).
- **UI** depends on **Core::Event** and **Graphics**.
- **Graphics** is self-contained (Renderer, Font, Canvas).
- **System::Console** is independent, used by all layers for logging.

---

This separation ensures:
- You can replace **Graphics::Renderer** (e.g., Vulkan → OpenGL) without touching Input or Core.
- You can swap **Core::Window** for another OS backend without rewriting UI.
- **Console** remains optional and doesn’t pollute other modules.

---

Here’s the **reverse dependency diagram (runtime flow)** — showing *who uses whom* during execution, not just compile-time references. This highlights the actual control flow when the app is running.

---

## 🔄 Runtime Dependency Flow

```
User Input
   |
   v
+-------------------+
| Input::Keyboard   |
| Input::Mouse      |
+-------------------+
   | emits events
   v
+-------------------+
| Core::Event       |
+-------------------+
   | packaged + sent
   v
+-------------------+
| Core::Dispatcher  |
+-------------------+
   | notifies listeners
   v
+-------------------+
| UI::Component     |
| (Button, TextBox) |
+-------------------+
   | updates state
   v
+-------------------+
| Graphics::Renderer|
| Graphics::Font    |
| Graphics::Canvas  |
+-------------------+
   | draws to screen
   v
+-------------------+
| Core::Window      |
+-------------------+
   | presents frame
   v
User sees output
```

---

### 🧩 Runtime Relationships
- **Keyboard/Mouse → Event**: Devices generate raw input events.
- **Event → Dispatcher**: Events are wrapped and routed.
- **Dispatcher → UI**: UI components consume events, update state.
- **UI → Graphics**: UI requests drawing via Renderer + Font.
- **Graphics → Window**: Renderer outputs to the window surface.
- **Console**: Can be tapped at *any stage* for logging/debugging, but doesn’t affect flow.

---

### ⚖️ Key Difference vs Compile-Time
- **Compile-time** shows *dependencies* (who includes whose headers).
- **Runtime** shows *control flow* (who calls whom during execution).
- Example: `UI::Button` *depends* on `Core::Event` headers at compile-time, but at runtime it *receives* events from `Dispatcher`.

---

This runtime view makes it easier to reason about **event propagation** and **rendering loops**.  

Here’s how the **main loop structure** ties all those subsystems together — expressed in ASCII pseudocode so you can see the runtime orchestration clearly:

---

## 🌀 Main Loop Structure

```
int main() {
    Core::Window window;
    Input::Keyboard keyboard;
    Input::Mouse mouse;
    Core::EventDispatcher dispatcher;
    Graphics::Renderer renderer;
    Graphics::Font font("Arial.ttf");
    System::Console console;

    // Register UI components
    UI::TextBox textBox;
    UI::Button  button;
    dispatcher.registerListener(&textBox);
    dispatcher.registerListener(&button);

    // Main loop
    while (window.isOpen()) {
        //---------------------------------------
        // 1. Poll Input
        //---------------------------------------
        keyboard.pollInput(dispatcher);  // emits KeyEvent
        mouse.pollInput(dispatcher);     // emits MouseEvent
        window.pollEvents(dispatcher);   // emits WindowEvent

        //---------------------------------------
        // 2. Dispatch Events
        //---------------------------------------
        dispatcher.dispatchAll();        // routes to UI components

        //---------------------------------------
        // 3. Update UI State
        //---------------------------------------
        textBox.update();
        button.update();

        //---------------------------------------
        // 4. Render Frame
        //---------------------------------------
        renderer.beginFrame(window);
        textBox.draw(renderer, font);
        button.draw(renderer, font);
        renderer.endFrame(window);

        //---------------------------------------
        // 5. Debug/Logging
        //---------------------------------------
        console.log("Frame rendered");
    }

    return 0;
}
```

---

## 🔄 Execution Flow
1. **Poll Input** → Keyboard, Mouse, Window generate raw events.
2. **Dispatch Events** → EventDispatcher routes them to subscribed UI components.
3. **Update UI State** → Components react (TextBox updates buffer, Button checks clicks).
4. **Render Frame** → Renderer + Font draw updated UI into the Window surface.
5. **Console Logging** → Debug info printed each frame.

---

## ⚖️ Design Notes
- Each subsystem is **isolated**: Input doesn’t know about Renderer, UI doesn’t know about OS window internals.
- The **dispatcher** is the glue — it decouples event sources from consumers.
- The **main loop** is the orchestrator — it sequences input, event handling, UI updates, and rendering.

---

This loop is the **heart of every GUI engine** — from SDL to Qt to custom Vulkan renderers.  

Here’s a **multi-threaded main loop design (ASCII pseudocode)** showing how input, event dispatch, and rendering can run in parallel for performance. This is a common pattern in modern GUI engines and game loops.

---

## 🧵 Multi-Threaded Loop Structure

```
+-------------------+       +-------------------+       +-------------------+
| Input Thread      |       | Event Thread      |       | Render Thread     |
+-------------------+       +-------------------+       +-------------------+
| while(running) {  |       | while(running) {  |       | while(running) {  |
|   keyboard.poll();|       |   dispatcher.     |       |   renderer.begin();|
|   mouse.poll();   |       |   dispatchAll();  |       |   ui.draw();       |
|   window.poll();  |       |   ui.update();    |       |   renderer.end();  |
|   push events --> |       |                   |       |   swapBuffers();   |
|   eventQueue      |       |                   |       |                   |
| }                 |       | }                 |       | }                 |
+-------------------+       +-------------------+       +-------------------+
```

---

## 🔄 Flow Between Threads

```
Input Thread ---> EventQueue ---> Event Thread ---> UI State ---> Render Thread ---> Window
```

- **Input Thread**: Polls raw OS input (Keyboard, Mouse, Window events) and pushes them into a thread-safe `EventQueue`.
- **Event Thread**: Consumes events from the queue, dispatches them to UI components, updates their state.
- **Render Thread**: Reads updated UI state, draws via Renderer + Font, presents frame to Window.

---

## ⚖️ Synchronization Points
- **EventQueue**: Must be thread-safe (e.g., lock-free queue or mutex-protected).
- **UI State**: Needs double-buffering or synchronization so Render Thread doesn’t read half-updated state.
- **Renderer**: Runs independently, but may need synchronization with GPU context.

---

## 🚀 Benefits
- **Responsiveness**: Input is never blocked by rendering.
- **Smooth Rendering**: Renderer can run at fixed FPS (e.g., 60Hz) while events arrive asynchronously.
- **Scalability**: Heavy UI updates don’t stall input or rendering.

---

## 🧩 Example Pseudocode (Thread Creation)

```cpp
std::thread inputThread([&] {
    while(running) {
        keyboard.poll(eventQueue);
        mouse.poll(eventQueue);
        window.poll(eventQueue);
    }
});

std::thread eventThread([&] {
    while(running) {
        dispatcher.dispatchAll(eventQueue);
        ui.update();
    }
});

std::thread renderThread([&] {
    while(running) {
        renderer.beginFrame(window);
        ui.draw(renderer, font);
        renderer.endFrame(window);
        window.swapBuffers();
    }
});
```

---

This design is essentially a **producer-consumer pipeline**: Input produces events, Event Thread consumes and updates state, Render consumes state and produces frames.  

Here’s a **synchronization primitives diagram (ASCII)** showing how the multi-threaded input–event–render pipeline can safely share data without race conditions:

---

## 🔒 Synchronization Diagram

```
+-------------------+        +-------------------+        +-------------------+
|   Input Thread    |        |   Event Thread    |        |   Render Thread   |
+-------------------+        +-------------------+        +-------------------+
| poll devices      |        | consume events    |        | read UI state     |
| push -> EventQ    |        | update UI state   |        | draw frame        |
+-------------------+        +-------------------+        +-------------------+
          |                           |                           |
          v                           v                           v
   +-------------------+      +-------------------+      +-------------------+
   | EventQueue        |<---->| Mutex/CondVar     |<---->| DoubleBuffer      |
   | (lock-free queue) |      | (sync dispatcher) |      | (UI state swap)   |
   +-------------------+      +-------------------+      +-------------------+
```

---

## 🧩 Key Synchronization Mechanisms

- **EventQueue (lock-free or mutex-protected)**  
  - Input Thread pushes events.  
  - Event Thread pops events.  
  - Ensures safe producer–consumer communication.

- **Mutex + Condition Variable**  
  - Dispatcher waits for new events.  
  - Input Thread signals when events are available.  
  - Prevents busy-waiting and race conditions.

- **Double Buffering (UI State)**  
  - Event Thread writes to **back buffer** (UI state updates).  
  - Render Thread reads from **front buffer** (stable snapshot).  
  - Buffers swap atomically each frame → no tearing or half-updated state.

---

## 🔄 Example Flow with Synchronization

```
Input Thread:
   lock(EventQueue)
   push(KeyEvent)
   unlock(EventQueue)
   signal(EventAvailable)

Event Thread:
   wait(EventAvailable)
   lock(EventQueue)
   pop(Event)
   unlock(EventQueue)
   update(UIBackBuffer)
   swapBuffers()

Render Thread:
   lock(UIFrontBuffer)
   read(UI state snapshot)
   unlock(UIFrontBuffer)
   render()
```

---

## ⚖️ Benefits
- **No race conditions**: Each shared resource is protected by either a mutex or double-buffer swap.
- **No blocking rendering**: Renderer always reads a stable snapshot of UI state.
- **Efficient input handling**: Lock-free queues or condition variables keep latency low.

---

This setup is essentially the **classic producer–consumer with double buffering** — the backbone of responsive GUI and game engines.  

Here’s a **timeline diagram (ASCII Gantt-style)** showing how the three threads (Input, Event, Render) interleave over time, with synchronization points marked. This makes it clear how asynchronous input can coexist with a fixed rendering cadence.

---

## 🕒 Timeline Diagram

```
Time --->

Input Thread:   | poll | pushQ | poll | pushQ | poll | pushQ | poll | pushQ |
                ------------------------------------------------------------

Event Thread:   | waitQ | dispatch | updateUI | waitQ | dispatch | updateUI |
                ------------------------------------------------------------

Render Thread:  | begin | draw | end | swap | begin | draw | end | swap    |
                ------------------------------------------------------------

Synchronization:
   - EventQueue (between Input → Event)
   - DoubleBuffer (between Event → Render)
```

---

## 🔄 Detailed Flow Over Time

```
t0: Input polls devices → pushes KeyEvent(MouseEvent) into EventQueue
t1: Event thread wakes (CondVar) → pops events → updates UIBackBuffer
t2: Render thread begins frame → reads UIFrontBuffer → draws → swaps buffers
t3: Event thread swaps buffers (UIBackBuffer → UIFrontBuffer)
t4: Render thread begins next frame → reads updated UI state
```

---

## ⚖️ Key Observations
- **Input is asynchronous**: Events arrive at irregular intervals (user typing, moving mouse).
- **Event thread is reactive**: Wakes only when EventQueue has data, processes quickly.
- **Render thread is periodic**: Runs at fixed cadence (e.g., 60 FPS), always consumes the latest stable UI snapshot.
- **Synchronization ensures consistency**: Double buffering prevents Render from reading half-updated UI state.

---

## 🚀 Benefits of Timeline View
- Shows **latency path**: Input → Event → Render → Window.
- Highlights **parallelism**: Input can keep running even while Render is busy.
- Clarifies **sync points**: EventQueue and buffer swaps are the only places threads meet.

---

This timeline is essentially the **heartbeat of a GUI/game engine**: input events trickle in asynchronously, event handling updates state, and rendering ticks at a steady rhythm.  

Here’s the **full frame lifecycle diagram (ASCII)** showing the end-to-end pipeline: from user input → event → UI update → render → GPU → display. This ties together everything we’ve built so far.

---

## 🎬 Frame Lifecycle Diagram

```
User Input (Keyboard/Mouse)
        |
        v
+-------------------+
| Input Thread      |
| poll devices      |
| push events       |
+-------------------+
        |
        v
+-------------------+
| EventQueue        |  <-- synchronized (mutex/lock-free)
+-------------------+
        |
        v
+-------------------+
| Event Thread      |
| dispatch events   |
| update UI state   |
| write to BackBuf  |
+-------------------+
        |
        v
+-------------------+
| Double Buffer     |  <-- atomic swap
| FrontBuf / BackBuf|
+-------------------+
        |
        v
+-------------------+
| Render Thread     |
| read FrontBuf     |
| draw UI w/ Font   |
| issue GPU calls   |
+-------------------+
        |
        v
+-------------------+
| GPU Pipeline      |
| rasterize, shade  |
| compose frame     |
+-------------------+
        |
        v
+-------------------+
| Core::Window      |
| swapBuffers()     |
+-------------------+
        |
        v
Display (User sees frame)
```

---

## 🔄 Lifecycle Steps
1. **Input**: User presses a key or moves the mouse.
2. **EventQueue**: Input thread pushes events into a thread-safe queue.
3. **Event Thread**: Dispatcher consumes events, updates UI state in BackBuffer.
4. **Double Buffer Swap**: BackBuffer becomes FrontBuffer, ensuring Render sees a stable snapshot.
5. **Render Thread**: Reads FrontBuffer, uses Renderer + Font to draw UI.
6. **GPU Pipeline**: Executes draw calls (vertex → fragment → rasterization).
7. **Window**: Presents final frame via swapBuffers.
8. **Display**: User sees updated UI.

---

## ⚖️ Design Highlights
- **Decoupling**: Input, Event, Render run independently.
- **Synchronization**: EventQueue + Double Buffer prevent race conditions.
- **Performance**: Render thread runs at fixed FPS, input remains responsive.
- **Predictability**: Each frame follows the same lifecycle, making debugging easier.

---

This diagram is essentially the **canonical GUI/game engine loop** — SDL, Qt, and custom Vulkan engines all follow this pattern.  

Here’s the **GPU pipeline breakdown (ASCII)** showing how the Renderer’s draw calls flow through the graphics pipeline stages — from CPU submission to final display:

---

## 🎨 GPU Pipeline Breakdown

```
+-------------------+
| CPU (Renderer)    |
| issue draw calls  |
| bind Font/Texture |
+-------------------+
        |
        v
+-------------------+
| Command Buffer    |
| (driver queues)   |
+-------------------+
        |
        v
+-------------------+
| Vertex Processing |
| - Vertex Shader   |
| - Transform coords|
| - Apply matrices  |
+-------------------+
        |
        v
+-------------------+
| Tessellation      |
| (optional stage)  |
| - Subdivide geom  |
+-------------------+
        |
        v
+-------------------+
| Geometry Shader   |
| (optional stage)  |
| - Expand/modify   |
+-------------------+
        |
        v
+-------------------+
| Rasterization     |
| - Convert prims   |
|   to fragments    |
+-------------------+
        |
        v
+-------------------+
| Fragment Shader   |
| - Shading, text   |
| - Font sampling   |
| - Lighting, color |
+-------------------+
        |
        v
+-------------------+
| Output Merger     |
| - Blend, depth    |
| - Compose frame   |
+-------------------+
        |
        v
+-------------------+
| Framebuffer       |
| - Final image     |
+-------------------+
        |
        v
+-------------------+
| Core::Window      |
| swapBuffers()     |
+-------------------+
        |
        v
Display (User sees pixels)
```

---

## 🔄 Step-by-Step
1. **CPU (Renderer)**: Issues draw calls, binds resources (Font glyph atlas, textures).
2. **Command Buffer**: Driver queues commands for GPU execution.
3. **Vertex Processing**: Vertex shader transforms positions (model → world → screen space).
4. **Tessellation/Geometry Shader**: Optional stages for complex geometry.
5. **Rasterization**: Converts triangles into fragments (pixel candidates).
6. **Fragment Shader**: Computes color per fragment (samples font glyphs, applies shading).
7. **Output Merger**: Blends fragments, applies depth/stencil tests, composes final frame.
8. **Framebuffer**: Stores the finished image.
9. **Window Swap**: Presents framebuffer to display.

---

## ⚖️ Notes for Text Rendering
- **Font glyphs** are usually stored in a texture atlas (from FreeType/HarfBuzz).
- **Vertex Shader** positions quads for each glyph.
- **Fragment Shader** samples glyph texture, applies anti-aliasing (MSDF/MTSDF).
- **Output Merger** blends glyph fragments with background.

---

This is the **canonical GPU pipeline** — every modern graphics API (OpenGL, Vulkan, DirectX, Metal) follows this structure, with minor variations in optional stages.  

