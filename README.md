# Mini-Minecraft
**Team JAH - Allison, Anna, and Jonas**

CIS 460/560, Professor Mally, University of Pennsylvania, School of Engineering and Applied Sciences

## Milestone 1
### Procedural Terrain (Jonas)

My approach was to write and refine my code in a Python notebook and then rewrite the logic in C++. A notebook of my Python work can be found [here](https://gist.github.com/oppenheimj/57008d578af5a41cba40492befdc186e).

The grasslands terrain is generated using two-dimensional fractal Brownian Motion (fBM) driven by a Perlin noise function. The mountain terrain is generated using Worley noise. Smooth transitions between these two biomes is achieved using another fBM function with `glm::smoothstep` applied in order to hasten the transitions from one biome to another. The end result is that 50% of the terrain is pure grassland, 40% is mountain, and the remaining 10% is transition.
 
The `ProcGen` class contains all of the logic related to procedural terrain generation. All of its methods are static because no state needs to be stored. The only public method is
```
static int getHeight(int x, int z);
```
which returns a height given `x` and `z` coordinates.


### Efficient Terrain Rendering and Chunking (Allison)
CHUNK:
- created a struct storing the position of a vertex and another struct holding the direction, offset, and vertices (and their positions) that make the face of a block neighbor
- made Chunk inherit from Drawable
- implemented create() function that iterates over the x, y, z coord of chunks and iterates over the 6 adjacent block coords (from the struct) to check if they are empty. If that neighbor block is empty, VBO data is appended for each quadrangle of each empty neighbor block
- implemented loadVBO() function that takes in a vector of interleaved vertex data and a vector of index data, and buffers them into the appropriate VBOs of the Drawable
- implemented setWorldPos() function that sets the x and z world position

TERRAIN:
- changed draw() function when Chunk inherits from Drawable, so it draws each Chunk with the given ShaderProgram; set the model matrix to the proper X and Z translation
- implemented updateScene() function that updates the terrain scene given the player's position. As the player approaches 16 blocks from an edge of a Chunk that does not connect to an existing Chunk, the terrain expands

SHADERPROGRAM: 
- implemented a second drawing function drawInterleaved() for the ShaderProgram class that renders a Drawable that has been set up with interleaved VBOs
  
DIFFICULTIES: 
- had trouble displaying the correct colors properly. Fixed this by changing the given glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 1 * sizeof(glm::vec4), (void*)sizeof(glm::vec4)) in drawInterleaved() by multiplying sizeof(glm::vec4) by 3 instead of 1
- difficulties with every corner chunk missing when drawing. Fixed this by moving where I assigned color in Chunk create()
- was very confused with how terrain expansion works/what it was supposed to look like. Kept getting conflicting info so I made it expand in a way that I thought looked most intuituve. Ended up confirming with 2 TAs during OH that my terrain is correctly expanding
- Chunk create didn't recognize that adjacent chunks of different blocks are empty so all the blocks in each chunk will be empty but not the whole terrain. Fixed this by checking the edges of a chunk in xyz so that if the edge blocks of a adjacent chunk were also empty, those in between faces would not be drawn. To do this more specific edge case checking, I no longer was able to just iterate through my array of 6 BlockNeighbors to see if a block existed at the offset value; I had to instead check each of the 6 edge cases and update each VBO accordingly. This resulted in less compact code, but it works :) 

### Game Engine Tick Function and Player Physics (Anna)
TICK: I invoked Player::tick from MyGL::tick and calculated dT using QDateTime::currentMSecsSinceEpoch() in order to make the movements smoother.

INPUTS: I edited the key and mouse events in myGL to update m_inputs when certain keys are pressed, rotate the camera on mouse movement, and to add/remove blocks on mouse clicks. I ran into an issue where the mouseMoveEvent would work find on my Mac, but it would “drift” and act weird on my partner’s Windows. We were able to resolve this issue and my partner generated some code that would work on his laptop after I explained how I calculated dx and dy using the dimensions of the screen and the position of the mouse and then reset the mouse to the center so the camera would stop moving if your mouse stopped moving.

