// Port of upstream/gaussian.fs; see upstream/LICENSE.
struct Color {
  float r, g, b, a;
};
struct GaussianParams {
  int layer;
  int radius;
  int direction_x;
  int direction_y;
  int atlas_min;
  int atlas_max;
  float kernel[16];
};
static_assert(sizeof(GaussianParams) == 88);
static_assert(__builtin_offsetof(GaussianParams, kernel) == 24);

Color scaled(Color value, float weight) {
  return mul(value, Color{weight, weight, weight, weight});
}

__fragment
void filament_gaussian(__binding(0, 0) texture_2d_layers<Color> image,
                       __push_constant const GaussianParams *params,
                       __output(0) Color *color) {
  int center_x = int(frag_coord.x);
  int center_y = int(frag_coord.y);
  int x = clamp(center_x, params->atlas_min, params->atlas_max);
  int y = clamp(center_y, params->atlas_min, params->atlas_max);
  Color result = scaled(image.load(x, y, params->layer, 0u), params->kernel[0]);
  // A constant index keeps each weight inside its C-layout push-constant
  // record. The source loop permits any radius; this record stores 16 weights.
#define GAUSSIAN_TAP(I)                                                        \
  if (params->radius >= I) {                                                   \
    int offset_x = params->direction_x * I;                                    \
    int offset_y = params->direction_y * I;                                    \
    int positive_x =                                                           \
        clamp(center_x + offset_x, params->atlas_min, params->atlas_max);      \
    int positive_y =                                                           \
        clamp(center_y + offset_y, params->atlas_min, params->atlas_max);      \
    int negative_x =                                                           \
        clamp(center_x - offset_x, params->atlas_min, params->atlas_max);      \
    int negative_y =                                                           \
        clamp(center_y - offset_y, params->atlas_min, params->atlas_max);      \
    float weight = params->kernel[I];                                          \
    result = add(result,                                                       \
                 scaled(image.load(positive_x, positive_y, params->layer, 0u), \
                        weight));                                              \
    result = add(result,                                                       \
                 scaled(image.load(negative_x, negative_y, params->layer, 0u), \
                        weight));                                              \
  }
  GAUSSIAN_TAP(1)
  GAUSSIAN_TAP(2)
  GAUSSIAN_TAP(3)
  GAUSSIAN_TAP(4)
  GAUSSIAN_TAP(5)
  GAUSSIAN_TAP(6)
  GAUSSIAN_TAP(7)
  GAUSSIAN_TAP(8)
  GAUSSIAN_TAP(9)
  GAUSSIAN_TAP(10)
  GAUSSIAN_TAP(11)
  GAUSSIAN_TAP(12)
  GAUSSIAN_TAP(13)
  GAUSSIAN_TAP(14)
  GAUSSIAN_TAP(15)
#undef GAUSSIAN_TAP
  *color = result;
}
