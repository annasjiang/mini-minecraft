#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>
#include <scene/procgen.h>

#include <thread>
#include <mutex>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context)
{}

Terrain::~Terrain() {
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const {
    if (hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));

    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}

bool Terrain::hasNewChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));

    return newChunks.find(toKey(16 * xFloor, 16 * zFloor)) != newChunks.end();
}

uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));

    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}

const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));

    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

uPtr<Chunk>& Terrain::getNewChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));

    return newChunks[toKey(16 * xFloor, 16 * zFloor)];
}

const uPtr<Chunk>& Terrain::getNewChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));

    return newChunks.at(toKey(16 * xFloor, 16 * zFloor));
}

bool Terrain::hasTerrainGenerationZoneAt(glm::ivec2 zone) {
    return m_generatedTerrain.find(toKey(zone.x, zone.y)) != m_generatedTerrain.end();
}

void Terrain::multithreadedWork(glm::vec3 currPlayerPos, glm::vec3 prevPlayerPos) {
    tryExpansion(currPlayerPos, prevPlayerPos);
    checkThreadResults();
}

std::vector<glm::ivec2> getTerrainGenerationZonesAround(glm::vec2 pos, int n) {
    std::vector<glm::ivec2> tgzs = {};
    int halfWidth = glm::floor(n / 2.f) * 64;

    for (int i = pos.x - halfWidth; i <= pos.x + halfWidth; i += 64) {
        for (int j = pos.y - halfWidth; j <= pos.y + halfWidth; j+= 64) {
            tgzs.push_back(glm::ivec2(i, j));
        }
    }

    return tgzs;
}

std::vector<glm::ivec2> diffVectors(std::vector<glm::ivec2> a, std::vector<glm::ivec2> b) {
    std::vector<glm::ivec2> diff;

    for (glm::ivec2 vecB : b) {
        bool match = false;
        for (glm::ivec2 vecA : a) {
            if (vecA.x == vecB.x && vecA.y == vecB.y) {
                match = true;
                break;
            }
        }

        if (!match) {
            diff.push_back(vecB);
        }
    }

    return diff;
}

void Terrain::tryExpansion(glm::vec3 currPlayerPos, glm::vec3 prevPlayerPos) {
    glm::ivec2 currZone = glm::ivec2(glm::floor(currPlayerPos.x / 64.f) * 64.f,
                                     glm::floor(currPlayerPos.z / 64.f) * 64.f);
    glm::ivec2 prevZone = glm::ivec2(glm::floor(prevPlayerPos.x / 64.f) * 64.f,
                                     glm::floor(prevPlayerPos.z / 64.f) * 64.f);

    std::vector<glm::ivec2> currTGZs = getTerrainGenerationZonesAround(currZone, 5);
    std::vector<glm::ivec2> prevTGZs = getTerrainGenerationZonesAround(prevZone, 5);

    std::vector<glm::ivec2> newZones = firstTick ? currTGZs : diffVectors(prevTGZs, currTGZs);
    std::vector<glm::ivec2> oldZones = diffVectors(currTGZs, prevTGZs);

    for (glm::ivec2 newZone : newZones) {
        if (!hasTerrainGenerationZoneAt(newZone)) {
            m_generatedTerrain.insert(toKey(newZone.x, newZone.y));

            for (int x = 0; x < 64; x += 16) {
                for (int z = 0; z < 64; z += 16) {
                    newChunks[toKey(newZone.x + x, newZone.y + z)] = instantiateChunkAt(newZone.x + x, newZone.y + z);
                }
            }
        }
    }


    for (auto & [ key, chunk ]: newChunks) {
        int x = chunk->getWorldPos().x;
        int z = chunk->getWorldPos().y;

        if (hasChunkAt(x, z + 16)) {
            auto &chunkNorth = getChunkAt(x, z + 16);
            chunk->linkNeighbor(chunkNorth, ZPOS);
        }
        if (hasChunkAt(x, z - 16)) {
            auto &chunkSouth = getChunkAt(x, z - 16);
            chunk->linkNeighbor(chunkSouth, ZNEG);
        }
        if (hasChunkAt(x + 16, z)) {
            auto &chunkEast = getChunkAt(x + 16, z);
            chunk->linkNeighbor(chunkEast, XPOS);
        }
        if (hasChunkAt(x - 16, z)) {
            auto &chunkWest = getChunkAt(x - 16, z);
            chunk->linkNeighbor(chunkWest, XNEG);
        }

        if (hasNewChunkAt(x, z + 16)) {
            auto &chunkNorth = getNewChunkAt(x, z + 16);
            chunk->linkNeighbor(chunkNorth, ZPOS);
        }
        if (hasNewChunkAt(x, z - 16)) {
            auto &chunkSouth = getNewChunkAt(x, z - 16);
            chunk->linkNeighbor(chunkSouth, ZNEG);
        }
        if (hasNewChunkAt(x + 16, z)) {
            auto &chunkEast = getNewChunkAt(x + 16, z);
            chunk->linkNeighbor(chunkEast, XPOS);
        }
        if (hasNewChunkAt(x - 16, z)) {
            auto &chunkWest = getNewChunkAt(x - 16, z);
            chunk->linkNeighbor(chunkWest, XNEG);
        }
    }

    for (auto & [ key, chunk ]: newChunks) {
        blockWorkerThreads.push_back(std::thread(&Terrain::blockWorker, this, move(chunk)));
    }

    newChunks.clear();
    firstTick = false;
}

