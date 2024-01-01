#version 330
precision mediump float;

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
in vec4 vertexTangent;
in vec2 vertexTexCoord2;

uniform mat4 matNormal;
uniform mat4 matView;
uniform mat4 matProjection;

out vec3 vWorldPosition;

void main()
{
	vec4 worldPosition = matNormal * vec4(vertexPosition, 1.0);
	vWorldPosition = worldPosition.xyz;
	gl_Position = matProjection * matView * vec4(vertexPosition, 1.0);
}
