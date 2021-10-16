#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include "drawable.h"
#include <iostream>


//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, SNOW, STONE, LAVA, WATER, ICE, SAND, WOOD, LEAF
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

typedef struct Vertex {
    glm::vec4 pos;

    // relative UV coords that are offset based on BlockType
    glm::vec4 uv;

    Vertex(glm::vec4 p, glm::vec4 u)
        : pos(p), uv(u) {}
} Vertex;

typedef struct BlockNeighbor {
    Direction dir;
    glm::vec3 offset;
    std::array<Vertex, 4> vertices;    // vertex set to make a quadrangle facing in this direction
    BlockNeighbor(Direction direction, glm::vec3 off,
                  Vertex a,  Vertex b, Vertex c,  Vertex d)
        : dir(direction), offset(off), vertices{a, b, c, d}
    {}
} BlockNeighbor;

const static std::array<BlockNeighbor, 6> neighbors = {

    BlockNeighbor(XPOS, glm::vec3(1, 0, 0), Vertex(glm::vec4(1, 0, 1, 1), glm::vec4(0, 0, 0, 0)),
                                            Vertex(glm::vec4(1, 0, 0, 1), glm::vec4(0.0625, 0, 0, 0)),
                                            Vertex(glm::vec4(1, 1, 0, 1), glm::vec4(0.0625, 0.0625, 0, 0)),
                                            Vertex(glm::vec4(1, 1, 1, 1), glm::vec4(0, 0.0625, 0, 0))),

    BlockNeighbor(XNEG, glm::vec3(-1, 0, 0), Vertex(glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 0)),
                                             Vertex(glm::vec4(0, 0, 1, 1), glm::vec4(0.0625, 0, 0, 0)),
                                             Vertex(glm::vec4(0, 1, 1, 1), glm::vec4(0.0625, 0.0625, 0, 0)),
                                             Vertex(glm::vec4(0, 1, 0, 1), glm::vec4(0, 0.0625, 0, 0))),

    BlockNeighbor(YPOS, glm::vec3(0, 1, 0), Vertex(glm::vec4(0, 1, 1, 1), glm::vec4(0, 0, 0, 0)),
                                            Vertex(glm::vec4(1, 1, 1, 1), glm::vec4(0.0625, 0, 0, 0)),
                                            Vertex(glm::vec4(1, 1, 0, 1), glm::vec4(0.0625, 0.0625, 0, 0)),
                                            Vertex(glm::vec4(0, 1, 0, 1), glm::vec4(0, 0.0625, 0, 0))),

    BlockNeighbor(YNEG, glm::vec3(0, -1, 0), Vertex(glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 0, 0)),
                                             Vertex(glm::vec4(1, 0, 0, 1), glm::vec4(0.0625, 0, 0, 0)),
                                             Vertex(glm::vec4(1, 0, 1, 1), glm::vec4(0.0625, 0.0625, 0, 0)),
                                             Vertex(glm::vec4(0, 0, 1, 1), glm::vec4(0, 0.0625, 0, 0))),

    BlockNeighbor(ZPOS, glm::vec3(0, 0, 1), Vertex(glm::vec4(0, 0, 1, 1), glm::vec4(0, 0, 0, 0)),
                                            Vertex(glm::vec4(1, 0, 1, 1), glm::vec4(0.0625, 0, 0, 0)),
                                            Vertex(glm::vec4(1, 1, 1, 1), glm::vec4(0.0625, 0.0625, 0, 0)),
                                            Vertex(glm::vec4(0, 1, 1, 1), glm::vec4(0, 0.0625, 0, 0))),

    BlockNeighbor(ZNEG, glm::vec3(0, 0, -1), Vertex(glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, 0, 0)),
                                             Vertex(glm::vec4(0, 0, 0, 1), glm::vec4(0.0625, 0, 0, 0)),
                                             Vertex(glm::vec4(0, 1, 0, 1), glm::vec4(0.0625, 0.0625, 0, 0)),
                                             Vertex(glm::vec4(1, 1, 0, 1), glm::vec4(0, 0.0625, 0, 0)))
};

