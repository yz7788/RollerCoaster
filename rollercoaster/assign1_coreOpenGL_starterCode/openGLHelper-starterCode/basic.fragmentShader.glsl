#version 150

in vec3 surfaceNormal;
in vec3 tolightVector;
in vec4 col;

out vec4 c;

uniform vec3 lightColor;

void main()
{
  // compute the final pixel color
  
  vec4 La;
  vec4 Ld;
  vec4 Ls;

  vec4 ka;
  vec4 kd;
  vec4 ks;
  float alpha;
  
  vec3 k_a = vec3(0.3, 0.3, 0.3);
  vec3 ambient = vec3(k_a.x * lightColor.x, k_a.y * lightColor.y, k_a.z * lightColor.z);

  vec3 unitNormal = normalize(surfaceNormal);
  vec3 unitLightVector = normalize(tolightVector);
  
  float brightness = max(dot(unitNormal, unitLightVector), 0.0);
  vec3 diffuse = brightness * lightColor;

  c = vec4((diffuse + ambient) * col.xyz, 1.0);
}