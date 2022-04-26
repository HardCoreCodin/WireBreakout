# WireBreakout

A Breakout clone rendered in wireframe using [SlimEngine++](https://github.com/HardCoreCodin/SlimEngineCpp) <br>
GameLib holds the game itself, decoupled from the engine or platform.<br>
It just exposes an API so can be hosted inside any engine <br>
It is all written from scratch in C++ with no dependencies so the executable is only 53Kb.<br>
It is essentially 2D as there is no notion of depth, so could be rendered in a 2D engine. <br>
It is rendered here in software in 3D using an orthographic projection while playing the game. <br>
Though the level can be navigated in 3D with a perspective camera when the game is paused.<br>
There is no editor so level maps are specified in ASCII form as a string. <br>
It has robust collision response using a few approaches including a Minkowski Sum of Rectangle/Circle.<br>

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
