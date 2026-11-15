# MediView - Whole-Body Anatomy Explorer

MediView is an interactive 2D anatomy visualizer for introductory learners. It presents selected major structures in eight body systems through a shared front-view body model. The application is written in C++11 and uses the built-in Windows API for the desktop window and input. All anatomy, fills, outlines, shading, labels and hit detection are computed in C++.

## Run on Windows

1. Extract the entire ZIP to a normal folder.
2. Open `MediView_Win32/build/MediView.exe` on 64-bit Windows.
3. The respiratory system opens initially. Choose a system in the sidebar.

No third-party graphics libraries or internet connection are required. The prebuilt executable has statically linked compiler runtime support and imports only Windows system DLLs. It was cross-compiled in a Linux environment. Complete the Windows acceptance checklist in `docs/VERIFICATION.md` before submission.

## Build from source

### MinGW-w64

Install a Windows C++ compiler and ensure `g++` is on PATH. Run `build.bat` to compile and run the two test suites and create the application. Run `run.bat` to rebuild, test and launch. A failed build prevents launching an old executable.

Manual command from this folder:

```bat
if not exist build mkdir build
g++ -std=c++11 -O2 -Wall -Wextra -pedantic -Iinclude src/win32_main.cpp src/explorer.cpp src/learning.cpp src/anatomy.cpp src/graphics.cpp src/font.cpp -mwindows -static -lgdi32 -luser32 -o build/MediView.exe
build\MediView.exe
```

### Microsoft Visual C++

Open the x64 Native Tools Command Prompt for Visual Studio, change to this folder and run `build_msvc.bat`. This requires the desktop C++ tools and Windows SDK. The MSVC route is supplied but has not been executed in the preparation environment.

### Portable core tests

The Windows interface requires Windows, but the rendering and model tests run on Linux/macOS with a C++11 compiler:

```sh
mkdir -p build evidence
g++ -std=c++11 -O2 -Wall -Wextra -pedantic -Iinclude tests/test_graphics.cpp src/graphics.cpp -o build/test_graphics
./build/test_graphics
g++ -std=c++11 -O2 -Wall -Wextra -pedantic -Iinclude tests/test_explorer.cpp src/explorer.cpp src/learning.cpp src/anatomy.cpp src/graphics.cpp src/font.cpp -o build/test_explorer
./build/test_explorer evidence
```

The last command regenerates seventeen BMP evidence images. Existing PNG and PDF snapshots do not update automatically.

## Implemented scope

- Full-body front-view silhouette shared by all systems.
- Eight selectable systems: skeletal, muscular, digestive, respiratory, circulatory, nervous, urinary and reproductive.
- Male and female reproductive model variants.
- Clickable structures with a gold selection outline, name, location and function.
- Structure list as an alternative to clicking small shapes.
- Label toggle, mouse-wheel zoom, drag-to-pan and reset view.
- Keyboard system selection and structure cycling.
- Time-based breathing animation with play/pause/reset in the respiratory system.
- Viewport clipping and hit testing against the actual transformed, animated geometry.
- Ten-question identification quizzes with shuffled choices, feedback, score and replay.
- Two-system comparison: primary anatomy filled, secondary anatomy outlined; cycle or swap systems.
- Swallowing animation with a moving food bolus, stage captions and pause/reset.
- Paginated information for every structure and system, including anatomy and connections.
- Complete-frame Win32 double buffering to remove the previous black-clear flicker path.

## System coverage

| System | Selectable entries |
| --- | --- |
| Skeletal | Skull, vertebral column, rib cage, pelvis, arm bones, leg bones |
| Muscular | Deltoids, pectorals, biceps, abdominal muscles, quadriceps, tibialis anterior |
| Digestive | Mouth, oesophagus, liver, stomach, pancreas, small intestine, large intestine |
| Respiratory | Right lung, left lung, trachea, bronchi, diaphragm |
| Circulatory | Major arteries, major veins, heart |
| Nervous | Peripheral nerves, spinal cord, brain |
| Urinary | Right kidney, left kidney, ureters, bladder, urethra |
| Reproductive - male | Testes, ductus deferens, seminal vesicles, prostate, penis |
| Reproductive - female | Ovaries, uterine tubes, uterus, cervix and vagina |

Grouped entries represent selected structures, not a complete anatomical inventory. Eight is the project's selected scope, not the number of organ systems in the human body.

## Controls

| Action | Mouse | Keyboard |
| --- | --- | --- |
| Change system | Sidebar | 1-8 |
| Select structure | Click body or list | Tab / Shift+Tab |
| Clear selection | Click empty viewport | Esc |
| Zoom | Wheel over body or +/- | + / - |
| Pan | Drag in viewport | — |
| Reset view | RESET VIEW | R |
| Labels | LABELS | L |
| Play/pause current animation | BREATHE / SWALLOW / PAUSE | Space |
| Reset current animation | RESET | B |
| Reproductive variant | VIEW FEMALE / VIEW MALE | F |
| Start / exit quiz | QUIZ / EXIT QUIZ | Q |
| Answer quiz | Click one of four answers | 1-4 |
| Next quiz question / results | CONTINUE / RESULTS | Enter |
| Compare / next secondary system | COMPARE / NEXT LAYER | C |
| Swap primary and secondary | SWAP SYSTEMS | X |
| End comparison | END COMPARE | — |
| Read more information | PREV / NEXT | PageUp / PageDown |
| System overview | SYSTEM INFO | Esc clears selection |

