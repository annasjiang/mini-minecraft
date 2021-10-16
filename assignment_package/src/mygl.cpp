#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <qdatetime.h>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progSky(this), m_progOverlay(this), m_quad(this),
      m_frameBuffer(this, this->width(), this->height(), this->devicePixelRatio()),
      m_terrain(this), m_player(glm::vec3(48.f, 150.f, 48.f), m_terrain),
      m_currMSecSinceEpoch(QDateTime::currentMSecsSinceEpoch()),
      m_texture(this), m_time(0.f),
      openInventory(false), numGrass(10), numDirt(10), numStone(10),
      numSand(10), numWood(10), numLeaf(10), numSnow(10), numIce(10),
      currBlockType(GRASS)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);

    m_quad.destroy();
    m_worldAxes.destroy();
    m_frameBuffer.destroy();
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL() {
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes + quad
    m_quad.create();
    m_worldAxes.create();

    // Initiailize frame buffers
    m_frameBuffer.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    // Create and set up the sky shader
    m_progSky.create(":/glsl/sky.vert.glsl", ":/glsl/sky.frag.glsl");
    m_quad.create();
    // overlay shader
    m_progOverlay.create(":/glsl/overlay.vert.glsl", ":/glsl/overlay.frag.glsl");

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_texture.create(":/minecraft_textures_all/minecraft_textures_all.png");
    m_texture.load(0);
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)
    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progSky.setViewProjMatrix(glm::inverse(viewproj));

    m_progSky.useMe();
    this->glUniform2i(m_progSky.unifDimensions, width(), height());
    this->glUniform3f(m_progSky.unifEye, m_player.mcr_camera.mcr_position.x,
                                         m_player.mcr_camera.mcr_position.y,
                                         m_player.mcr_camera.mcr_position.z);

    m_frameBuffer.resize(this->width(), this->height(), this->devicePixelRatio());
    m_frameBuffer.destroy();
    m_frameBuffer.create();

    printGLErrorLog();
}

// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    glm::vec3 prevPlayerPos = m_player.mcr_position;

    float dT = (QDateTime::currentMSecsSinceEpoch() - m_currMSecSinceEpoch) / 1000.f;
    m_player.tick(dT, m_inputs);
    m_currMSecSinceEpoch = QDateTime::currentMSecsSinceEpoch();

    m_terrain.multithreadedWork(m_player.mcr_position, prevPlayerPos);

    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
    sendInventoryDataToGUI(); // Update inventory quanities
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

void MyGL::sendInventoryDataToGUI() const {
    emit sig_sendNumGrass(numGrass);
    emit sig_sendNumDirt(numDirt);
    emit sig_sendNumStone(numStone);
    emit sig_sendNumSand(numSand);
    emit sig_sendNumWood(numWood);
    emit sig_sendNumLeaf(numLeaf);
    emit sig_sendNumIce(numIce);
    emit sig_sendNumSnow(numSnow);
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind frame buffer for overlay
//    m_frameBuffer.bindFrameBuffer();
//    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    // SKY CODE
    m_progSky.setViewProjMatrix(glm::inverse(m_player.mcr_camera.getViewProj()));
    m_progSky.useMe();
    this->glUniform3f(m_progSky.unifEye, m_player.mcr_camera.mcr_position.x,
                                         m_player.mcr_camera.mcr_position.y,
                                         m_player.mcr_camera.mcr_position.z);
    this->glUniform1f(m_progSky.unifTime, m_time++);
    m_progSky.draw(m_quad);

    m_progLambert.setTime(m_time++);
    m_progLambert.setPlayerPosition(glm::vec4(m_player.mcr_position.x,
                                              m_player.mcr_position.y,
                                              m_player.mcr_position.z, 0));

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    // m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);

//    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
//    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw the river and trees (wait for chunk to be drawn)
    if (m_time == 500) {
        cout << "draw river + trees" << endl;
        m_terrain.drawRiver();
        m_terrain.drawTrees();
    }

    // determine overlay after the river is drawn
    if (m_time > 500) {
        if (m_player.isUnderWater(m_terrain, m_inputs)) {
            // cout << "underwater overlay" << endl;
            m_progOverlay.overlayType(1);
            m_frameBuffer.bindToTextureSlot(2);
            m_progOverlay.drawOverlay(m_quad);
        } else if (m_player.isUnderLava(m_terrain, m_inputs)) {
            // cout << "underlava overlay" << endl;
            m_progOverlay.overlayType(2);
            m_frameBuffer.bindToTextureSlot(2);
            m_progOverlay.drawOverlay(m_quad);
        } else {
            m_progOverlay.overlayType(0);
        }
    }

    renderTerrain();
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    m_texture.bind(0);

    int xFloor = static_cast<int>(glm::floor(m_player.mcr_position.x / 16.f));
    int zFloor = static_cast<int>(glm::floor(m_player.mcr_position.z / 16.f));
    int x = 16 * xFloor;
    int z = 16 * zFloor;

    m_terrain.draw(x - 256, x + 256, z - 256, z + 256, &m_progLambert);
}

