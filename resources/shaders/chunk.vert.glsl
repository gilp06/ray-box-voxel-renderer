#version 460 core

struct VertexOutput {
    vec4 position;
    vec3 color;
};

float random(uint seed) {
    return fract(sin(float(seed) * 12.9898) * 43758.5453);
}


layout(location = 0) in ivec3 aPos;
layout(location = 1) in uint aColor;

layout(std140) uniform ubo {
    mat4 proj;
    mat4 view_rot;
    mat4 view_pos;
    mat4 u_inv_proj_view_rot;
    vec4 uViewport;
};



out VertexOutput vertexOutput;
flat out int lod;



mat4 getViewRotation(in mat4 view) {
    return mat4(mat3(view));
};

vec4 unpackData(in uint color) {
    return vec4(
        float((color & 0xFF000000) >> 24) / 255.0,
        float((color & 0x00FF0000) >> 16) / 255.0,
        float((color & 0x0000FF00) >> 8) / 255.0,
        float(color & 0x000000FF)
    );
};

void main() {
    vec4 block_data = unpackData(aColor);
    lod = int(block_data.a);
    vertexOutput.position = view_pos * (vec4(aPos,1.0) + vec4(0.5,0.5,0.5,0.0) * lod);
    float color_shift = random(abs(aPos.x) ^ abs(aPos.y) ^ abs(aPos.z));
    // clamp to 0.1 - 0.2
    color_shift = -0.15 + color_shift * 0.3;

    vertexOutput.color = block_data.rgb + vec3(color_shift);

    gl_Position = proj * view_rot * vertexOutput.position;
    float ratio = uViewport.z / uViewport.w;
    float reduce = max(
		abs( gl_Position.x*ratio/gl_Position.w  ),
		abs( gl_Position.y/gl_Position.w  )
	);

    reduce += 0.02; // larger size = cleaner normals but worse performance
    float scale = 1.05;
    float size = (uViewport.w*scale) / gl_Position.z * max(reduce, 1.0) * lod;

    gl_PointSize = size;
}