PROCESS INPUTS/COMPUTE PHYSICS: I used the InputBundles struct to affect how the player moves in processInputs() and I updated the player’s velocity and acceleration (which I chose to be a constant 20.f) based on the inputs. I simply matched the key that was pressed to the direction I wanted to associate the key with. I differentiated between when the player is and isn’t in Flight Mode, so different keys are valid in different modes (such as space/jump only works when the player isn’t in Flight Mode). Using the inputs, I edited computePhysics() to update the player’s position based on acceleration and velocity. I first reduced the velocity by multiplying it by 0.95 in order to simulate friction and drag. I also chose to use -9.8 for gravity when jumping/falling down after turning off Flight Mode so it had a natural effect (however, this also means that you fall quite slowly from greater heights, so I’m considering making the constant larger in the future because I’m a little impatient). I had some difficultly working on the jumping function at first, but I was able to implement a function that checked if the player was on the ground (isOnGround()) and then add gravity to bring the player back down if it wasn’t on the ground and not in Flight Mode. 

COLLISIONS: In computePhysics(), I also worked with collisions by using ray casting as it was the simplest approach assuming that the cubes that make up the player are the same size as the cubes in the terrain. In order to stop the player from moving in a collision, I assigned a small epsilon value to the colliding block to act as a barrier and make the player stick to the block.

ADDING/REMOVING BLOCKS: I also used ray casting to see if any blocks intersected within 3 units of the player. If there were any blocks overlapping with the center of the screen, I would either set the block to EMPTY or STONE depending on the mouse click. However, I had some issues with this part as at first my remove function didn’t work after merging my code and both functions were adding/removing blocks in funky places (ex: I would remove the block under me and add a block under the terrain). I was able to get my removeBlock() to work on our combined code after using create() and destroy() and fixing my origin to the camera position, but my placeBlock() is still sometimes a little bit spotty (though it seems to work better on my partner’s computer, my computer is really laggy ;-;).

## Milestone 2

### L-System Rivers (Anna)
RIVER: I created the turtle and river class and used L-system grammar as well as random numbers and sin/cos functions to make the river curve and branch randomly to create a more natural effect. I had difficultly generating a natural looking river and I’m probably going to experiment with it more before the final deadline. I also had a weird bug when we merged the code that created holes in the mountain where the river started/ended, but we were able to fix that. I went back to work on the overlay in a later milestone. I checked if the player was under water or under lava and I would switch the type of overlay used to tint the screen using a quad.

RIVER PLAYER PHYSICS: A player can fall into the river when exiting flight mode. When under water or lava, the player moves at 2/3 speed. If you hold the spacebar, the player moves up at a constant rate, but because you can’t fly out of the river when out of flight mode I allowed the player to poke its head out of the water and it’ll bob up and down a little to simulate swimming. I had some trouble figuring out what swimming should look like but I confirmed with a TA and asked on piazza if it was okay that you don’t exit the water when swimming in the middle of the river.

### Texturing and Texture Animation (Allison)
LOADING IMAGES AS TEXTURES (TEXTURE) : set up a texture class from the base code given in hw4

SPLITTING INTERLEAVED VBO DATA (DRAWABLE, SHADERPROGRAM) : split up interleaved vbos into one vbo for opaque blocks and one for transparent blocks. Each interleaved vbo now holds pos, nor, and uv. uv coordinates are stored as a vec4, where the z cooordinate will store the animateable flag (1 if animated, 0 otherwise). I also split up the idx data into separate opaque idx and transparent idx vbos. 

ANIMATEABLE (LAMBERT) : changed lambert shader files so they could support reading a texture and time. The time variable is incremented every call to paintGL() and is used to create the uv coord offsets for the water and lava to give the illusion of animation. 

DRAWING TRANSPARENT BLOCKS (CHUNK, TERRAIN) : altered chunk create() so that it will update either the opaque or transparent vbos depending on transparency. So, in updateVBO(), current pos, nor, uv, data will be correctly pushed into the correct vbos dependeny on opacity. Interleaved vbo data is loaded in the order- opaque, idx opaque, transparent, idx transparent. In terrain draw(), all the opaque cunks are drawn then all the transparent chunks are drawn.

DIFFICULTIES : 
- had an issue with the terrain rendering black. This was fixed once I realized I had the wrong file path for the textures
- had a lot of issues with the terrain rendering very strangely with weird colors and shapes. At first this was due to the wrong argument values in shaderprogram draw(). Once I fixed this, it still wasn't working and I realized it was because I wasn't correctly pushing in uv data in my interleaved vbos in the updateVBO function in chunk.
- also had issues with only opaque blocks showing up or only transparent blocks rendering but not both simutaneoulsy. This was fixed as I realized I was pushing back idx data for only transparent/only opaquue blocks.

