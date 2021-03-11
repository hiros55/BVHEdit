attribute vec4 Position;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
void main()
{	
	gl_Position = projectionMatrix* modelViewMatrix *Position;
}