ImGui (Immediate Mode GUI) is a library for creating user interfaces in C++. It's super popular for tool development, game editors, debugging tools, and yeah... overlays for games.

- Easy to use compared to other UI libraries
- No complex setup with designers or XML files
- Code-based UI (write code, see results immediately)
- Perfect for overlays and real-time applications
- Widely used in game development tools

## tutorial I followed
TheCherno's ImGui tutorial: https://www.youtube.com/watch?v=vWXrFetSH8w

he's a legend, explains everything clearly. highly recommend his whole C++ series :)

## key Takeaway from Cherno's Video
don't code everything from scratch! ImGui comes with example projects in the `examples/` folder. Just:
1. pick the graphics backend you want (DirectX 11, OpenGL, etc)
2. copy that example's main.cpp
3. customize it for your needs

way easier than writing all the DirectX/OpenGL boilerplate by yourself

---

## how ImGui works

### IM vs RM
Retained Mode (traditional UI):
- create UI elements once
- they exist in memory
- update them when needed
- complex state management

Immediate Mode (ImGui):
- create UI every frame
- no persistent objects
- just call functions in a loop
- way simpler

### the main loop pattern
```cpp
while (running) {
    // 1. start new frame
    ImGui::NewFrame();
    
    // 2. create your ui (this runs every frame)
    ImGui::Begin("Window Title");
    if (ImGui::Button("Click me")) {
        // do something
    }
    ImGui::End();
    
    // 3. render everything
    ImGui::Render();
    // ... DirectX/OpenGL drawing code ...
}
```

## the DirectX 11 setup (what's happening in main.cpp)

### 1. create a window
standard Windows API stuff. nothing ImGui-specific yet
- `WNDCLASSEX` = window class definition
- `CreateWindow` = actually makes the window
- `WndProc` = handles window messages (clicks, resize, etc)

### 2. initialize DirectX 11
DirectX does the actual drawing. We need:
- Device (`ID3D11Device`) - main D3D object, creates resources
- DeviceContext (`ID3D11DeviceContext`) - executes rendering commands
- SwapChain (`IDXGISwapChain`) - manages front/back buffers
- RenderTargetView - where we actually draw

i personally think of it like:
- Device = the GPU itself
- DeviceContext = a painter with a brush
- SwapChain = two canvases (one visible, one being painted)
- RenderTargetView = the current canvas we're painting on

### 3. initialize ImGui
```cpp
ImGui::CreateContext();           // create ImGui
ImGui::StyleColorsDark();         // set theme
ImGui_ImplWin32_Init(hwnd);       // connect to Windows
ImGui_ImplDX11_Init(device, ctx); // connect to DirectX
```

### 4. main Loop
every frame:
1. process windows messages
2. `ImGui::NewFrame()` - start new frame
3. Create your UI here (buttons, sliders, etc)
4. `ImGui::Render()` - finalize draw data
5. DirectX draws everything
6. `Present()` - show the frame

### 5. cleanup
release all DirectX resources when closing. important to avoid memory leaks!

## Basic ImGui Widgets (the UI elements)

### windows
```cpp
ImGui::Begin("Window Title");
// ... widgets go here
ImGui::End();
```

### text
```cpp
ImGui::Text("Hello!");
ImGui::Text("Value: %d", someVariable);
```

### buttons
```cpp
if (ImGui::Button("Click Me")) {
    // this runs when clicked
}
```

### checkboxes
```cpp
bool myBool = false;
ImGui::Checkbox("Enable Feature", &myBool);
// myBool changes automatically when clicked
```

### sliders
```cpp
float myFloat = 0.5f;
ImGui::SliderFloat("Slider", &myFloat, 0.0f, 1.0f);
```

### input fields
```cpp
char buffer[256] = "text";
ImGui::InputText("Input", buffer, sizeof(buffer));
```

### color pickers
```cpp
float color[4] = {1.0f, 0.0f, 0.0f, 1.0f}; // RGBA
ImGui::ColorEdit4("Color", color);
```

