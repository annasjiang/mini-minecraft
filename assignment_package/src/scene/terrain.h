#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "chunk.h"
#include <array>
#include <unordered_map>
#include <unordered_set>
#include "shaderprogram.h"
#include "cube.h"
#include "thread"
#include <vector>
#include <mutex>
#include "river.h"

using namespace std;
using namespace glm;

// Helper functions to convert (x, z) to and from hash map key
int64_t toKey(int x, int z);
glm::ivec2 toCoords(int64_t k);

// The container class for all of the Chunks in the game.
// Ultimately, while Terrain will always store all Chunks,
// not all Chunks will be drawn at any given time as the world
// expands.
class Terrain {
private:
    // Stores every Chunk according to the location of its lower-left corner
    // in world space.
    // We combine the X and Z coordinates of the Chunk's corner into one 64-bit int
    // so that we can use them as a key for the map, as objects like std::pairs or
    // glm::ivec2s are not hashable by default, so they cannot be used as keys.


    std::unordered_map<int64_t, uPtr<Chunk>> newChunks;

    std::unordered_map<int64_t, uPtr<Chunk>> chunksWithBlockData;
    std::mutex chunksWithBlockDataMutex;

    std::unordered_map<int64_t, uPtr<Chunk>> chunksWithVBOData;
    std::mutex chunksWithVBODataMutex;

    std::unordered_map<int64_t, uPtr<Chunk>> m_chunks;
    std::mutex chunksMutex;

    // We will designate every 64 x 64 area of the world's x-z plane
    // as one "terrain generation zone". Every time the player moves
    // near a portion of the world that has not yet been generated
    // (i.e. its lower-left coordinates are not in this set), a new
    // 4 x 4 collection of Chunks is created to represent that area
    // of the world.
    // The world that exists when the base code is run consists of exactly
    // one 64 x 64 area with its lower-left corner at (0, 0).
    // When milestone 1 has been implemented, the Player can move around the
    // world to add more "terrain generation zone" IDs to this set.
    // While only the 3 x 3 collection of terrain generation zones
    // surrounding the Player should be rendered, the Chunks
    // in the Terrain will never be deleted until the program is terminated.
    std::unordered_set<int64_t> m_generatedTerrain;

    OpenGLContext* mp_context;

    std::vector<std::thread> blockWorkerThreads;
    std::vector<std::thread> vboWorkerThreads;

    bool firstTick = true;

public:
    Terrain(OpenGLContext *context);
    ~Terrain();

    uPtr<Chunk> instantiateChunkAt(int x, int z);

    void makeTerrainGenerationZones(int x, int z);

    bool hasTerrainGenerationZoneAt(glm::ivec2);

    void makeVBOData();

    bool hasChunkAt(int x, int z) const;
    bool hasNewChunkAt(int x, int z) const;

    // Assuming a Chunk exists at these coords,
    // return a mutable reference to it
    uPtr<Chunk>& getChunkAt(int x, int z);
    // Assuming a Chunk exists at these coords,
    // return a const reference to it
    const uPtr<Chunk>& getChunkAt(int x, int z) const;


    uPtr<Chunk>& getNewChunkAt(int x, int z);
    const uPtr<Chunk>& getNewChunkAt(int x, int z) const;

    void multithreadedWork(glm::vec3, glm::vec3);

    void tryExpansion(glm::vec3, glm::vec3);
    void checkThreadResults();

    void spawnBlockWorkers(glm::ivec2);
    void blockWorker(uPtr<Chunk>);
    void spawnVBOWorkers();
    void VBOWorker(uPtr<Chunk>);

    BlockType generateBlockTypeByHeight(int, bool);



    // Given a world-space coordinate (which may have negative
    // values) return the block stored at that point in space.
    BlockType getBlockAt(int x, int y, int z) const;
    BlockType getBlockAt(glm::vec3 p) const;

    // Given a world-space coordinate (which may have negative
    // values) set the block at that point in space to the
    // given type.
    void setBlockAt(int x, int y, int z, BlockType t);

    void setNewBlockAt(int x, int y, int z, BlockType t);
    // Draws every Chunk that falls within the bounding box
    // described by the min and max coords, using the provided
    // ShaderProgram
    void draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram);

//    // Initializes the Chunks that store the 64 x 256 x 64 block scene you
//    // see when the base code is run.
//    void CreateTestScene();

    // checks whether a new Chunk should be added to the Terrain based on the
    // Player's proximity to the edge of a Chunk without a neighbor in a particular direction
    void updateScene(glm::vec3 pos);

    void fillColumn(int x, int z);

    void drawRiver();
    void drawTrees();
    void drawTree(int x, int z, int height);
};
