# VulkanRender
Simple vulkan renderer.

It's a small application to systemize vulkan methodology and hierarchy of vulkan entities.
End goal is to make a lightweight framework to try some available rendering techniques
and concentrate on that instead of boilerplate code needed to make vulkan work.

Overview

Clustered deferred rendering path is used. For now 5 passes are used.

  1. Z-prepass
  2. Compute clustering
  3. GBuffer pass
  4. Deferred lighting
  5. Post processing

Material system uses spirv-cross for shader reflection and performs shader parameters matching by name.
Mips are generated at runtime. Assimp and stb_image are used to load resources. Shadows are not there atm,
because I might do it all eventually with mesh shading and abandon traditional vertex pipeline. IBL for PBR
is not there too, I'd be glad to replace it all with raytracing solutions. One big buffer is used for
transform data, it takes 64MB for now and should store 1024x1024 model matrices. Scene is very primitive 
with no accelerations structure used. Objects are batched by shaders-->material-->uniqueMeshData-->instances.
Instancing used as well. At the moment 3 light types supported: directional, spot, point. Lights count per
view is limited to 1024 lights, it should be enough and reasonably fast to cluster. Many directional lights 
supported just in case. Clustering data is limited to 256 lights per cluster. Overall grid is 32x32x64 with
exponential depth slicing. Memory manager uses 4 types of memomry chunks, which use buddy system for a specific
minimal allocatable size. They use it avoid fragmentation, memory chunk is divided in 2048 smallest slots
with the ability for data to occupy several slots. With this scheme memory managing structure is a tree packed
into array where each tree layer nodes are packed after previous layer nodes, and have 12 layers atm, which gives
2048 leaf nodes (2^11). The biggest and smallest memory chunks you can see here. It's not final, some ended up
being kind of useless

64 << sizeRangeShift = 512          :: 64 * 2048     = 128K chunk

1024 << sizeRangeShift = 8K         :: 1024 * 2048   = 2M chunk

16K << sizeRangeShift = 128K        :: 16K * 2048    = 32M chunk

256K << sizeRangeShift = 2M         :: 256K * 2048   = 512M chunk

Job system is based on quite simple templated job taking something callable. There's a message bus with only synchronous
notification for now with a plan to be extended with multithreaded messages delivery.


Backlog



  Volumetric fog

  IBL

  Mesh Shading

  Shadow cache ?

  Raytracing / SSR ?

  SVOGI ?

  Proper scene graph

  Occlusion culling

  Compute Hi-Z generation

  Async command buffers

  Async scene processing



BackBacklog



  Input system

  Blender tools to assemble and export full scenes

  Physics engine integration, ODE, Bullet or whatever

