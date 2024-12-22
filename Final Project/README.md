# Project #6 – Shaders

## Description of what I did do get the display I got:

**Final Project Proposal:**

For my final project, I would like to create a 3D drawing software where you build
something out of blocks. My idea was to present the user with a 25x25 (or something else) sized grid. To
place a block, the user can hover over a place on the grid and then click left to put a block. To delete a
block, they can right-click on the block. I was thinking of using shaders to color the blocks and allowing
the user to switch between specific colors. I want to add lighting, but it will just be above the grid
somewhere. Since there will be lighting, I also want to add shadows for all the blocks.


To make everything more efficient, I considered using an instance to display all the
blocks. There would be a VBO for the basic block geometry and another VBO for the positions of all the
blocks. Then, I can combine the base geometry with the instance position. To calculate the block
location, I could use OpenGL Picking to find the side of the block that the user clicks on. If they click on
the side, it will place a block at the same height and adjacent to the block they clicked on. If I cannot
figure out how to determine what side I clicked, then I will make it so the blocks keep going up. The plan
was to make the grid a row of blocks that are all one color and cannot be deleted so that I can share logic
between everything.


_Added Comment: Would it make more sense to use keyboard controls to place blocks instead of picking?
If I use keyboard controls, I can use WASD to move on two axes (like x and z) and Q and E to move on the
third axis. This might give the user more flexibility in block placement at the sacrifice of speed._


**Description:**

I started by following the provided grass instancing sample to get the hang of how instancing
worked. I then used that to create floors for each of the drawing canvases. Then, for what the user draws,
I just made an array with a custom block type I made that held the block position, the block color, and
whether the block was active or not. I then tried to make an essential fragment and vertex shader using
instancing and an array with the saved location. I got something to work but realized I was using
immediate mode-style rendering. This was fine for now, so I moved on to setting up controls so I could
test everything. The floor platforms still were using instancing.

I created controls for moving the selection block north, west, south, and east (W A S D) and two
controls for moving the selection block up and down (E and Q). I then decided to use F as the key to place
a block and G as a key to remove a block. There are also controls for changing the block color, 1-6.

This ended up working as expected, but there was one issue: If you were in the location of the
block you placed, you could not tell since both the selection blocks were the same size and color. So I
had an idea. I enabled transparency and told the vertex shader to enlarge the block if its transparency
was under a certain threshold. I technically made the select block two blocks, one being the currently
selected color and the other a slightly bigger, transparent white one.

Initially, I just wanted to make a canvas for people to draw on, but then I had an idea for a simple
game where you would copy what you see on the right side to the left side. To make levels, I drew (placed)
blocked and first made the hotkey “t” export the current user drawing to the console. I did this several
times and made five levels, as seen below.

I tried to use some interesting shader techniques like shadows, but I needed more time since I had
added so much new functionality. I gave each block a basic form of depth so it was easier to tell them
apart.

I hate how video games are unoptimized. I realized that the array that stored my blocks (drawings)
was looping every single time display(). This loop worked by going through each item in the array and then
checking whether it was enabled. If it was enabled, it would draw a cube there. I planned to get this to
work with instancing, so I would only have to do one draw call, but I could not get it to work correctly. I got
it primarily working, but for some reason, my color logic, including for the selection block, was not
breaking. I then remembered something from our first week that we had been using for most of the
projects: Display Lists. While it’s not as good as instancing (I think), I could process the drawing once and
store it on a display list. Then, I just need to update the display list when something changes, and now my
program only processes the array of blocks when required. I then worked on making the levels swap from
one to the next, and I was done.

**What’s Different?**

I planned on just creating a 3D drawing space in my proposal. Instead, I turned it into a simple
game. I also planned on adding better lighting and shadows, but designing the game and learning how to
do instancing already took a decent amount of time. I still color the cubes with shaders, but it's a simple
fade effect.

I also wanted to use instancing instead of a display list for the user’s drawing, but it never gave me
the result I was looking for.

**What I learned:**
- Instancing
- Creating patterns with shaders
- Transparency


## Video Demo:
https://youtu.be/hsl9r0Xn6h8