## important concepts

### static variables
```cpp
static int counter = 0;  // persists between frames!
```
Without `static`, the variable resets to 0 every frame. With `static`, it keeps its value.

### passing by reference (&)
```cpp
bool myBool = false;
ImGui::Checkbox("Test", &myBool);  // note the &
```
the `&` means ImGui can modify the variable directly. that's how it updates when you click.

### ImGuiIO
```cpp
ImGuiIO& io = ImGui::GetIO();
io.Framerate  // current FPS
io.MousePos   // mouse position
```
this object has info about input and performance.

## common patterns

### conditional windows
```cpp
bool showWindow = true;
if (showWindow) {
    ImGui::Begin("My Window", &showWindow);  // x button closes it
    ImGui::Text("Content");
    ImGui::End();
}
```

### same line widgets
```cpp
ImGui::Button("Button 1");
ImGui::SameLine();  // next widget on same line
ImGui::Button("Button 2");
```

### separators
```cpp
ImGui::Text("Section 1");
ImGui::Separator();  // visual separator line
ImGui::Text("Section 2");
```

## file Structure for VS2022 Project

### required files to add:
from `imgui/`:
- imgui.cpp
- imgui_demo.cpp
- imgui_draw.cpp
- imgui_tables.cpp
- imgui_widgets.cpp

from `imgui/backends/`:
- imgui_impl_win32.cpp
- imgui_impl_dx11.cpp

### include directories:
```
$(SolutionDir)imgui
$(SolutionDir)imgui\backends
```

### linker dependencies:
```
d3d11.lib
d3dcompiler.lib
```

## learning desources

1. ImGui Demo Window - the best resource. it shows the code for everything:
   ```cpp
   ImGui::ShowDemoWindow();
   ```
   study this. it's like a living documentation.

2. official Docs: https://github.com/ocornut/imgui
   - check the `docs/` folder
   - FAQ is super helpful

3. TheCherno's Tutorial: https://www.youtube.com/watch?v=vWXrFetSH8w
   - best video introduction
   - shows real-world usage

## tips

1. always check the demo window - it has examples of everything
2. bse static variables for values that need to persist
3. don't overthink it - ImGui is meant to be simple
4. the main loop pattern is always the same - once you get it, you get it
5. read the comments in imgui_demo.cpp for advanced techniques

## common mistakes that I should avoid

- forgetting to call `ImGui::NewFrame()` before creating UI
- not calling `ImGui::Render()` after creating UI
- forgetting `static` on variables (they'll reset every frame)
- not matching `Begin()` with `End()`
- trying to create UI outside the main loop

## graphics backend options

ImGui supports many backends (in `imgui/backends/`):
- DirectX 9/10/11/12 - Windows, what I'm using
- OpenGL 2/3 - Cross-platform
- Vulkan - Modern, complex
- Metal - macOS/iOS
- SDL/GLFW - window management libraries

i chosen DX11 because:
- native to Windows
- good balance of modern features and simplicity
- widely used in games I'm interested in (also to create game cheats)

## my understanding so far

ImGui doesn't actually draw anything itself. It just:
1. tracks what UI elements you want
2. builds vertex/index buffers
3. tells DirectX/OpenGL to draw them

the backend (imgui_impl_dx11.cpp) handles translating ImGui's draw commands into actual DirectX calls.

that's why we need both:
- `imgui_impl_win32.cpp` - handles Windows input
- `imgui_impl_dx11.cpp` - handles DirectX rendering

## useful code snippets

### FPS counter
```cpp
ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
```

### centered text
```cpp
ImVec2 windowSize = ImGui::GetWindowSize();
ImVec2 textSize = ImGui::CalcTextSize("Text");
ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
ImGui::Text("Text");
```

### disable widget
```cpp
ImGui::BeginDisabled(true);
ImGui::Button("Can't Click");
ImGui::EndDisabled();
```
