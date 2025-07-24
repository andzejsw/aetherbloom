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
- More

#### Building

##### Windows
Unlike main repo, i decided to to build on Windows using wsl. Eveerything neccessary is building already.

##### TODO List
1. Face Culling during Chunk Meshing ✔
    * Description: When generating the mesh for a chunk, only create faces that are exposed to the air or a transparent block. If a block
        face is adjacent to an opaque block (e.g., a dirt block next to another dirt block), that shared face is not added to the mesh. This
        significantly reduces the number of triangles that need to be rendered.
    * Ease of Implementation: Medium. Requires modifying the chunk meshing logic (src/world/blockmesh.c) to check neighboring blocks for
        opacity before adding a face.
    * Performance/FPS Gains: High. This is one of the most fundamental and impactful optimizations for voxel games, as it drastically
        reduces the amount of geometry sent to the GPU.

2. Frustum Culling (Chunk Level) ✔
    * Description: Before rendering, check if an entire chunk's bounding box is within the camera's view frustum (the visible area). If a
        chunk is completely outside this frustum, it is skipped entirely, preventing unnecessary rendering calls.
    * Ease of Implementation: Medium. Involves calculating the bounding volume for each chunk and implementing a frustum-culling test
        against the camera's view frustum (likely in src/gfx/renderer.c or a related camera utility).
    * Performance/FPS Gains: High. Reduces the number of draw calls and the amount of geometry processed by the GPU, especially effective
        when the player is in an enclosed space or looking at a small part of the world.

3. Greedy Meshing
    * Description: Instead of rendering each individual block face as a separate quad, this algorithm combines adjacent, identical faces
        into larger quads. For example, a 2x2 wall of dirt blocks would be represented by one large 2x2 quad instead of four 1x1 quads.
    * Ease of Implementation: Hard. This is a more complex algorithm to implement within your src/world/blockmesh.c, requiring careful
        handling of block types, textures, and face orientations to merge them correctly.
    * Performance/FPS Gains: Very High. Drastically reduces the vertex count and the number of draw calls, especially for large, flat
        surfaces, leading to significant performance improvements.

4. Asynchronous Chunk Loading/Generation
    * Description: Perform the computationally intensive tasks of chunk generation and meshing on separate threads, rather than on the
        main rendering thread. This prevents the game from "stuttering" or "hitching" when new chunks are loaded or generated as the player
        moves.
    * Ease of Implementation: Hard. Introduces complexities related to multi-threading, synchronization (ensuring data consistency
        between threads), and efficient data transfer from worker threads back to the main rendering thread.
    * Performance/FPS Gains: High (smoothness). While it might not directly increase the raw FPS number, it significantly improves the
        perceived smoothness of the game by eliminating hitches caused by synchronous chunk operations, making the game feel much more
        responsive.