uPtr<Chunk> Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context);
    chunk.get()->setWorldPos(x, z);

    return chunk;
}

BlockType Terrain::generateBlockTypeByHeight(int height, bool isTop) {

    if (height < 120) {
        return STONE;
    }

    if (height < 128) {
        return SAND;
    }

    if (height < 160) {
        return isTop ? GRASS : DIRT;
    }

    if (height > 200) {
        return isTop ? SNOW : STONE;
    }

    return STONE;
}

void Terrain::blockWorker(uPtr<Chunk> chunk) {
    glm::ivec2 chunkWorldPos = chunk->getWorldPos();

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int height = ProcGen::getHeight(chunkWorldPos.x + x, chunkWorldPos.y + z);

            for (int k = 0; k <= height; k++) {
                chunk->setBlockAt(x, k, z, generateBlockTypeByHeight(height, k == height));
            }
        }
    }

    chunksWithBlockDataMutex.lock();
    chunksWithBlockData[toKey(chunk->getWorldPos().x, chunk->getWorldPos().y)] = move(chunk);
    chunksWithBlockDataMutex.unlock();
}

void Terrain::checkThreadResults() {
    chunksWithBlockDataMutex.lock();
    for (auto & [ key, chunk ] : chunksWithBlockData) {
        vboWorkerThreads.push_back(std::thread(&Terrain::VBOWorker, this, move(chunk)));
    }
    chunksWithBlockData.clear();
    chunksWithBlockDataMutex.unlock();

    chunksWithVBODataMutex.lock();
    for (auto & [ key, chunk ] : chunksWithVBOData) {
        chunk->create();
        m_chunks[key] = move(chunk);
    }
    chunksWithVBOData.clear();
    chunksWithVBODataMutex.unlock();
}

void Terrain::VBOWorker(uPtr<Chunk> chunk) {
    chunk->generateVBOData();

    chunksWithVBODataMutex.lock();
    chunksWithVBOData[toKey(chunk->getWorldPos().x, chunk->getWorldPos().y)] = move(chunk);
    chunksWithVBODataMutex.unlock();
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t) {
    if (hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    chunksWithBlockDataMutex.lock();

    for(int x = minX; x < maxX; x += 16) {
         for(int z = minZ; z < maxZ; z += 16) {
             if (hasChunkAt(x, z)) {
                 const uPtr<Chunk> &chunk = getChunkAt(x, z);
                 shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
                 shaderProgram->drawOpq(*chunk);
             }
         }
     }

     for(int x = minX; x < maxX; x += 16) {
         for(int z = minZ; z < maxZ; z += 16) {
             if (hasChunkAt(x, z)) {
                 const uPtr<Chunk> &chunk = getChunkAt(x, z);
                 shaderProgram->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(x, 0, z)));
                 shaderProgram->drawTrans(*chunk);
             }
         }
     }

    chunksWithBlockDataMutex.unlock();
}

