/*
  ==============================================================================

    RenderObject3D.h
    Created: 23 Jan 2026 12:24:10pm
    Author:  lucas

  ==============================================================================
*/

#pragma once

#include <vector>

class RenderObject3D {

public:
    RenderObject3D(int id, std::vector<float> v, std::vector<float> n, std::vector<float> u, std::vector<int> i) : vertices(v), normals(n), uvs(u), indices(i) {

    }

    void init() {

    }

private:
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> uvs;
    std::vector<int> indices;

};
