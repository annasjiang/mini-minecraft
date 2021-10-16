#version 150

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform float u_Time;

out vec4 outColor;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

// sunset palette
const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
                              vec3(254, 192, 81) / 255.0,
                              vec3(255, 137, 103) / 255.0,
                              vec3(253, 96, 81) / 255.0,
                              vec3(57, 32, 51) / 255.0);
// dusk palette
const vec3 dusk[5] = vec3[](vec3(144, 96, 144) / 255.0,
                            vec3(96, 72, 120) / 255.0,
                            vec3(72, 48, 120) / 255.0,
                            vec3(48, 24, 96) / 255.0,
                            vec3(0, 24, 72) / 255.0);

// full sun color
const vec3 sunColor = vec3(255, 255, 190) / 255.0;
const vec3 cloudColor = sunset[3];

// given point as sphere coordinate and map it to uv coordinate
vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    // scale to uv
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

// takes in uv coords of polar coords sphereToUV
// measures just y coord for linearly arranged piecewise values
// gives right sunset color mix depending uv.y
vec3 uvToSunset(vec2 uv) {
    if(uv.y < 0.5) {
        return sunset[0];
    }
    else if(uv.y < 0.55) {
        return mix(sunset[0], sunset[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(sunset[1], sunset[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(sunset[2], sunset[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(sunset[3], sunset[4], (uv.y - 0.65) / 0.1);
    }
    return sunset[4];
}

// gives right dusk color mix depending on the uv.y
vec3 uvToDusk(vec2 uv) {
    if(uv.y < 0.5) {
        return dusk[0];
    }
    else if(uv.y < 0.55) {
        return mix(dusk[0], dusk[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(dusk[1], dusk[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(dusk[2], dusk[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(dusk[3], dusk[4], (uv.y - 0.65) / 0.1);
    }
    return dusk[4];
}

// noise function that returns random 3D point from given seed
vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

// noise function that returns random 2D point from given seed
vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

// sun x axis rotation based on angle
vec4 rotateX(vec3 p, float a) {
    return vec4(p.x, cos(a) * p.y + -sin(a) *p.z, sin(a) * p.y + cos(a) * p.z, 0.0);
}

// 3D worley noise from given uv vector
float WorleyNoise3D(vec3 p) {
    // Tile the space
    vec3 pointInt = floor(p);
    vec3 pointFract = fract(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int z = -1; z <= 1; z++)
    {
        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}

// 2D worley noise from uv coord
float WorleyNoise(vec2 uv)
{
    // Tile the space
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            vec2 neighbor = vec2(float(x), float(y));

            // Random point inside current neighboring cell
            vec2 point = random2(uvInt + neighbor);

            // Animate the point
            point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6.2831 * point); // 0 to 1 range

            // Compute the distance b/t the point and the fragment
            // Store the min dist thus far
            vec2 diff = neighbor + point - uvFract;
            float dist = length(diff);
            minDist = min(minDist, dist);
        }
    }
    return minDist;
}

// computes 3D worley noise at point but changes frequency of
// noise grid to get some worley noise
float worleyFBM(vec3 uv) {
    float sum = 0;
    float freq = 4;
    float amp = 0.5;

    // change frequency and amplitudes
    // sum worley noise each iteration
    for(int i = 0; i < 8; i++) {
        sum += WorleyNoise3D(uv * freq) * amp;
        freq *= 2;
        amp *= 0.5;
    }
    return sum;
}

#define WORLEY_OFFSET
#define SUNSET_THRESHOLD 0.375
#define DUSK_THRESHOLD -0.3

void main()
{
    // ray casting
    // convert to world space
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC
    // got 3D space into pixel space
    // now reproject pixel back out to thrustum
    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world
    // direction of ray
    // draw line from the eye to that point in the thrustum
    vec3 rayDir = normalize(p.xyz - u_Eye);
    // uv coord based on ray direction make quad look like spherical shape
    vec2 uv = sphereToUV(rayDir);
    // offset to get cloud / noise effect in sky based on worley noise
    vec2 offset = vec2(0.0);

#ifdef WORLEY_OFFSET
    // Get a noise value in the range [-1, 1]
    // by using Worley noise as the noise basis of FBM
    offset = vec2(worleyFBM(rayDir));
    offset *= 2.0;
    offset -= vec2(1.0);
#endif

    // Compute a gradient from the bottom of the sky-sphere to the top
    vec3 sunsetColor = uvToSunset(uv + offset * 0.1);   // uv to sunset uses the y coords
    vec3 duskColor = uvToDusk(uv + offset * 0.1);

    // add glowing sun in sky
    // sun direction rotation based on time
    vec3 sunDir = normalize(vec3(rotateX(normalize(vec3(0, 0, -1.f)), u_Time * 0.01)));
    float sunSize = 30.;
    // angle between ray direction and sun direction
    float angle = (acos(dot(rayDir, sunDir)) * 360.0 / PI);
    // ray looking into sky
    float raySunDot = dot(rayDir, sunDir);
    float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);

    // between dusk and sunset
    // Any dot product between 0.0 and -0.375 is a LERP b/t sunset and dusk color
    if (raySunDot > DUSK_THRESHOLD) {
        // if angle between ray dir and vector to sun center is
        // less than threshold we're looking at sun
        if (angle < sunSize) {
            // Full center of sun
            if (angle < 7.5) {
                outColor = vec4(sunColor, 1);
            // corona of sun
            // mix with the current sky color
            } else {
                outColor = vec4(mix(sunColor, mix(sunsetColor, duskColor, t), (angle - 7.5) / 22.5), 1);
            }
            // otherwise color is sunset & dusk
        } else {
            outColor = vec4(mix(sunsetColor, duskColor, t), 1);
        }
     // Any dot product <= -0.3 are pure dusk color
    } else {
        // when no sun
        outColor = vec4(mix(sunsetColor, duskColor, t), 1);
    }
}
