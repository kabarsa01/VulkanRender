# VulkanRender

![alt text](https://github.com/kabarsa01/VulkanRender/blob/master/2021-07-01%2001_10_21-Vulkan%20renderer.png)

Simple vulkan renderer.

	It's a small application to systemize vulkan methodology and hierarchy of vulkan entities.
End goal is to make a lightweight framework to try some available rendering techniques
and concentrate on that instead of boilerplate code needed to make vulkan work.

Overview

Clustered deferred rendering path is used. For now 5 passes are used.

  1. Z-prepass
  2. Compute clustering and world space grid for GI or reservoir sampling
  3. GBuffer pass
  4. LightVisibility/Shadow pass (hardware raytracing only supported)
  5. Deferred lighting for direct light only
  6. RTGI (hardware raytracing only supported)
  7. Post processing

   Material system uses spirv-cross for shader reflection and performs shader parameters matching by name.
Mips are generated at runtime. Assimp and stb_image are used to load resources. One big buffer is used for
transform data, it takes 64MB for now and should store 1024x1024 model matrices. Objects are batched by 
shaders-->material-->uniqueMeshData-->instances. Instancing used as well. 

   At the moment 3 light types supported: directional, spot, point. 
Lights count per view is limited to 1024 lights, it should be enough and reasonably fast to cluster. Many directional lights 
supported just in case. Clustering data is limited to 256 lights per cluster. Overall grid is 32x32x64 with
exponential depth slicing. 

   Memory manager is designed to preallocate 512MB chunks and use freelist with 4KB block.

   Job system is based on quite simple templated job taking something callable. There's a message bus with 
only synchronous notification for now with a plan to be extended with multithreaded messages delivery.
Scene graph is a simple templated octree with some template specialization for SceneObjectBase and it's components.
Octree allocates node pool straight away so several MB being taken for this, I think it's about 16MB to 32MB at the moment.
Frustum culling is implemented on CPU and uses job system. Still need to experiment with some threading schemes for this.

RT Shadows

   Shadows are implemented using hardware ray tracing. So only ray tracing capable gpu's are supported. Soft shadows are
planned using separate RT pass. Shadows use clustered lights data to choose lights influencing current pixel's cluster.

RT GI

   Realtime ray traced global illumination or RTGI used a separate pass and grid storage for lights to accumulate lighting.
Grid hierarchy consists of a series of grids each 5 times bigger than a previous one, it's used in the same way clustering data
used for light influencing screen space, but covering the entire area around camera in world space instead of frustum dependent
view space division in clustering. Hemisphere ray distribution for now is baked in a buffer to give predictable amount of rays 
and directions, maybe some noise/jitter will be introduced later and proper filtering to minimize visual artifacts. RTGI tries
to check if hemisphere ray hit a pixel in screenspace and in this case skips light visibility traces and just relies on direct
lighting data from a previous pass where light visibility was already solved with visibility buffers from RT Shadows Pass.
Still need temporal accumulation and reprojection to save time and reuse data.

![alt text](https://github.com/kabarsa01/VulkanRender/blob/master/2022-01-16%2000_36_23-Vulkan%20renderer.png)
![alt text](https://github.com/kabarsa01/VulkanRender/blob/master/2022-01-16%2000_38_47-Vulkan%20renderer.png)
![alt text](https://github.com/kabarsa01/VulkanRender/blob/master/2022-01-16%2000_39_09-Vulkan%20renderer.png)
![alt text](https://github.com/kabarsa01/VulkanRender/blob/master/2022-01-16%2000_58_12-Vulkan%20renderer.png)
![alt text](https://github.com/kabarsa01/VulkanRender/blob/master/2022-01-16%2001_19_02-Vulkan%20renderer.png)


//---------- Backlog


  Volumetric fog,
  Mesh Shading,
  Occlusion culling,
  Compute Hi-Z generation,
  Async command buffers,
  Async scene processing,


//---------- BackBacklog


  Input system,
  Blender tools to assemble and export full scenes,
  Physics engine integration, ODE, Bullet or whatever

