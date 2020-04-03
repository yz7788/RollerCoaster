Subject 	: CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author		: Yangzhen Zhang
USC ID 		: 7052227306

Description: In this assignment, we use Catmull-Rom splines along with OpenGL core profile shader-based texture mapping and Phong shading to create a roller coaster simulation.

Core Credit Features: 
======================

1. Uses OpenGL core profile, version 3.2 or higher - Y

2. Completed all Levels:
  Level 1 : - Y
  level 2 : - Y
  Level 3 : - Y
  Level 4 : - Y
  Level 5 : - Y

3. Rendered the camera at a reasonable speed in a continuous path/orientation - Y

4. Run at interactive frame rate (>15fps at 1280 x 720) - Y

5. Understandably written, well commented code - Y

6. Attached an Animation folder containing not more than 1000 screenshots - Y

7. Attached this ReadMe File - Y

Extra Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Render a T-shaped rail cross section - Y

2. Render a Double Rail - Y

3. Made the track circular and closed it with C1 continuity - N

4. Any Additional Scene Elements? (list them here) - Y (Island/mountain)

5. Render a sky-box -Y (Dome)

6. Create tracks that mimic real world roller coaster - Y
my_1.sp: Millennium Force, a steel roller coaster located at Cedar Point amusement park in Sandusky, Ohio. 
my_2.sp: no real world reference
reference: 
https://rcdb.com/594.htm
http://plusplus.free.fr/rollercoaster/index.html

7. Generate track from several sequences of splines - Y

8. Draw splines using recursive subdivision - Y 
Code line 702, in hw2.cpp

9. Render environment in a better manner - Y

10. Improved coaster normals - N

11. Modify velocity with which the camera moves - Y

12. Derive the steps that lead to the physically realistic equation of updating u - Y

Additional Features: (Please document any additional features you may have implemented other than the ones described above)
1. Scene changing (key '1' / key '2')
2. Camera View Rotate (left mouse button drag)

Open-Ended Problems: (Please document approaches to any open-ended problems that you have tackled)-----------------------------------------------------------------------------------------------------------------
1. I rendered ground, sky and mountain for scene. Because there are three different texture images, there will be 3 VAOs and 3 glDrawElements for one scene, which is very inefficient.
Solution: 
I combine 3 texture images into a single image. The s in texture coordinates for each texture image is from (0, 1/3), (1/3, 2/3), (2/3, 1). In this way, all the texture information can be bound together to one VAO and draw one time only.

2. Generate a Sphere or Dome in three dimensional space (for sky use)
   Solution: 
   Use spherical coordinate system
   x = r * sinθ * cosφ
   y = r * sinθ * sinφ
   z = r * cosθ
  Traverse θ from 0 to  π. Traverse φ from - π to  π

3. Rotate Camera View while traveling on the rollercoaster
   Solution:
   First consider rotating camera view at origin in the world frame.
   The camera (eye) position is (0, 0, 0);
   The camera focus point is (0, 0, 0) + (cos(pitch) * cos(yaw), sin(pitch), cos(pitch) * sin(yaw));

   Then for camera traveling on the rollercoaster, we consider it in a new frame.
   The new frame has unit spline tangent, unit spline normal, & unit spline binormal as 3 basis. And the spline point as the origin of the frame.
   So we only need to change the frame for the rollercoaster camera rotation.

Keyboard/Mouse controls: (Please document Keyboard/Mouse controls if any)----------------------------------------------------------------------------------------------------------------------------------------------------
1. Key "W": accelarate camera moving by adding a constant speed
   Key "S": decelarate camera moving by minus a constant speed

2. Key " " (spacebar): pause/continue. The camera stops/continues moving automatically.

3. Left Mouse Button Click & Drag: rotate the camera

4. Key "1": change scene to ocean
    Key "2": change scene to grass

Names of the .cpp files you made changes to:---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
1. HW2.cpp
2. basicPipelineProgram.cpp

Comments : (If any)---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
In this program I set the gravitational accelaration as 0.75 instead of 9.8 to make the camera not moving too fast 
