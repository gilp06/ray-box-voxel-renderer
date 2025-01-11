#version 460 core

struct VertexOutput {
    vec4 position;
    vec3 color;
};


layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

layout(std140) uniform ubo {
    mat4 proj;
    mat4 view_rot;
    mat4 view_pos;
    mat4 u_inv_proj_view_rot;
    vec4 uViewport;
};


mat4 getViewRotation(in mat4 view) {
    return mat4(mat3(view));
};

out VertexOutput vertexOutput;

void main() {
    vertexOutput.position = view_pos * (vec4(aPos,1.0) + vec4(0.5,0.5,0.5,0.0) * 1.0);
    vertexOutput.color = aColor;

    gl_Position = proj * view_rot * vertexOutput.position;
    float ratio = uViewport.z / uViewport.w;
    float reduce = max(
		abs( gl_Position.x*ratio/gl_Position.w  ),
		abs( gl_Position.y/gl_Position.w  )
	);

    reduce += 0.03;
    float scale = 1.05;
    float size = (uViewport.w*scale) / gl_Position.z * max(reduce, 1.0);
    gl_PointSize = size;
}