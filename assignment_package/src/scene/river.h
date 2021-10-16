#ifndef RIVER_H
#define RIVER_H

#pragma once

#include <QString>
#include <QChar>
#include <QHash>
#include <QStack>
#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include "turtle.h"

using namespace std;
using namespace glm;

class River
{
public:
    typedef void (River::*Rule)(void);

    // member vars
    QString m_path;
    Turtle m_turtle;
    QStack<Turtle> m_turtleStack;
    QHash<QChar, QString> m_branchRules;
    QHash<QChar, Rule> m_charToDrawingOperation;

    // constructor
    River(vec2 position, vec2 orientation, float distance,
          QString axiom, int iterations, float branchProbability);

    // functions
    void setUpRules();
    void savePos();
    void loadPos();
    void moveForwardCurveRight();
    void moveForwardCurveLeft();
    void rotateRight();
    void rotateLeft();
    void X();
};

#endif // RIVER_H
