#version 150

in vec3 surfaceNormal;
in vec3 tolightVector;
in vec4 col;

out vec4 c;

uniform vec3 lightColor;

void main()
{
  // compute the final pixel color

  float ambientStrength = 0.3;
  vec3 ambient = ambientStrength * lightColor;

  vec3 unitNormal = normalize(surfaceNormal);
  vec3 unitLightVector = normalize(tolightVector);
  
  float brightness = max(dot(unitNormal, unitLightVector), 0.0);
  vec3 diffuse = brightness * lightColor;

  c = vec4((diffuse + ambient) * col.xyz, 1.0);
}