void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    }

    if (!e->isAutoRepeat()) {
        if (e->key() == Qt::Key_W) {
            m_inputs.wPressed = true;
        } else if (e->key() == Qt::Key_S) {
            m_inputs.sPressed = true;
        } else if (e->key() == Qt::Key_D) {
            m_inputs.dPressed = true;
        } else if (e->key() == Qt::Key_A) {
            m_inputs.aPressed = true;
        } else if (e->key() == Qt::Key_F) {
            m_inputs.flightMode = !m_inputs.flightMode;
            if (m_inputs.flightMode) {
                std::cout << "flight mode on" << std::endl;
            } else {
                std::cout << "flight mode off" << std::endl;
            }
        } else if (e->key() == Qt::Key_I) {
            openInventory = !openInventory;
            if (openInventory) {
                std::cout << "show inventory" << std::endl;
            } else {
                std::cout << "hide inventory" << std::endl;
            }
            emit sig_inventoryOpenClose(openInventory);
        }

        if (m_inputs.flightMode) {
            if (e->key() == Qt::Key_Q) {
                m_inputs.qPressed = true;
            } else if (e->key() == Qt::Key_E) {
                m_inputs.ePressed = true;
            }
        } else {
            if (e->key() == Qt::Key_Space) {
                m_inputs.spacePressed = true;
            }
        }

        // sounds
        if (m_inputs.flightMode) {
            m_player.swimming->stop();
            m_player.walking->stop();
            m_player.flying->play();
        } else {
            m_player.flying->stop();
            if (m_inputs.underWater) {
                m_player.swimming->play();
            } else if (m_inputs.onGround) {
                m_player.walking->play();
            } else {
                m_player.walking->stop();
                m_player.swimming->stop();
            }
        }
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e) {
    if (!e->isAutoRepeat()) {
        if (e->key() == Qt::Key_W) {
            m_inputs.wPressed = false;
        } else if (e->key() == Qt::Key_S) {
            m_inputs.sPressed = false;
        } else if (e->key() == Qt::Key_D) {
            m_inputs.dPressed = false;
        } else if (e->key() == Qt::Key_A) {
            m_inputs.aPressed = false;
        } else if (e->key() == Qt::Key_Q) {
            m_inputs.qPressed = false;
        } else if (e->key() == Qt::Key_E) {
            m_inputs.ePressed = false;
        } else if (e->key() == Qt::Key_Space) {
            m_inputs.spacePressed = false;
        }
    }
}

// rotate camera with mouse
void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // for mac
    float dx = (this->width() * 0.5 - e->pos().x()) / width();
    float dy = (this->height() * 0.5 - e->pos().y()) / height();
    m_player.rotateOnUpGlobal(dx * 360 * 0.005f);
    m_player.rotateOnRightLocal(dy * 360 * 0.005f);
    moveMouseToCenter();

 //     for windows
//    const float SENSITIVITY = 50.0;

//    float dx = width() * 0.5 - e->pos().x();
//    if (dx != 0) {
//        m_player.rotateOnUpGlobal(dx/width() * SENSITIVITY);
//    }

//    float dy = height() * 0.5 - e->pos().y() - 0.5;
//    if (dy != 0) {
//        m_player.rotateOnRightLocal(dy/height() * SENSITIVITY);
//    }

//     moveMouseToCenter();
}

// add and remove chunks
void MyGL::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        BlockType removed = m_player.removeBlock(&m_terrain);
        if (removed == GRASS) {
            numGrass++;
        } else if (removed == DIRT) {
            numDirt++;
        } else if (removed == STONE) {
            numStone++;
        } else if (removed == SAND) {
            numSand++;
        } else if (removed == WOOD) {
            numWood++;
        } else if (removed == LEAF) {
            numLeaf++;
        } else if (removed == SNOW) {
            numSnow++;
        } else if (removed == ICE) {
            numIce++;
        }
    } else if (e->button() == Qt::RightButton) {
        if (currBlockType == GRASS && numGrass > 0 && m_player.placeBlock(&m_terrain, GRASS) != EMPTY) {
            numGrass--;
        } else if (currBlockType == DIRT && numDirt > 0 && m_player.placeBlock(&m_terrain, DIRT) != EMPTY) {
            numDirt--;
        } else if (currBlockType == STONE && numStone > 0 && m_player.placeBlock(&m_terrain, STONE) != EMPTY) {
            numStone--;
        } else if (currBlockType == SAND && numSand > 0 && m_player.placeBlock(&m_terrain, SAND) != EMPTY) {
            numSand--;
        } else if (currBlockType == WOOD && numWood > 0 && m_player.placeBlock(&m_terrain, WOOD) != EMPTY) {
            numWood--;
        } else if (currBlockType == LEAF && numLeaf > 0 && m_player.placeBlock(&m_terrain, LEAF) != EMPTY) {
            numLeaf--;
        } else if (currBlockType == SNOW && numSnow > 0 && m_player.placeBlock(&m_terrain, SNOW) != EMPTY) {
            numSnow--;
        } else if (currBlockType == ICE && numIce > 0 && m_player.placeBlock(&m_terrain, ICE) != EMPTY) {
            numIce--;
        }
    }
}
