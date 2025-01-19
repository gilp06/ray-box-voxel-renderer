#version 460 core

struct VertexOutput {
    vec4 position;
    vec3 color;
};

layout(std140) uniform ubo {
    mat4 proj;
    mat4 view_rot;
    mat4 view_pos;
    mat4 u_inv_proj_view_rot;
    vec4 uViewport;
};



in VertexOutput vertexOutput;
flat in int lod;
out vec4 FragColor;

vec3 ss2wsVec(float x, float y){
	vec4 ray_clip = vec4(
		 x * 2.0 - 1.0,
		 y * 2.0 - 1.0,
		-1.0, 1.0
	);

	vec4 ray = u_inv_proj_view_rot * ray_clip;

	return normalize(ray.xyz/ray.w);
}

vec2 AABBIntersect(vec3 ro, vec3 rd, vec3 minV, vec3 maxV)
{
    vec3 invR = 1.0 / rd;

    float t0, t1;

    vec3 tbot = (minV - ro) * invR;
    vec3 ttop = (maxV - ro) * invR;

    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);


    vec2 t = max(tmin.xx, tmin.yz);

    t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    t1 = min(t.x, t.y);

    return vec2(t0, t1); 
	// if (t0 <= t1) { did hit } else { did not hit }
}


void main() {

    vec3 light = normalize(vec3(-1.0, 1.0, 0));
    vec3 vxl = vertexOutput.position.xyz;
    vec3 ray = ss2wsVec(gl_FragCoord.x/uViewport.z, gl_FragCoord.y/uViewport.w);
    vec2 result = AABBIntersect(vec3(0.0), ray, vec3(vxl - 0.5 * lod), vec3(vxl + 0.5 * lod));
    if(result.x > result.y) {
        discard;
    }

    vec3 hit = vxl - ray * result.x;
    vec3 hit_abs = abs(hit);
    float max_dim = max(max(hit_abs.x, hit_abs.y), hit_abs.z);
    vec3 normal = vec3 (
        float(hit_abs.x == max_dim),
        float(hit_abs.y == max_dim),
        float(hit_abs.z == max_dim)
    )*sign(hit);

    vec3 color;
    vec3 r = -normal;
    color = vertexOutput.color * max( 0.7, sign( dot(r, light) ) ) * float(result.x > 0.0);

    gl_FragDepth = result.x / 1000.0;
    FragColor = vec4(color, 1.0);
}