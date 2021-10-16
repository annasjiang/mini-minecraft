#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform sampler2D u_Texture;
uniform int u_Time;
uniform vec4 u_Player;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_UV;
in vec4 fs_CameraPos;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

float mySmoothStep(float a, float b, float t) {
    t = smoothstep(0, 1, t);
    return mix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}

float fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = float(0.0);
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

// sun rotation around x axis given point and angle
vec4 rotateX(vec4 p, float a) {
    return vec4(p.x, cos(a) * p.y + -sin(a) * p.z, sin(a) * p.y + cos(a) * p.z, 0.0);
}

vec4 rotateY(vec4 p, float a) {
    return vec4(cos(a) * p.x + sin(a) + p.z, p.y, -sin(a) * p.x + cos(a) * p.z, 0.0);
}

void main()
{
    vec2 uv = vec2(fs_UV);

    vec2 offset = vec2(0, 0);

    // animated water / lava
    if (fs_UV.z != 0) {
        offset.x = u_Time % 100 * 0.005 / 16;
        uv += offset;
    }

    vec4 diffuseColor = texture(u_Texture, uv);

    // procedural grass
    if (fs_UV.x > 8.f / 16.f &&
        fs_UV.y > 13.f / 16.f &&
        fs_UV.x < 9.f / 16.f &&
        fs_UV.y < 14.f / 16.f) {

        vec3 p = vec3((fs_Pos.xz / 16.f), 1);

        // brownish grass
        vec3 grass_1 = vec3(0.95, 0.65, 0.35);
        // darker green grass
        vec3 grass_2 = vec3(0.5, 0.5, 0.25);

        // using fbm to dynamically color
        vec3 col = mix(grass_2 * diffuseColor.rgb, grass_1 * diffuseColor.rgb, fbm(p));
        vec3 color = mix(col * diffuseColor.rgb, diffuseColor.rgb, fbm(p));

        diffuseColor = vec4(color, diffuseColor.a);
    }

    vec4 lightDir = normalize(vec4(0, 0, -1.0, 0));

    // day and night lighting
    vec3 sunDir = vec3(rotateX(normalize(fs_LightVec), u_Time * 0.008));

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), vec4(normalize(sunDir), 0.0));

    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.2;

    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
                                                        //to simulate ambient lighting. This ensures that faces that are not
                                                        //lit by our point light are not completely black
    // fog
    vec4 fog_color = vec4(0.75, 0.75, 0.75, 1);
    vec4 fog = rotateY(normalize(fog_color), u_Time * 0.011);    // fog color matches color of sky
    float dist = length(fs_Pos.xz - u_Player.xz) * 0.01;        // fog moves with player

    vec4 color = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);
    color = mix(color, fog, pow(smoothstep(0, 1, min(1, dist)), 2));
    out_Col = vec4(color.rgb, diffuseColor.a);
}
