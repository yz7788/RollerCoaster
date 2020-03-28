#version 150

in vec3 position;
in vec3 normal;
in vec4 color;

out vec3 surfaceNormal; 
out vec3 tolightVector;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;

uniform vec3 lightPosition;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);

  surfaceNormal = normalize((normalMatrix * vec4(normal, 0.0f)).xyz);
  tolightVector = normalize(lightPosition - (modelViewMatrix * vec4(position, 1.0f)).xyz);
  
  col = color;
}