Breathing controls appear and operate only when Respiratory is the primary system. Digestive shows its own swallowing controls. Other systems have no animation buttons. Quiz mode hides all animation controls. Reset breathing pauses at the initial pose. Reset view changes zoom/pan only. Switching systems clears selection, resets the view and stops breathing. Changing reproductive variant clears selection.

## Architecture and algorithms

| File | Purpose |
| --- | --- |
| `include/graphics.h` | Canvas, colours, points, polygons and primitive interfaces |
| `src/graphics.cpp` | BMP output, Bresenham lines, Cohen-Sutherland, scanline fill, Sutherland-Hodgman, transforms, midpoint circle/ellipse, Bézier and Catmull-Rom |
| `src/font.cpp` | Bitmap font drawn into the custom canvas |
| `include/explorer.h` | Platform-independent application state and anatomy interfaces |
| `src/anatomy.cpp` | System/structure metadata, model geometry and body outline |
| `src/learning.cpp` | Extended information, pagination, quiz rounds, comparison state and swallowing path |
| `src/explorer.cpp` | Rendering, shading, animation, inverse transforms, picking, controls and labels |
| `src/win32_main.cpp` | Window/message loop, mouse/keyboard events, timer and double-buffered presentation |
| `tests/test_graphics.cpp` | Original algorithm checks |
| `tests/test_explorer.cpp` | Model, picking, animation, control and clipping checks plus evidence export |

Ordered polygon vertices represent body and organ shapes. Cubic Bézier sampling forms smooth organ contours and vessel routes. Scanline masks support radial shading. An even-odd test selects the topmost visible structure after converting the pointer to model coordinates. Breathing uses a cosine cycle with elapsed time. Win32 copies the complete frame to the visible window through one `BitBlt`.

Midpoint circles/ellipses and Catmull-Rom remain available and tested in the preserved CP1 demonstrations. The full-body renderer also uses sampled ellipse polygons where a selectable filled outline is needed. DDA and general alpha compositing are not implemented.

## Documents and evidence

- `docs/TECHNICAL_REPORT.md`: design, algorithms, flicker fix, scope and limitations.
- `docs/VERIFICATION.md`: checks actually performed and Windows acceptance steps.
- `docs/DEMO_GUIDE.md`: demonstration sequence and explanation prompts.
- `docs/SUBMISSION_CHECKLIST.md`: submission and repository checklist.
- `docs/REFERENCES.md`: educational anatomy and Windows API references.
- `docs/MediView_Final_Presentation.pptx`: editable final presentation.
- `evidence/MediView_Graphical_Evidence.pdf`: seventeen rendered output pages.
- `evidence/*.png`: actual output of the C++ renderer.

## Limitations

The geometry is schematic and not anatomically validated by an expert. Organ sizes, vessel routes, peripheral nerves and reproductive structures are simplified for an introductory 2D graphics demonstration. Internal structures are projected onto a front view; depth is not represented. The neutral silhouette is shared between reproductive variants. Breathing is illustrative rather than a physiological simulation. Fixed curve sampling can reveal edges under high zoom. Win32 is Windows-specific. The bitmap font has limited character coverage.

Quiz mode, swallowing and two-system comparison are now implemented. Back views and animations for other systems are not implemented. Comparison is a shared-coordinate outline overlay, not a split-screen view. Swallowing is a schematic eight-second sequence, not a physiological simulation; airway closure and coordinated muscle actions are not modelled. Quiz scores last for the current round only. Native Windows acceptance testing and any lecturer-specific requirements must be completed before submission. There is no supplied final marking rubric, so the implementation follows the original proposal's core feature list.


## Graphics update 
The anatomy viewport now uses 2x supersampling on each axis (four samples per displayed pixel), rounded contours, stronger model-anchored shading and additional organ detail. Lungs and heart have revised outlines. Muscle fibres, brain folds, airway branches, lung lobe lines, digestive folds and kidney highlights are illustrative surface details. Existing controls and native Win32 double buffering are retained. Zoom in and press L to hide labels when examining detail.

Use the newly included `build/MediView.exe`; rebuilding is optional. Extract this update to a separate folder so you can retain the version you have already tested. The updated executable was cross-compiled and the renderer tested, but this revision still needs a Windows smoke test.

## Learning update 

Select **QUIZ** for ten identification questions spanning all eight systems. The target is outlined in gold. Labels and organ notes are hidden until an answer is submitted. Choose 1–4, read the feedback and explanation, then press Enter. Replay creates a new shuffled round. Q or Esc exits. The reproductive variant chosen before the round is retained.

Select **COMPARE**, choose the primary system using the sidebar, and use **NEXT LAYER** to choose the secondary system. The primary is filled; the secondary is outlined in amber. **SWAP SYSTEMS** exchanges them so you can select and read either system's structures. Only the primary layer is selectable and animated. F changes the reproductive variant if either layer is reproductive.

Select **Digestive**, then **SWALLOW**. A gold bolus moves from the mouth through the oesophagus to the stomach, with an illustrative contraction marker. The sequence stops after eight seconds; press SWALLOW again to replay. Pause holds its position and RESET returns to the beginning.

Select a structure and use **NEXT/PREV** to read its location, role, structure and connections. **SYSTEM INFO** opens the system overview. All 44 distinct structure entries across both reproductive variants have expanded notes. Added details include vertebral divisions, pancreatic lobules, uterine contours and ovarian/testicular texture.
