#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_count(-1), m_countOpq(-1), m_countTrans(-1),
      m_bufIdx(), m_bufPos(), m_bufNor(), m_bufCol(), m_bufUV(),
      m_bufOpq(), m_bufIdxOpq(), m_bufTrans(), m_bufIdxTrans(),
      m_idxGenerated(false), m_posGenerated(false), m_norGenerated(false), m_colGenerated(false), m_UVGenerated(false),
      m_opqGenerated(false), m_idxOpqGenerated(false), m_transGenerated(false), m_idxTransGenerated(false),
      mp_context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    mp_context->glDeleteBuffers(1, &m_bufIdx);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufCol);

    mp_context->glDeleteBuffers(1, &m_bufOpq);
    mp_context->glDeleteBuffers(1, &m_bufTrans);
    mp_context->glDeleteBuffers(1, &m_bufIdxOpq);
    mp_context->glDeleteBuffers(1, &m_bufIdxTrans);

    m_idxGenerated = m_posGenerated = m_norGenerated = m_colGenerated =
    m_opqGenerated = m_idxOpqGenerated = m_transGenerated = m_idxTransGenerated = false;

    m_count = -1;
    m_countOpq = -1;
    m_countTrans = -1;
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return m_count;
}

// added in milestone 2
int Drawable::elemCountOpq()
{
    return m_countOpq;
}

int Drawable::elemCountTrans()
{
    return m_countTrans;
}

void Drawable::generateIdx()
{
    m_idxGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdx);
}

// added in milestone 2
void Drawable::generateIdxOpq()
{
    m_idxOpqGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdxOpq);
}

void Drawable::generateIdxTrans()
{
    m_idxTransGenerated = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    mp_context->glGenBuffers(1, &m_bufIdxTrans);
}

void Drawable::generatePos()
{
    m_posGenerated = true;
    // Create a VBO on our GPU and store its handle in bufPos
    mp_context->glGenBuffers(1, &m_bufPos);
}

void Drawable::generateNor()
{
    m_norGenerated = true;
    // Create a VBO on our GPU and store its handle in bufNor
    mp_context->glGenBuffers(1, &m_bufNor);
}

void Drawable::generateCol()
{
    m_colGenerated = true;
    // Create a VBO on our GPU and store its handle in bufCol
    mp_context->glGenBuffers(1, &m_bufCol);
}

void Drawable::generateUV()
{
    m_UVGenerated = true;
    mp_context->glGenBuffers(1, &m_bufUV);
}

// added in milestone 2
void Drawable::generateOpq() {
    mp_context->glGenBuffers(1, &m_bufOpq);
    m_opqGenerated = true;
}

void Drawable::generateTrans() {
    mp_context->glGenBuffers(1, &m_bufTrans);
    m_transGenerated = true;
}

bool Drawable::bindIdx()
{
    if(m_idxGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdx);
    }
    return m_idxGenerated;
}

// added in milestone 2
bool Drawable::bindIdxOpq()
{
    if(m_idxOpqGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpq);
    }
    return m_idxOpqGenerated;
}

bool Drawable::bindIdxTrans()
{
    if(m_idxTransGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTrans);
    }
    return m_idxTransGenerated;
}

bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindUV()
{
    if(m_UVGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    }
    return m_UVGenerated;
}

// added in milestone 2
bool Drawable::bindOpq() {
    if (m_opqGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufOpq);
    }
    return m_opqGenerated;
}

bool Drawable::bindTrans() {
    if (m_transGenerated) {
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufTrans);
    }
    return m_transGenerated;
}
