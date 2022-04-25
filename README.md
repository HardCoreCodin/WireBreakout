# WireBreakout

A Breakout clone rendered in wireframe using [SlimEngine++](https://github.com/HardCoreCodin/SlimEngineCpp) <br>

<img src="Breakout.gif"><br><br>

Controls:
* Left Arrow or 'A' : Slide paddle left
* Right Arrow or 'D' : Slide paddle right
* Up Arrow or 'W' : Launch the ball (if close to the paddle)
* SpaceBar : Pause/Unpause the game
* Escape : Quite the game

When the game is paused, a perspective camra allows for f322 3D nabigation of the scene, and has 2 modes: <br>
1. FPS navigation (WASD + mouse look + zooming)<br>
2. DCC application (default)<br>

<img src="BreakoutPause.gif"><br><br>

Double clicking the `left mouse button` anywhere within the window toggles between these 2 modes.<btr>

Entering FPS mode captures the mouse movement for the window and hides the cursor.<br>
Navigation is then as in a typical first-person game (plus lateral movement and zooming):<br>

Move the `mouse` to freely look around (even if the cursor would leave the window border)<br>
Scroll the `mouse wheel` to zoom in and out (changes the field of view of the perspective)<br>
Hold `W` to move forward<br>
Hold `S` to move backward<br>
Hold `A` to move left<br>
Hold `D` to move right<br>
Hold `R` to move up<br>
Hold `F` to move down<br>