### Multithreading (Jonas)
Multithreading was a significant undertaking. It involved turning the chunk creation process into a sort of conveyer belt, where several vectors of chunks stored the chunks at different stages of creation. Every tick, the contents of each vector are checked for new chunks and threads are created to move the chunks along to the next stage.

The greatest challenge was that any time we had to use the `Terrain::hasChunkAt()` function, we would have to check in multiple places and use mutexes.

## Milestone 3

### Inventory & Sound Effects & Procedurally Placed Assets (Anna)
INVENTORY: For the inventory, I created a new gui and added pictures of each block along with labels. The gui opens and closes when you press I, and I confirmed with a TA that my gui window/format was okay. You can change the block type that you're using by selecting a block in the gui, which also displays the current block type. Furthermore, I had the gui display the quantity of each block. I edited removeBlock and createBlock to keep track of what block was removed/placed in order to update the quantities and I made it so that if you can only place blocks that you have in your inventory.

SOUND EFFECTS: For sound effects, I looked up how to use the QSound class and I edited the .pro file so the sounds would play. When you're in flying, there will be a wind sound. And if you're out of flight mode, you can either hear underwater sounds while swimming in the river or footsteps while walking in the grass. I wanted to add more sounds such as a funny sound effect whenever the player jumped or random bird calls, but unfortunately my laptop was too weak and it made the game lag so much (esp after merging with the sky) that it was affecting other functions :( Currently, sound is extremely choppy with the sky because "sheeeesh we are sampling an expensive noise function to render the sky!" (Allison Chen, 4/28/21) So, you can one or the other: the sky, or your sanity. Basically, comment out the chunk of  “sky code” in order to have the sounds play or comment out the sounds under keyPressEvent to save your eardrums.

PROCEDURALLY PLACED ASSETS: I made a helper function that will draw trees on the grasslands by our ✨fertile✨ river bed. I also used random numbers to scatter the trees around the terrain. I call the function drawTrees after drawing the chunk and the river in order to make sure that no trees are drawn in the water.

NOTE: A lot of my features + just moving around in general ran really slowly on my poor laptop after merging, so I commented out the sky while making my recording.

### day / night cycle + fog + procedural grass (Allison)
DAY / NIGHT CYCLE (SKY, LAMBERT, MYGL) : I made new sky shader files and slightly altered the ray casted day and night sky code from class. The output color at any point in the sky is calculated and this sky illusion is made by putting spherical uvs on the quad (as if we are drawing a large sphere surrounding the scene). The given raytracing code is used to calculate the color values based on where the rays point from the player. The sun will radiate a bright glow while the parts of the sky further away are more dusk colored-this is done by interpolating and mixing values closer to sun/dusk thresholds. The given worley noise and fbm functions are used to create the clouds (noise). SHEEESH I’m sampling an expensive noise function to render the sky which is why its super laggy (asked on piazza and this is expected behavior!!) The sun rotation is done by rotating the sun direction vector across the x axis (rotateX) and it depends on the time. The lambert shader is modified so the lighting shines / dims depending on time so it matches if the sky has sun or is at dusk.

FOG (LAMBERT) :  I had to add a unifPlayer handle in Shaderprogram as well as a function to set the player’s position in order for the fog to move with the player.  I had to receive the player’s position (u_Player) in the shader to compute the distance between the player and the further terrain (basically the distance the terrain is from the camera) so that when the player moves the fog will clear up. The fog color is altered according to time and is rotated so that it follows / kinda blends in the the current sky.

PROCEDURAL GRASS  (LAMBERT) :  I used the given fbm function so depending on the x and z position, darker greens as well as more brownish greens will be dynamically mixed into the grass. 

DIFFICULTIES : had trouble syncing up the night and day lighting as well as the fog coloring. Figuring out how to make the fog look like fog was a bit tricky at first as well and I had to look back at the smoothstep and other math functions used in the shaders in hw4.

NOTE: its super laggy with the sky but this is expected behavior I promise🙄☝️

### Additional Biome (Jonas)
We already had grassy hills and mountains. The third biome was desert. This additional biome occupied the region between 120 and 128 and was generated using perlin fBM with both `x` and `z` scaled by `1/256`.

The tricky part was overlaying the third biome onto of the other two. The approach was to use two separate perlin fBM functions, the first for mixing grass and mountains, and the second for layering the desert on top of the other two. The end result is that we have three separate biomes that smoothly transition into one another. Noise function stuff is definitely my greatest takeaway from this class. It makes me look at nature differently. Nature is order, with a little randomness sprinkled on top.
