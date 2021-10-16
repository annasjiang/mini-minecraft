#pragma once
#include <openglcontext.h>
#include <glm_includes.h>

//This defines a class which can be rendered by our shader program.
//Make any geometry a subclass of ShaderProgram::Drawable in order to render it with the ShaderProgram class.
class Drawable
{
protected:
    int m_count;     // The number of indices stored in bufIdx.

    // milestone 2
    int m_countOpq;
    int m_countTrans;

    GLuint m_bufIdx; // A Vertex Buffer Object that we will use to store triangle indices (GLuints)
    GLuint m_bufPos; // A Vertex Buffer Object that we will use to store mesh vertices (vec4s)
    GLuint m_bufNor; // A Vertex Buffer Object that we will use to store mesh normals (vec4s)
    GLuint m_bufCol; // Can be used to pass per-vertex color information to the shader, but is currently unused.
                     // Instead, we use a uniform vec4 in the shader to set an overall color for the geometry

    GLuint m_bufUV;

    // milestone 2
    GLuint m_bufOpq;
    GLuint m_bufIdxOpq;
    GLuint m_bufTrans;
    GLuint m_bufIdxTrans;

    bool m_idxGenerated; // Set to TRUE by generateIdx(), returned by bindIdx().
    bool m_posGenerated;
    bool m_norGenerated;
    bool m_colGenerated;
    bool m_UVGenerated;

    // milestone 2
    bool m_opqGenerated;
    bool m_idxOpqGenerated;
    bool m_transGenerated;
    bool m_idxTransGenerated;

    OpenGLContext* mp_context; // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                          // we need to pass our OpenGL context to the Drawable in order to call GL functions
                          // from within this class.


public:
    Drawable(OpenGLContext* mp_context);
    virtual ~Drawable();

    virtual void create() = 0; // To be implemented by subclasses. Populates the VBOs of the Drawable.
    void destroy(); // Frees the VBOs of the Drawable.

    // Getter functions for various GL data
    virtual GLenum drawMode();
    int elemCount();

    // milestone 2
    int elemCountOpq();
    int elemCountTrans();

    // Call these functions when you want to call glGenBuffers on the buffers stored in the Drawable
    // These will properly set the values of idxBound etc. which need to be checked in ShaderProgram::draw()
    void generateIdx();
    void generatePos();
    void generateNor();
    void generateCol();
    void generateUV();

    // milestone 2
    void generateOpq();
    void generateIdxOpq();
    void generateTrans();
    void generateIdxTrans();    

    bool bindIdx();
    bool bindPos();
    bool bindNor();
    bool bindCol();
    bool bindUV();

    // milestone 2
    bool bindOpq();
    bool bindIdxOpq();
    bool bindTrans();
    bool bindIdxTrans();
};
