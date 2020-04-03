#version 150

in vec3 position;
in vec3 lightnormal;

out vec3 viewPosition;
out vec3 viewNormal;
out vec3 viewLightDirection;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;
uniform vec3 LightDirection;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)

  vec4 viewPosition4 = modelViewMatrix * vec4(position, 1.0f);
  viewPosition = viewPosition4.xyz;

  // final position in the normalized device coordinates space
  gl_Position = projectionMatrix * viewPosition4;

  // view-space normal
  viewNormal = normalize((normalMatrix*vec4(lightnormal, 0.0f)).xyz);

  //view-space LightDirection
  viewLightDirection = (modelViewMatrix * vec4(LightDirection, 0.0f)).xyz;
}