#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

in vec4 fs_Col;
in vec2 fs_UV;

out vec4 out_Col;

uniform sampler2D u_Texture;
uniform int u_Mode;

void main()
{
    vec3 col = texture(u_Texture, fs_UV).rgb;

    if (u_Mode == 1) {
        col += vec3(0, 0, col.b * 0.6);
    } else if (u_Mode == 2) {
        col += vec3(col.s * 0.6, 0, 0);
    }

    out_Col = vec4(col, 1);
}
