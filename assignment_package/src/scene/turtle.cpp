#include "turtle.h"
#define GLM_FORCE_RADIANS

Turtle::Turtle()
    : m_position(vec2(0.f, 0.f)), m_orientation(vec2(0.f, 1.f)),
      m_distance(1.f), m_depth(1)
{}

Turtle::Turtle(vec2 position, vec2 orientation, float distance)
    : m_position(position), m_orientation(orientation), m_distance(distance), m_depth(1)
{}

Turtle::Turtle(const Turtle& t)
    : m_position(t.m_position), m_orientation(t.m_orientation),
      m_distance(t.m_distance), m_depth(t.m_depth)
{}

void Turtle::rotateRight(const float factor) {
    float angle = linearRand(0.f, 1.f) * PI / factor;
    m_orientation = vec2(cos(angle) * m_orientation.x - sin(angle) * m_orientation.y,
                         sin(angle) * m_orientation.x + cos(angle) * m_orientation.y);
}

void Turtle::rotateLeft(const float factor) {
    float angle = -linearRand(0.f, 1.f) * PI / factor;
    m_orientation = vec2(cos(angle) * m_orientation.x - sin(angle) * m_orientation.y,
                         sin(angle) * m_orientation.x + cos(angle) * m_orientation.y);
}

void Turtle::moveForward() {
    m_position += (m_orientation * m_distance);
}
