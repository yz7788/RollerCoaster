#version 150

//in vec3 surfaceNormal;
//in vec3 toLightVector;

in vec4 col;
out vec4 c;

//uniform vec3 lightColor;

void main()
{
  // compute the final pixel color

  //vec3 unitNormal = normalize(surfaceNormal);
  //vec3 unitLightVector = normalize(toLightVector);
  
  //float brightness = max(dot(unitNormal, unitLightVector), 0.0);
  //vec3 Diffuse = brightness * lightColor;

  //color = vec4(diffuse, 1.0);
  c = col;
}

