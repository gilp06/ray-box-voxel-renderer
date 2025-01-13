#version 460 core

struct VertexOutput {
    vec4 position;
    vec3 color;
};


layout(location = 0) in uvec3 aPos;
layout(location = 1) in uint aColor;

layout(std140) uniform ubo {
    mat4 proj;
    mat4 view_rot;
    mat4 view_pos;
    mat4 u_inv_proj_view_rot;
    vec4 uViewport;
};

uniform ivec3 chunk_pos;
uniform uint CHUNK_SIZE = 32;


out VertexOutput vertexOutput;



mat4 getViewRotation(in mat4 view) {
    return mat4(mat3(view));
};

vec4 unpackColor(in uint color) {
    return vec4(
        float((color & 0xFF000000) >> 24) / 255.0,
        float((color & 0x00FF0000) >> 16) / 255.0,
        float((color & 0x0000FF00) >> 8) / 255.0,
        float((color & 0x000000FF)) / 255.0
    );
};

void main() {
    vertexOutput.position = view_pos * (vec4((int(CHUNK_SIZE) * chunk_pos + ivec3(aPos)),1.0) + vec4(0.5,0.5,0.5,0.0) * 1.0);
    vertexOutput.color = unpackColor(aColor).rgb;

    gl_Position = proj * view_rot * vertexOutput.position;
    float ratio = uViewport.z / uViewport.w;
    float reduce = max(
		abs( gl_Position.x*ratio/gl_Position.w  ),
		abs( gl_Position.y/gl_Position.w  )
	);

    reduce += 0.2; // larger size = cleaner normals but worse performance
    float scale = 1.05;
    float size = (uViewport.w*scale) / gl_Position.z * max(reduce, 1.0);
    gl_PointSize = size;
}