# Project #2 – Transformations (Animate a Helicopter!)

## Description of what I did do get the display I got:

I started off by importing the helicopter and then turning it into its own display list. Then I
created a single display list for one of the blades, and then called it twice in the Display()
function, where I applied the transformations. 

I then had to figure out how to animate the
blades, which I did by modifying one of the example functions, and then I used a global
variable for the angle that I would change for both blades. Both blades use the same angle,
just the rear blade is always 2x times the angle, which makes it spin twice as fast. 

Creating the cockpit and outside views was not too challenging, as the eye system just made sense
to me. I added the menu option by replacing the perspective menu options, as those were
not needed for this project. I also had to move all the scaling and movement stuff into an if
statement so you could not look around while in the cockpit.

## Video Demo:
https://youtu.be/7WL23-xfA1M