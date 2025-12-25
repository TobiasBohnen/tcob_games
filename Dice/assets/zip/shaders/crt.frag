#version 450 core

layout(std140, binding = 0) uniform Globals
{
    mat4  camera;
    uvec2 view_size;
    ivec2 mouse_pos;
    float time;
    bool  debug;
} global;

layout(std140, binding = 1) uniform Material
{
    vec4  color;
    float point_size;
} material;

layout(location = 0) out vec4 fragColor;

layout(location = 0) in VS_OUT
{
    vec4 color;
    vec3 tex_coords;   // xy = UV, z = layer
} fs_in;

layout(binding = 0) uniform sampler2DArray texture0;

float hash(ivec2 p)
{
    uint x = uint(p.x) * 1973u + uint(p.y) * 9277u + 0x68bc21ebu;
    x = (x << 13u) ^ x;
    uint h = x * (x * x * 15731u + 789221u) + 1376312589u;
    return float(h & 0x7fffffffu) / 2147483647.0;
}

vec4 sample_tex_offset(ivec2 offset, vec2 uv)
{
    ivec2 ts = textureSize(texture0, 0).xy;
    vec2 texel = 1.0 / vec2(ts);
    return texture(texture0, vec3(uv + vec2(offset) * texel,
                                  fs_in.tex_coords.z));
}

void main()
{
    ivec2 ts = textureSize(texture0, 0).xy;
    int x = int(fs_in.tex_coords.x * ts.x);
    int y = int(fs_in.tex_coords.y * ts.y);

    vec2 uv = fs_in.tex_coords.xy;

    const float glitchProb = 0.04;
    float rnd = hash(ivec2(x / 8, y / 2)
                     + ivec2(int(global.time * 6.0), 0));

    float flicker = 1.0;

    if (rnd < glitchProb)
    {
        float offset =
            (hash(ivec2(x, y)
            + ivec2(int(global.time * 37.0), 0)) - 0.5) * 0.006;

        uv.x += offset;

        flicker = 0.9 + 0.1 *
            hash(ivec2(x, y)
            + ivec2(int(global.time * 5.0), 0));
    }

    vec4 center = sample_tex_offset(ivec2(0, 0), uv);
    vec4 right  = sample_tex_offset(ivec2(1, 0), uv);
    vec4 down   = sample_tex_offset(ivec2(0, 1), uv);

    vec4 blended = mix(center, right, 0.2);
    blended = mix(blended, down, 0.2);

    vec4 base = blended * fs_in.color * material.color;

    float scan = ((y & 1) == 0) ? 1.0 : 0.75;

    int col = x % 3;
    float mask =
        (col == 0) ? 1.00 :
        (col == 1) ? 0.92 :
                     0.85;

    float intensity = scan * mask;

    fragColor = vec4(base.rgb * intensity * flicker, base.a);
}
