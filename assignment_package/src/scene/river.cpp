#define GLM_FORCE_RADIANS
#include "river.h"

River::River(vec2 position, vec2 orientation, float distance,
             QString axiom, int iterations, float branchProbability)
    : m_turtle(Turtle(position, orientation, distance))
{
    setUpRules();
    // expand axiom for branching
    for (int i = 0; i < iterations; i++) {
        for (auto rule = m_branchRules.begin(); rule != m_branchRules.end(); rule++) {
            vector<int> indices;
            for (int k = 0; k < axiom.length(); k++) {
                if (axiom[k] == rule.key()) {
                    indices.push_back(k);
                }
            }
            int offset = 0;
            for (unsigned int k = 0; k < indices.size(); k++) {
                // random branching + expansion
                if (linearRand(0.f, 1.f) < branchProbability) {
                    axiom.replace(indices[k] + offset, 1, rule.value());
                    offset += rule.value().length() - 1;
                } else {
                    axiom.replace(indices[k] + offset, 1, "-FFFFF");
                    offset += 5;
                }
            }
        }
    }
    m_path = axiom;
}

// helper
void River::setUpRules() {
    m_branchRules['X'] = "[+FFGGX]-FFGGX";

    m_charToDrawingOperation['F'] = &River::moveForwardCurveLeft;
    m_charToDrawingOperation['G'] = &River::moveForwardCurveRight;
    m_charToDrawingOperation['-'] = &River::rotateRight;
    m_charToDrawingOperation['+'] = &River::rotateLeft;
    m_charToDrawingOperation['['] = &River::savePos;
    m_charToDrawingOperation[']'] = &River::loadPos;
    m_charToDrawingOperation['X'] = &River::X;
}

// turtle functions
void River::savePos() {
    m_turtleStack.push(Turtle(m_turtle));
    m_turtle.m_depth++;
}

void River::loadPos() {
    m_turtle = m_turtleStack.pop();
}

void River::moveForwardCurveRight() {
    m_turtle.rotateRight(10.f);
    m_turtle.moveForward();
}

void River::moveForwardCurveLeft() {
    m_turtle.rotateLeft(10.f);
    m_turtle.moveForward();
}

void River::rotateRight() {
    m_turtle.rotateRight(5.f);
}

void River::rotateLeft() {
    m_turtle.rotateLeft(5.f);
}

void River::X() {}
