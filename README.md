# ShapeWars
Shape Wars is a fast-paced 2D clicking game built in C++ with SFML, featuring multiple game modes, power-ups, and colorful animated backgrounds. Click good shapes to score points, avoid bad shapes that cost lives, and grab power-ups for special abilities. Choose between Normal, Timed, Endless, and Hardcore modes, each with its own challenge.


---

## Features
- Multiple shape types with unique behaviors:
  - Good Shape – gain points
  - Bad Shape – lose points and lives
  - Exploder Shape – bursts into smaller good shapes
- Power-ups: Freeze, Double Points, Extra Life
- 4 Game Modes:
  - Normal – standard play with limited lives
  - Timed – race against the clock
  - Endless – play forever, but shapes get faster
  - Hardcore – no mercy, fewer lives
- Dynamic backgrounds with smooth color transitions
- Sound effects and responsive UI
- Keyboard & mouse input for navigation and gameplay
- OOP-based architecture (Inheritance, Polymorphism, Encapsulation)
- Dynamic spawn rate adjustment for progressive difficulty

---

## Tech Stack
- Language: C++
- Library: [SFML 2.5+](https://www.sfml-dev.org/) (Graphics, Audio)
- Key Concepts:
  - Object-Oriented Programming  
  - Smart pointers (`std::unique_ptr`)  
  - Random number generation with `<random>`  
  - Game loop and frame-based updates

---

## Installation & Build

### 1. Install SFML
- Download SFML from: [https://www.sfml-dev.org/download.php](https://www.sfml-dev.org/download.php)
- Extract it and note its path for later.

### 2. Build in MSYS2 MINGW Powershell
```cd /c/Users/your name/Documents/folder
```g++ main.cpp -o game.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```./game.exe

