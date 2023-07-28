# Visual_Algorithms
Simple video games with minimal footprint.

<h2>Conway's Game of Life</h2>
The algorithmic implementation is rather naive, but most important are sandbox possibilities - user can:<br>
  create new cells on the grid,<br>
  kill cells on the grid,<br>
  clear the grid,<br>
  repopulate the grid with random placement of cells,<br>
  speed up or slow down the simulation tempo.<br>
Controls are specified in the .txt file. As of now, the font file (.ttf) is required for the game to run.

<h2>FPS gen2</h2>
This Wolfenstein3D clone utilizes DDA raycasting algorithm for 3D world simulation, although no enemy to shoot at is present.
Every time the game is booted up, the map is generated randomly with simple, yet fast and scalable labirynth algorithm.
Any map sizes, minimap scale and corridor width are supported, though change requires re-compilation.
Player moves with "WASD", and can also toggle minimap with the "M" key.
OpenGL version is released too, but as of now it looks less interesting than Windows Console edition.

<h2>Lazy Snake</h2>
Classic snake game running on the Windows Console.
The twist is that now snake's body behaves like a piece of rope being dragged around, and is very cool to look at :D.
Porting to OpenGL is planned for the future.
