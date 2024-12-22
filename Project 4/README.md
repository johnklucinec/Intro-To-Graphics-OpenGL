# Project #4 – Keytime Animation

## Description of what I did do get the display I got:

I first figured out how to combine my Wheatly OBJ file with a spotlight so he looks similar to how he is in
the game. I just shared the position of Wheatley and the spotlight, so that was not too difficult. The hard
part was ensuring the spotlight always looked in the right direction. While I could have used keyframes for
this, I used math. After a bunch of videos on "Quaternions," I came up with a solution that allowed me to
update where the light is looking at all times based on the rotation of Wheatly. This is calculated in the
Animate() function.


After I did this, I started playing around with the keyframing for positioning Wheatly. After I got something I
was happy with, I added three companion cubes and a Minecraft-style piston, which I made black to
signify that Wheatly couldn't get the color from it. Once I placed the cubes, I animated Wheatly's rotation
so he looked at the cube. I then played around with the camera angles and positioning until I got
something that helped me see the scene better but not too distracting. I also used keyframes to change
the spotlight color.


For the extra credit, I split the piston OBJ file into a piston body and head to change the piston X position
to make it look like it was opening. I did not think this was enough, so I used keyframes to drain and add
color to the blocks when Wheatly flew by.

**Animated Quantities:**

_Wheatly Positioning and Rotation:_ Xpos1, Xrot1, Yrot1, Ypos1, Zpos1

_Camera Look-at and Eye Position:_ Xlook, Ylook, Xeye

_Inner core/Spotlight Color:_ Red4, Green4, Blue4

**Bonus Animated Quantities:**

_Piston Head Position:_ Xpos2

_Companion Cube Colors:_ Red1, Red2, Red3, Green1, Green2, Green3, Blue1, Blue2, Blue3

## Video Demo:
https://youtu.be/D4FoCPK97u0