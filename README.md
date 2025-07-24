# Minecraft like game: Aetherbloom *

![screenshot](screenshots/1.png)

#### Features:
- Infinite, procedurally generated world
- Infinite height/depth
- Day/night cycle
- Biomes
- ECS-driven player and entities with full collision and movement
- Full RGB lighting
- Full transparency + translucency support
- Sprite blocks (flowers)
- Animated blocks (water + lava)
- Distance fog
- A whole lot of different block types
- Optimisations
    * Multi threaded chunk meshing
    * Face Culling during Chunk Meshing
    * Frustum Culling (For chunks and light updates in them)
    * Optimized `glBufferData` Usage

#### Building

##### Windows
Unlike main repo, i decided to build on Windows using WSL from VS Code.
Here are the packages you'll need to install in your WSL environment (assuming a Debian/Ubuntu-based distribution)            
   1. `mingw-w64`: This package provides the x86_64-w64-mingw32-gcc compiler and windres utility, which are essential for compiling Windows
      executables and resources.
   2. `make`: To execute the Makefile.
   3. `cmake`: Used for building the cglm and glfw libraries.

You can install them using the following commands in your WSL terminal:
   1. `sudo apt update`
   2. `sudo apt install -y mingw-w64 make cmake`

To build project, run command `make` from root directory. Game will be created in bin directory. Run ./bin/game.exe from terminal to run the game and see it loging output in console.

##### TODO List
1. Greedy Meshing
    * Description: Instead of rendering each individual block face as a separate quad, this algorithm combines adjacent, identical faces
        into larger quads. For example, a 2x2 wall of dirt blocks would be represented by one large 2x2 quad instead of four 1x1 quads.
    * Ease of Implementation: Hard. This is a more complex algorithm to implement within your src/world/blockmesh.c, requiring careful
        handling of block types, textures, and face orientations to merge them correctly.
    * Performance/FPS Gains: Very High. Drastically reduces the vertex count and the number of draw calls, especially for large, flat
        surfaces, leading to significant performance improvements.

2. Asynchronous Chunk Loading/Generation
    * Description: Perform the computationally intensive tasks of chunk generation and meshing on separate threads, rather than on the
        main rendering thread. This prevents the game from "stuttering" or "hitching" when new chunks are loaded or generated as the player
        moves.
    * Ease of Implementation: Hard. Introduces complexities related to multi-threading, synchronization (ensuring data consistency
        between threads), and efficient data transfer from worker threads back to the main rendering thread.
    * Performance/FPS Gains: High (smoothness). While it might not directly increase the raw FPS number, it significantly improves the
        perceived smoothness of the game by eliminating hitches caused by synchronous chunk operations, making the game feel much more
        responsive.

3. Batch Rendering (Merge VBOs/VAOs):
       * Description: Instead of drawing each chunk individually, combine the mesh data of multiple adjacent chunks into a single, larger VBO
          and VAO. This significantly reduces the number of OpenGL draw calls, which is often a major CPU bottleneck. You would still use
         frustum culling to determine which chunks are visible, but then group their data for a single draw call.
       * Implementation Difficulty: Medium to High.
           * Requires a system to manage the combined VBOs (e.g., dynamic allocation, sub-data updates).
           * Needs careful handling of texture atlases and UV coordinates if textures are not uniform across combined chunks.
           * Updating a single block might require re-meshing and re-uploading a larger portion of the combined VBO.
       * Benefits: High. Can provide a significant FPS boost, especially when many chunks are visible. Reduces CPU overhead dramatically.

4. Dynamic Level of Detail (LOD) for Chunks:
    * Description: Render distant chunks with a lower level of detail (e.g., simpler meshes, fewer vertices, or even just a single block
        representation) to reduce rendering load. As the player moves closer, switch to higher detail meshes.
    * Implementation Difficulty: Medium.
        * Requires generating multiple mesh versions for chunks or a simplified rendering approach for distant chunks.
        * Needs a system to manage LOD transitions.
    * Benefits: Medium to High. Reduces GPU load for distant geometry, allowing for a larger render distance with less performance
        impact.

5. Occlusion Culling (Beyond Frustum Culling):
    * Description: Implement a system to prevent rendering of chunks or parts of chunks that are completely hidden by other opaque
        geometry (e.g., a mountain in front of a chunk). This is more advanced than frustum culling, which only checks if an object is
        outside the camera's view.
        * Requires generating multiple mesh versions for chunks or a simplified rendering approach for distant chunks.
        * Needs a system to manage LOD transitions.
    * Benefits: Medium to High. Reduces GPU load for distant geometry, allowing for a larger render distance with less performance
        impact.
