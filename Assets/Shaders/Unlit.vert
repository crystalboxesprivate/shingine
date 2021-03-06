#version 410 

uniform mat4 _ModelMatrix;
uniform mat4 _ModelMatrixInverseTransposed;
uniform mat4 _ViewMatrix;
uniform mat4 _ProjectionMatrix;

in vec3 _PositionAttribute;
in vec3 _NormalAttribute;
in vec3 _TexCoordAttribute;

out vec4 _WorldPosition;

void main()
{
  vec3 position = _PositionAttribute;
  mat4 MVP = _ProjectionMatrix * _ViewMatrix * _ModelMatrix;
  gl_Position = MVP * vec4(position, 1.0);

	_WorldPosition = _ModelMatrix * vec4(position, 1.0);
}
