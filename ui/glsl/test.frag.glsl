/*
* Fragment shader
*/
uniform sampler2D tex;

void main (void)  
{     
    //gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); 
	vec4 color = texture2D(tex, gl_TexCoord[0]);
	gl_FragColor = color;
}
