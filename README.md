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
    * Frustum Culling

#### Building

##### Windows
Unlike main repo, i decided to to build on Windows using wsl. Eveerything neccessary is building already.

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
