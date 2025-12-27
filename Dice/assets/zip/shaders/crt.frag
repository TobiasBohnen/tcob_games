#version 450 core
layout(std140, binding = 0) uniform Globals {
    mat4  camera;
    uvec2 view_size;
    ivec2 mouse_pos;
    float time;
    bool  debug;
} global;

layout(std140, binding = 1) uniform Material {
    vec4  color;
    float point_size;
} material;

layout(location = 0) out vec4 fragColor;

layout(location = 0) in VS_OUT {
    vec4 color;
    vec3 tex_coords;
} fs_in;

layout(binding = 0) uniform sampler2DArray texture0;

float hash(ivec2 p) {
    uint x = uint(p.x) + uint(p.y) * 374761393u;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = (x >> 16) ^ x;
    return float(x) / 4294967295.0;
}

vec2 curveUV(vec2 uv) {
    uv.y = abs(uv.y);
    uv = uv * 2.0 - 1.0;

    float curvature = 7.0;
    vec2 offset = abs(uv.yx) / curvature;
    uv = uv + uv * offset * offset;
    
    uv = uv * 0.5 + 0.5;
    uv.y = -uv.y;
    
    return uv;
}

void main() {
    vec2 uv = curveUV(fs_in.tex_coords.xy);
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y > 0.0 || uv.y < -1.0) {
        discard;
    }

    ivec2 tex_size      = textureSize(texture0, 0).xy;
    ivec2 pixel_coords  = ivec2(uv.x * tex_size.x, abs(uv.y) * tex_size.y);
    ivec2 time_seed     = ivec2(int(floor(global.time / 80.0))); 
    ivec2 block_id      = ivec2(pixel_coords.x / (12.0 + 12.0 * hash(time_seed)), pixel_coords.y);
    float glitch_roll   = hash(block_id * ivec2(127, 311) + time_seed);
    
    float shift         = 0.0;
    float flicker       = 1.0;
    
    if (glitch_roll < 0.008) {
        float jitter_time  = floor(global.time / 40.0);
        flicker            = 0.8 + 0.4 * hash(ivec2(block_id.y + int(jitter_time), block_id.x));
        shift              = (hash(block_id + ivec2(int(jitter_time))) * 2.0 - 1.0) * 0.01;
    }
    
    float r   = texture(texture0, vec3(uv.x + shift, uv.y, fs_in.tex_coords.z)).r;
    float g   = texture(texture0, vec3(uv.x,         uv.y, fs_in.tex_coords.z)).g;
    float b   = texture(texture0, vec3(uv.x - shift, uv.y, fs_in.tex_coords.z)).b;
    vec3 rgb  = vec3(r, g, b);
    
    vec3 ghost      = texture(texture0, vec3(uv.x - 0.008, uv.y, fs_in.tex_coords.z)).rgb * 0.1;
    rgb += ghost;

    float dotcrawl  = sin((float(pixel_coords.x) + float(pixel_coords.y) + global.time / 0.5) * 3.14159) * 0.05 * length(rgb);
    rgb += dotcrawl;

    float noise     = (hash(pixel_coords + ivec2(int(global.time * 5.0))) - 0.5) * 0.02;
    rgb += noise;

    float scanline  = 0.92 + 0.08 * cos(float(pixel_coords.y) * 3.14159);

    float mask      = 0.96 + 0.04 * sin(float(pixel_coords.x) * 2.094);  
    
    vec2 vignetteUV = vec2(uv.x, abs(uv.y)) * 2.0 - 1.0;
    float vignette  = pow(1.0 - dot(vignetteUV, vignetteUV) * 0.55, 0.8);    
    
    fragColor = vec4(rgb * scanline * mask * flicker * vignette, 1);
}