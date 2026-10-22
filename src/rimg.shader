#shader vertex
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;

out vec2 v_TexCoord;

void main()
{
    gl_Position = vec4(position, 1.0);
    v_TexCoord = tex_coord;
}

#shader fragment
#version 330 core

out vec4 frag_color;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;

void main()
{
    frag_color = texture(u_Texture, v_TexCoord);
}
