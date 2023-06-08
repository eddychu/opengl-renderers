#pragma once

#include <resource.h>

Geometry make_quad() {
  Geometry geometry;
  geometry.vertices = {
      {{-1.0f, -1.0f, 0.0f},
       {0.0f, 0.0f, 1.0f},
       {0.0f, 0.0f},
       {0.0f, 0.0f, 0.0f}},
      {{1.0f, -1.0f, 0.0f},
       {0.0f, 0.0f, 1.0f},
       {0.0f, 0.0f},
       {0.0f, 0.0f, 0.0f}},
      {{1.0f, 1.0f, 0.0f},
       {0.0f, 0.0f, 1.0f},
       {0.0f, 0.0f},
       {0.0f, 0.0f, 0.0f}},
      {{-1.0f, 1.0f, 0.0f},
       {0.0f, 0.0f, 1.0f},
       {0.0f, 0.0f},
       {0.0f, 0.0f, 0.0f}},
  };
  geometry.indices = {0, 1, 2, 2, 3, 0};
  geometry.calcTangents();
  return geometry;
}