void Terrain::drawRiver() {
    River river = River(vec2(12.f, 15.f), vec2(0.5f, 1.0f), 8.f, "FFGGGX", 2, 0.8f);
    // draw river
    for (int i = 0; i < river.m_path.length(); i++) {
        vec2 start = river.m_turtle.m_position;
        (river.*(river.m_charToDrawingOperation[river.m_path[i]]))();
        vec2 end = river.m_turtle.m_position;

        if (river.m_path[i] == 'F' || river.m_path[i] == 'G') {
            int zMin = end[1];
            if (start[1] <= end[1]) {
                zMin = start[1];
            }

            int zMax = start[1];
            if (end[1] >= start[1]) {
                zMax = end[1];
            }

            // calc radius
            int radius = 2;
            if (river.m_turtle.m_depth < 5) {
                radius = (5 - river.m_turtle.m_depth);
            }

            // skip straight line
            if (start[1] == end[1]) {
                continue;
            }

            for (int z = zMin; z <= zMax; z++) {
                int waterLevel = 128;
                // get x-intercept
                float xIntercept = start[0];
                if (start[0] != end[0]) {
                    xIntercept = (z - start[1]) / ((start[1] - end[1]) / (start[0] - end[0])) + start[0];
                }
                int l = floor(xIntercept);

                for (int x = -radius; x <= radius; x++) {
                    // get rid of every block above river
                    for (int y = waterLevel; y < 256; y++) {
                        setBlockAt(l + x, y, z, EMPTY);
                    }

                    // add water
                    for (int y = -radius; y < 0; y++) {
                        float dist = length(vec3(l + x, waterLevel + y, z) - vec3(l, waterLevel, z));
                        if (dist < radius) {
                            if (waterLevel + y < waterLevel) {
                                setBlockAt(l + x, waterLevel + y, z, WATER);
                            }
                        }
                    }

                }
            }
        }
    }
}

void Terrain::drawTrees() {
    for (int i = 0; i < 15; i++) {
        int x = rand() % 80 + 1;
        int z = rand() % 80 + 1;

        int height = 0;
        // find height
        for (int h = 256; h >= 128; h--) {
            if (getBlockAt(x, h, z) != EMPTY) {
                height = h + 1;
                break;
            }
        }

         // only draw on grasslands
         if (getBlockAt(x, height - 1, z) == GRASS) {
               drawTree(x, z, height);
         }
    }

    // render them trees (and the river)
    for (int x = -64; x <= 128; x += 16) {
        for (int z = -64; z <= 128; z += 16) {
            const uPtr<Chunk> &chunk = getChunkAt(x, z);
            chunk->destroy();
            chunk->generateVBOData();
            chunk->create();
        }
    }
}

void Terrain::drawTree(int x, int z, int height) {
    // tree tronk
    for (int y = height; y < height + 2; y++) {
        setBlockAt(x, y, z, WOOD);
    }
    // center ring
    for (int y = height + 2; y < height + 7; y++) {
        for (int i = x - 1; i <= x + 1; i++) {
            for (int j = z - 1; j <= z + 1; j++) {
                setBlockAt(i, y, j, LEAF);
            }
        }
    }
    // outer ring
    for (int y = height + 3; y < height + 6; y++) {
        for (int i = x - 2; i <= x + 2; i++) {
            for (int j = z - 2; j <= z + 2; j++) {
                if (getBlockAt(i, y, j) == EMPTY) {
                    setBlockAt(i, y, j, LEAF);
                }
            }
        }
    }
}

