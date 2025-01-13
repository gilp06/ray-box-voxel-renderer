# Voxel Engine using Ray-Box splatting

This is an implementation of the [A Ray-Box Intersection Algorithm and Efficient Dynamic Voxel](https://jcgt.org/published/0007/03/04/) pipeline, mainly referencing the Ray-Box implementation by [kosshi-net](https://github.com/kosshi-net/voxplat) but written in C++.

In case you haven't read the paper, this is a hybrid approach compared to traditional rasterization and ray tracing. For each voxel, you send a single point in the vertex shader, and draw using GL_POINTS. The vertex shader expands the size of the point to cover the bounding sphere of the voxel. The fragment shader then ray-traces the voxel. This is nice since the further you are from a voxel the less GPU performance it consumes, in addition to the little amount of data required to send compared to rasterization. The voxplot repository linked above handles the case of voxels too close to the screen by rasterizing chunks near the player, however I haven't found this to be too detrimental yet.

The chunk system is currently a 16x16x16 flat array z-curve ordered containing uint_8ts referencing an index in a global block info palette, loaded via json. It additionally supports compression in memory using RLE, however the feature is currently disabled right now.

Each chunk is rendered individually, with each solid block given a u8vec3 for position within the chunk and a uint32_t for color from the palette.

Below is the engine rendering roughly 26 million untextured cubes on a RTX 3070 at 75 fps without any lod or culling.

![render of cubes](images/render.png)
