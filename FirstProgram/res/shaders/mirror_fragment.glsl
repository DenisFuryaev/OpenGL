#version 330 core
out vec4 FragColor;

in vec4 ClipCoord;
uniform sampler2D mirrorTexture;

void main()
{
	vec2 ndc = (ClipCoord.xy/ClipCoord.w)/2.0 + 0.5;
	vec2 newTexCoords = vec2(1-ndc.x, ndc.y);
	FragColor = texture(mirrorTexture, newTexCoords);
}
