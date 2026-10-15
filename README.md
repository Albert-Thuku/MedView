# MediView — CP1 Technical Foundation

MediView is a proposed 2D anatomy visualizer. This checkpoint uses Pure C++ to render a detailed, schematic respiratory system and demonstrate the graphics algorithms covered in class.

The current program produces four image files. The sidebar and information panel in the respiratory image are static graphics. Clicking organs, switching systems and breathing animation are planned for later checkpoints.

## Group members and assigned parts

| Member | File | Responsibility |
| --- | --- | --- |
| Nicole | `src/canvas.cpp` | Pixel canvas, pixel access and BMP image export |
| Trina | `src/anatomy.cpp` | Respiratory anatomy, shading, airway details and smooth edges |
| Ivy | `src/polygons.cpp` | Polygon outlines, filling, clipping and transformations |
| Brit | `src/curves.cpp` | Cubic Bézier curves and Catmull–Rom splines |
| Kian | `src/primitives.cpp` | Line drawing, line clipping, circles and ellipses |
| AJ | `src/application.cpp` | Main program, demonstration scenes and bitmap text |


## Technologies and tools

- Standard C++11 and the C++ standard library.
- A C++ compiler supporting C++11, such as GCC/MinGW with the `g++` command.
- Git and a shared GitHub repository.
- An image viewer to display the generated BMP files.
- A document editor for assembling screenshots into a PDF, if needed.

The renderer calculates and colours its own pixels. It does not use OpenGL, SDL, SFML, Qt or an external graphics library. `graphics.h` is our own shared header, not a third-party graphics library.

## 1.   We Put everyone's files in one repository

We used one shared GitHub repository for the group. 

```text
MediView/
├── README.md
├── include/
│   └── graphics.h
└── src/
    ├── nicole/
    │   └── canvas.cpp
    ├── trina/
    │   └── anatomy.cpp
    ├── ivy/
    │   └── polygons.cpp
    ├── brit/
    │   └── curves.cpp
    ├── kian/
    │   └── primitives.cpp
    └── aj/
        └── application.cpp
```



The six parts are modules of one program. They are not intended to run separately. AJ's file contains the single `main()` function that starts the application.

## 2. Get the project onto a computer


If the project is already cloned locally, save or commit your own changes, then update your main branch:

```powershell
git switch main
git pull
```

Open PowerShell in the project root: the folder containing `README.md`, `include` and `src`. Do not run the compile command from inside one member's folder. If downloading a ZIP, extract it first.

## 3. Compile and run from the code

This method works with the seven source/header files shared in the chat. It does not require the extra build scripts, gallery or tests from the ZIP.

First check whether the compiler is available:

```powershell
g++ --version
```

If the command is not found, install a C++ compiler such as MinGW-w64, add its `bin` directory to PATH and open a new terminal. Installing a code editor alone does not install a compiler.

Create the output folders:

```powershell
New-Item -ItemType Directory -Force build, evidence | Out-Null
```

Compile all six source files together. Copy the entire following command as one line:

```powershell
g++ -std=c++11 -O2 -Wall -Wextra -pedantic -Iinclude src/nicole/canvas.cpp src/trina/anatomy.cpp src/ivy/polygons.cpp src/brit/curves.cpp src/kian/primitives.cpp src/aj/application.cpp -o build/mediview.exe
```

If compilation reports an error, resolve it before running the executable. If it succeeds, run:

```powershell
.\build\mediview.exe evidence
```

Allow the detailed respiratory image to finish rendering. The program generates four BMP files and then exits. It does not open an interactive anatomy window.

Open the respiratory output:

```powershell
Start-Process .\evidence\01_respiratory.bmp
```

The other three images are in the same `evidence` folder.

### If using the complete ZIP instead

The complete ZIP also includes `build.bat`, `run_demo.bat`, tests and an HTML gallery. From the project root, run:

```powershell
.\build.bat
```

Then open the gallery:

```powershell
Start-Process .\evidence\index.html
```

You can also double-click `run_demo.bat`. It can reuse an existing executable, so run `build.bat` first after changing any source code.

The tests and gallery are not included in the seven copy-paste code blocks. Their absence does not prevent the manual compile-and-run method above.

## Graphics techniques implemented

| Technique | Algorithm | Where it appears |
| --- | --- | --- |
| Lines | Bresenham | Airways, leaders and polygon outlines |
| Line operations | Cohen–Sutherland clipping | Before/after line-clipping example |
| Polygon representation | Ordered vertex lists | Torso and sampled organ contours |
| Polygon filling | Even-odd scanline filling | Body and lung interiors |
| Polygon clipping | Sutherland–Hodgman | Enlarged lung clipped to a rectangular window |
| Polygon transformations | Scaling about an origin and translation | Original, translated and scaled lungs |
| Circles | Midpoint circle algorithm | Algorithm atlas and markers |
| Ellipses | Midpoint ellipse algorithm | Tracheal rings and algorithm atlas |
| Curves | Cubic Bézier evaluation | Organ contours and diaphragm |
| Splines | Catmull–Rom interpolation | Curved-path study in the atlas |

The detailed respiratory view also uses colour interpolation, masks and 3× supersampling implemented in C++.

## 4. Prepare the graphical evidence

There are four graphical outputs

| Output file | What to show | Suggested caption |
| --- | --- | --- |
| `01_respiratory.bmp` | The complete detailed respiratory view | Respiratory model rendered in Pure C++ using filled Bézier contours, branching airways, ellipse rings, shading and supersampling. |
| `02_algorithms.bmp` | All four sections of the algorithm atlas | Bresenham lines, midpoint circle and ellipse, cubic Bézier controls and an interpolated Catmull–Rom spline. |
| `03_clipping.bmp` | Both the before and after views, including the gold windows | Sutherland–Hodgman polygon clipping and Cohen–Sutherland line clipping against a rectangular viewport. |
| `04_transforms.bmp` | The original, translated and scaled versions | The same lung geometry reused with translation and scaling about its local origin. |

