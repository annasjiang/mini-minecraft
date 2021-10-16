#include "chunk.h"
#include <iostream>

Chunk::Chunk(OpenGLContext *context) :
    Drawable(context), m_blocks(),
    m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {

    if (x >= 16 || x < 0) {
        return EMPTY;
    }

    if (y >= 256 || y < 0) {
        return EMPTY;
    }

    if (z >= 16 || z < 0) {
        return EMPTY;
    }

    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}

const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

void Chunk::setWorldPos(int x, int z) {
    int x_floor = static_cast<int>(glm::floor(x / 16.f));
    int z_floor = static_cast<int>(glm::floor(z / 16.f));
    worldPos_x = 16 * x_floor;
    worldPos_z = 16 * z_floor;
}

glm::ivec2 Chunk::getWorldPos() {
    return glm::ivec2(worldPos_x, worldPos_z);
}

bool isTrans(BlockType t) {
    return t == WATER || t == LAVA;
}

Chunk::~Chunk() {}

void Chunk::create() {
    loadVBO();
}

void Chunk::generateVBOData() {
    chunkVBOData.chunk = this;

    // for opaque blocks
    std::vector<GLuint> idx_opq;
    std::vector<glm::vec4> interleave_opq;

    // for transparent blocks
    std::vector<GLuint> idx_trans;
    std::vector<glm::vec4> interleave_trans;

    int faces_trans = 0;
    int vertices_trans = 0;

    int faces_opq = 0;
    int vertices_opq = 0;

    // iterates over all 3 coords of chunks
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 256; ++y) {
            for (int z = 0; z < 16; ++z) {

                BlockType t = getBlockAt(x, y, z);
                glm::vec4 block(x, y, z, 0);

                if (t != EMPTY) {

                    BlockType x_pos = getBlockAt(x + 1, y, z);
                    BlockType x_neg = getBlockAt(x - 1, y, z);
                    BlockType y_pos = getBlockAt(x, y + 1, z);
                    BlockType y_neg = getBlockAt(x, y - 1, z);
                    BlockType z_pos = getBlockAt(x, y, z + 1);
                    BlockType z_neg = getBlockAt(x, y, z - 1);

                    if (x == 0 && m_neighbors[XNEG]) {
                        x_neg = m_neighbors[XNEG]->getBlockAt(15, y, z);
                    }

                    if (x == 15 && m_neighbors[XPOS]) {
                        x_pos = m_neighbors[XPOS]->getBlockAt((x + 1) % 16, y, z);
                    }

                    if (y == 0) {
                        y_neg = EMPTY;
                    }

                    if (y == 255) {
                        y_pos = EMPTY;
                    }

                    if (z == 0 && m_neighbors[ZNEG]) {
                        z_neg = m_neighbors[ZNEG]->getBlockAt(x, y, 15);
                    }

                    if (z == 15 && m_neighbors[ZPOS]) {
                        z_pos = m_neighbors[ZPOS]->getBlockAt(x, y, 0);
                    }

                    if (x_pos == EMPTY || (isTrans(x_pos) && x_pos != t)) {
                        if (!isTrans(t)) {
                            updateVBO(interleave_opq,  XPOS, block, t, faces_opq++);
                        } else {
                            updateVBO(interleave_trans, XPOS, block, t, faces_trans++);
                        }
                    }

                    if (x_neg == EMPTY || (isTrans(x_neg) && x_neg != t)) {
                        if (!isTrans(t)) {
                            updateVBO(interleave_opq, XNEG, block, t, faces_opq++);
                        } else {
                            updateVBO(interleave_trans, XNEG, block, t, faces_trans++);
                        }
                    }

                    if (y_pos == EMPTY || (isTrans(y_pos) && y_pos != t)) {
                        if (!isTrans(t)) {
                            updateVBO(interleave_opq, YPOS, block, t, faces_opq++);
                        } else {
                            updateVBO(interleave_trans, YPOS, block, t, faces_trans++);
                        }
                    }

                    if (y_neg == EMPTY || (isTrans(y_neg) && y_neg != t)) {
                        if (!isTrans(t)) {
                            updateVBO(interleave_opq, YNEG, block, t, faces_opq++);
                        } else {
                            updateVBO(interleave_trans, YNEG, block, t, faces_trans++);
                        }
                    }

                    if (z_pos == EMPTY || (isTrans(z_pos) && z_pos != t)) {
                        if (!isTrans(t)) {
                            updateVBO(interleave_opq, ZPOS, block, t, faces_opq++);
                        } else {
                            updateVBO(interleave_trans, ZPOS, block, t, faces_trans++);
                        }
                    }

                    if (z_neg == EMPTY || (isTrans(z_neg) && z_neg != t)) {
                        if (!isTrans(t)) {
                            updateVBO(interleave_opq, ZNEG, block, t, faces_opq++);
                        } else {
                            updateVBO(interleave_trans, ZNEG, block, t, faces_trans++);
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < faces_opq; i++) {
        idx_opq.push_back(vertices_opq);
        idx_opq.push_back(vertices_opq + 1);
        idx_opq.push_back(vertices_opq + 2);
        idx_opq.push_back(vertices_opq);
        idx_opq.push_back(vertices_opq + 2);
        idx_opq.push_back(vertices_opq + 3);
        vertices_opq += 4;
    }

    for (int i = 0; i < faces_trans; i++) {
        idx_trans.push_back(vertices_trans);
        idx_trans.push_back(vertices_trans + 1);
        idx_trans.push_back(vertices_trans + 2);
        idx_trans.push_back(vertices_trans);
        idx_trans.push_back(vertices_trans + 2);
        idx_trans.push_back(vertices_trans + 3);
        vertices_trans += 4;
    }

    chunkVBOData.m_vboDataOpaque = interleave_opq;
    chunkVBOData.m_idxDataOpaque = idx_opq;

    chunkVBOData.m_vboDataTrans = interleave_trans;
    chunkVBOData.m_idxDataTrans = idx_trans;
}

void Chunk::updateVBO(std::vector<glm::vec4> &interleave,
                      Direction dir, glm::vec4 pos,
                      BlockType blockType, int faces) {

    BlockNeighbor neighbor = neighbors.at(dir);
    glm::vec4 neighbor_uv = blockFaceUVs.at(blockType).at(dir);

    for (int i = 0; i < 4; i++) {
        interleave.push_back(neighbor.vertices[i].pos + pos);
        interleave.push_back(glm::vec4(neighbor.offset, 1));

        if (blockType == LAVA || blockType == WATER) {
            neighbor_uv.z = 1;
        }
        interleave.push_back(neighbor_uv + neighbor.vertices[i].uv);
    }
}

// takes in a vector of interleaved vertex data and a vector of index data,
// and buffers them into the appropriate VBOs of your Drawable
void Chunk::loadVBO() {
    std::vector<glm::vec4> interleave_opq = chunkVBOData.m_vboDataOpaque;
    std::vector<glm::vec4> interleave_trans = chunkVBOData.m_vboDataTrans;
    std::vector<GLuint> idx_opq = chunkVBOData.m_idxDataOpaque;
    std::vector<GLuint> idx_trans = chunkVBOData.m_idxDataTrans;

    m_countOpq = idx_opq.size();
    m_countTrans = idx_trans.size();

    generateOpq();
    bindOpq();
    mp_context->glBufferData(GL_ARRAY_BUFFER, interleave_opq.size() * sizeof(glm::vec4), interleave_opq.data(), GL_STATIC_DRAW);

    generateIdxOpq();
    bindIdxOpq();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_opq.size() * sizeof(GLuint), idx_opq.data(), GL_STATIC_DRAW);

    generateTrans();
    bindTrans();
    mp_context->glBufferData(GL_ARRAY_BUFFER, interleave_trans.size() * sizeof(glm::vec4), interleave_trans.data(), GL_STATIC_DRAW);

    generateIdxTrans();
    bindIdxTrans();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx_trans.size() * sizeof(GLuint), idx_trans.data(), GL_STATIC_DRAW);
}


GLenum Chunk::drawMode() {
    return GL_TRIANGLES;
}
