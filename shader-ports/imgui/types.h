// Dear ImGui-derived shader ports; see upstream/LICENSE.txt.
#pragma once

struct Float2 {
  float x, y;
};
struct Color {
  float r, g, b, a;
};
struct Varyings {
  Color color;
  Float2 uv;
};
struct alignas(8) Params {
  alignas(8) float scale[2];
  alignas(8) float translate[2];
};
static_assert(sizeof(Params) == 16);
static_assert(__builtin_offsetof(Params, scale) == 0);
static_assert(__builtin_offsetof(Params, translate) == 8);
