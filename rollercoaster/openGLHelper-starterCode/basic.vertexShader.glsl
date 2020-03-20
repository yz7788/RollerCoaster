#version 150

in vec3 position;
in vec4 color;
//in vec3 normal;

//out vec3 surfaceNormal; 
//out vec3 tolightVector;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

//uniform vec3 lightPosition;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);

  //surfaceNormal = normal;
  //tolightVector = lightPosition - position;
  col = color;
}