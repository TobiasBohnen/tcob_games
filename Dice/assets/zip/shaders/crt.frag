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

float hash(uint x) {
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = (x >> 16) ^ x;
    return float(x) / 4294967295.0;
}

float hash2D(ivec2 p) {
    return hash(uint(p.x) + uint(p.y) * 374761393u);
}

void main() {
    vec2 uv = fs_in.tex_coords.xy;
    ivec2 ts = textureSize(texture0, 0).xy;
    ivec2 pixel_coords = ivec2(uv * vec2(ts));
    
    float time_seed = floor(global.time / 80.0); 
    ivec2 block_id = ivec2(pixel_coords.x / 24, pixel_coords.y / 1);
    
    float glitch_roll = hash2D(block_id * ivec2(127, 311) + ivec2(int(time_seed) * 73, int(time_seed) * 199));
    
    float shift = 0.0;
    float flicker = 1.0;
    
    if (glitch_roll < 0.01) {
        float jitter_time = floor(global.time / 40.0);
        ivec2 random_seed = ivec2(int(jitter_time * 73.0), int(glitch_roll * 9973.0));
        float h = hash2D(block_id + random_seed);
        shift = (h * 2.0 - 1.0) * 0.015;
        
        ivec2 flicker_seed = ivec2(block_id.y * 127 + int(jitter_time), block_id.x * 311);
        flicker = 0.8 + 0.4 * hash2D(flicker_seed);
    }
    
    float r = texture(texture0, vec3(uv.x + shift, uv.y, fs_in.tex_coords.z)).r;
    float g = texture(texture0, vec3(uv.x,         uv.y, fs_in.tex_coords.z)).g;
    float b = texture(texture0, vec3(uv.x - shift, uv.y, fs_in.tex_coords.z)).b;
    
    vec4 base = vec4(r, g, b, 1.0);
    
    float scanline = (fract(float(pixel_coords.y) * 0.5) < 0.5) ? 1.0 : 0.7;
    
   vec3 masks[3] = vec3[3](
        vec3(1.05, 0.95, 0.90),
        vec3(0.90, 1.05, 0.95),
        vec3(0.95, 0.90, 1.05)
    );
    vec3 mask = masks[pixel_coords.x % 3];
    
    fragColor = vec4(base.rgb * scanline * mask * flicker, base.a);
}