// match BlockType to specific collection of UV coords
const static std::unordered_map<BlockType, std::unordered_map<Direction, glm::vec4, EnumHash>, EnumHash> blockFaceUVs {
    {GRASS, std::unordered_map<Direction, glm::vec4, EnumHash>{{XPOS, glm::vec4(3.f/16.f, 15.f/16.f, 0, 0)},
                                                               {XNEG, glm::vec4(3.f/16.f, 15.f/16.f, 0, 0)},
                                                               {YPOS, glm::vec4(8.f/16.f, 13.f/16.f, 0, 0)},
                                                               {YNEG, glm::vec4(2.f/16.f, 15.f/16.f, 0, 0)},
                                                               {ZPOS, glm::vec4(3.f/16.f, 15.f/16.f, 0, 0)},
                                                               {ZNEG, glm::vec4(3.f/16.f, 15.f/16.f, 0, 0)}}},

    {DIRT, std::unordered_map<Direction, glm::vec4, EnumHash>{{XPOS, glm::vec4(2.f/16.f, 15.f/16.f, 0, 0)},
                                                              {XNEG, glm::vec4(2.f/16.f, 15.f/16.f, 0, 0)},
                                                              {YPOS, glm::vec4(2.f/16.f, 15.f/16.f, 0, 0)},
                                                              {YNEG, glm::vec4(2.f/16.f, 15.f/16.f, 0, 0)},
                                                              {ZPOS, glm::vec4(2.f/16.f, 15.f/16.f, 0, 0)},
                                                              {ZNEG, glm::vec4(2.f/16.f, 15.f/16.f, 0, 0)}}},

    {STONE, std::unordered_map<Direction, glm::vec4, EnumHash>{{XPOS, glm::vec4(1.f/16.f, 15.f/16.f, 0, 0)},
                                                               {XNEG, glm::vec4(1.f/16.f, 15.f/16.f, 0, 0)},
                                                               {YPOS, glm::vec4(1.f/16.f, 15.f/16.f, 0, 0)},
                                                               {YNEG, glm::vec4(1.f/16.f, 15.f/16.f, 0, 0)},
                                                               {ZPOS, glm::vec4(1.f/16.f, 15.f/16.f, 0, 0)},
                                                               {ZNEG, glm::vec4(1.f/16.f, 15.f/16.f, 0, 0)}}},

    {LAVA, std::unordered_map<Direction, glm::vec4, EnumHash>{{XPOS, glm::vec4(15.f/16.f, 1.f/16.f, 0, 0)},
                                                              {XNEG, glm::vec4(15.f/16.f, 1.f/16.f, 0, 0)},
                                                              {YPOS, glm::vec4(15.f/16.f, 1.f/16.f, 0, 0)},
                                                              {YNEG, glm::vec4(15.f/16.f, 1.f/16.f, 0, 0)},
                                                              {ZPOS, glm::vec4(15.f/16.f, 1.f/16.f, 0, 0)},
                                                              {ZNEG, glm::vec4(15.f/16.f, 1.f/16.f, 0, 0)}}},

    {WATER, std::unordered_map<Direction, glm::vec4, EnumHash>{{XPOS, glm::vec4(15.f/16.f, 3.f/16.f, 0, 0)},
                                                              {XNEG, glm::vec4(15.f/16.f, 3.f/16.f, 0, 0)},
                                                              {YPOS, glm::vec4(15.f/16.f, 3.f/16.f, 0, 0)},
                                                              {YNEG, glm::vec4(15.f/16.f, 3.f/16.f, 0, 0)},
                                                              {ZPOS, glm::vec4(15.f/16.f, 3.f/16.f, 0, 0)},
                                                              {ZNEG, glm::vec4(15.f/16.f, 3.f/16.f, 0, 0)}}},

    {SNOW, std::unordered_map<Direction, glm::vec4, EnumHash>{{XPOS, glm::vec4(2.f/16.f, 11.f/16.f, 0, 0)},
                                                              {XNEG, glm::vec4(2.f/16.f, 11.f/16.f, 0, 0)},
                                                              {YPOS, glm::vec4(2.f/16.f, 11.f/16.f, 0, 0)},
                                                              {YNEG, glm::vec4(2.f/16.f, 11.f/16.f, 0, 0)},
                                                              {ZPOS, glm::vec4(2.f/16.f, 11.f/16.f, 0, 0)},
                                                              {ZNEG, glm::vec4(2.f/16.f, 11.f/16.f, 0, 0)}}},

    {SAND, std::unordered_map<Direction, glm::vec4, EnumHash>{{XPOS, glm::vec4(2.f/16.f, 14.f/16.f, 0, 0)},
                                                              {XNEG, glm::vec4(2.f/16.f, 14.f/16.f, 0, 0)},
                                                              {YPOS, glm::vec4(2.f/16.f, 14.f/16.f, 0, 0)},
                                                              {YNEG, glm::vec4(2.f/16.f, 14.f/16.f, 0, 0)},
                                                              {ZPOS, glm::vec4(2.f/16.f, 14.f/16.f, 0, 0)},
                                                              {ZNEG, glm::vec4(2.f/16.f, 14.f/16.f, 0, 0)}}},

    {WOOD, std::unordered_map<Direction, glm::vec4, EnumHash>{{XPOS, glm::vec4(6.f/16.f, 10.f/16.f, 0, 0)},
                                                              {XNEG, glm::vec4(6.f/16.f, 10.f/16.f, 0, 0)},
                                                              {YPOS, glm::vec4(6.f/16.f, 10.f/16.f, 0, 0)},
                                                              {YNEG, glm::vec4(6.f/16.f, 10.f/16.f, 0, 0)},
                                                              {ZPOS, glm::vec4(6.f/16.f, 10.f/16.f, 0, 0)},
                                                              {ZNEG, glm::vec4(6.f/16.f, 10.f/16.f, 0, 0)}}},

    {LEAF, std::unordered_map<Direction, glm::vec4, EnumHash>{{XPOS, glm::vec4(4.f/16.f, 12.f/16.f, 0, 0)},
                                                              {XNEG, glm::vec4(4.f/16.f, 12.f/16.f, 0, 0)},
                                                              {YPOS, glm::vec4(4.f/16.f, 12.f/16.f, 0, 0)},
                                                              {YNEG, glm::vec4(4.f/16.f, 12.f/16.f, 0, 0)},
                                                              {ZPOS, glm::vec4(4.f/16.f, 12.f/16.f, 0, 0)},
                                                              {ZNEG, glm::vec4(4.f/16.f, 12.f/16.f, 0, 0)}}}

};

class Chunk;

struct ChunkVBOData {
    Chunk* chunk;
    std::vector<glm::vec4> m_vboDataOpaque, m_vboDataTrans;
    std::vector<GLuint> m_idxDataOpaque, m_idxDataTrans;
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

    int worldPos_x;
    int worldPos_z;

public:
    ChunkVBOData chunkVBOData;
    Chunk(OpenGLContext* context);

    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;

    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);

    void setWorldPos(int x, int z);
    glm::ivec2 getWorldPos();

    void loadVBO();
    void updateVBO(std::vector<glm::vec4> &interleave,
                   Direction dir, glm::vec4 pos,
                   BlockType blockType, int faces);

    void virtual create() override;
    void generateVBOData();

    GLenum drawMode() override;

    ~Chunk();
};
