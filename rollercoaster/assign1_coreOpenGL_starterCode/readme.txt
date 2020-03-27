The project is build on Visual Studio 2019.


Arguments--------------------------------------------------------------------------------------------------------------------
This project provides support for color input from a new input image. So there are two different types of arguments.

The first one is only for height map input. The result will be a 3d height map in black and white. Here is a sample argument. (The same as starter code)
./hw1 heightmap/spiral.jpg

The second one supports color input. In this type it needs one more argument for the color image. Here is a sample argument.
./hw1 heightmap/spiral.jpg heightmap/lena.jpg 

Note: 
1. The color image must be the same size as the heightmap image (support only .jpg).
2. The color image can be both 3 bytes per pixel or 1 byte per pixel.
3. If the color image is the same as the height map image, please apply the previous type of argument.


Keyboard Control------------------------------------------------------------------------------------------------------------
Press "1": Render as Points
Press "2": Render as Wireframes
Press "3": Render as Solid Triangles
Press "4": Render as Smoothed Triangles 
Press "5": Render as Wireframe over Smoothed Triangles (I set the color of wireframe as all grey because it is much easier to observe)


Viewport control-----------------------------------------------------------------------------------------------------------------------
Left mouse button drag/ Middle mouse button drag: rotate the object
ctrl + left mouse button drag: Moves up and down
ctrl + middle mouse button drag/shift + left mouse button drag: scale the object
shift +middle mouse button drag: move the camera forward and backward


EBO--------------------------------------------------------------------------------------------------------------------------
This project enables both EBO and VBO (without EBO). 
There is a sentence #define EBO in line 35 in the code. 
With EBO defined, the project is using EBO. Otherwise it is just using VBO.


Modes------------------------------------------------------------------------------------------------------------------------
Mode is used for different vertex shader. It is different from keyboard input.
In this project there are 3 modes in total.
The first two modes are the same as defined in Homework Requirement.
The third mode is for smooth version supporting color input. In this case only the height (z value) should be smoothed, but not the color.


Feature Summary----------------------------------------------------------------------------------------------------------
1. EBO
2. Render as Wireframe over solid triangle
3. Support another color image of equal size as color input (for both bytes per pixel = 1 & bytes per pixel = 3)


Animation-----------------------------------------------------------------------------------------------------------------
For the animation, the input sequence is as follows.

set argument as ./hw1 heightmap/spiral.jpg -> press key 2 -> press key 3 -> press key 4 -> press key 5
set argument as ./hw1 heightmap/spiral.jpg ./hw1 heightmap/lena.jpg -> press key 2 -> press key 3 -> press key 4 -> press key 5
left mouse button-> middle mouse button -> ctrl + left mouse button -> ctrl + middle mouse button -> shift + left mouse button ->shift + middle mouse button
