// Port of https://www.shadertoy.com/view/NsScWt; author: nyri0.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NsScWt.json.
#include "../../support.h"
using namespace glsl;
struct Texture2D {
  texture_2d<f32x4> image;
  sampler sampling;
};
f32x4 texture(Texture2D t, f32x2 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1]);
}
f32x4 textureLod(Texture2D t, f32x2 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], lod);
}
i32x2 textureSize(Texture2D t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x2(size[0], size[1]);
}
f32x4 texelFetch(Texture2D t, i32x2 uv) {
  return t.image.load(uv.data[0], uv.data[1]);
}
struct Texture3D {
  texture_3d<f32x4> image;
  sampler sampling;
};
f32x4 texture(Texture3D t, f32x3 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1], uv.data[2]);
}
f32x4 textureLod(Texture3D t, f32x3 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], uv.data[2],
                            lod);
}
i32x3 textureSize(Texture3D t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x3(size[0], size[1], size[2]);
}
f32x4 texelFetch(Texture3D t, i32x3 uv) {
  return t.image.load(uv.data[0], uv.data[1], uv.data[2]);
}
struct TextureCube {
  texture_cube<f32x4> image;
  sampler sampling;
};
f32x4 texture(TextureCube t, f32x3 uv) {
  return t.image.sample(t.sampling, uv.data[0], uv.data[1], uv.data[2]);
}
f32x4 textureLod(TextureCube t, f32x3 uv, float lod) {
  return t.image.sample_lod(t.sampling, uv.data[0], uv.data[1], uv.data[2],
                            lod);
}
i32x2 textureSize(TextureCube t, int lod) {
  auto size = t.image.dimensions(static_cast<unsigned>(lod));
  return i32x2(size[0], size[1]);
}
using sampler2D = Texture2D;
using sampler3D = Texture3D;
using samplerCube = TextureCube;
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

const f32x3 BG = f32x3(0, 0, 0);
const float EPS = 0.001f;
const float FOVH = 70.0f;
const float D = 0.1f;
const float MAX_DIST = 30.0f;
const int MAX_ITER = 400;
const float AMBIENT = 0.1f;
const int FSAA = 2;
const float GRID_DIM = 1.0f;
const bool JUMP = false;
float jTime;
float sq(Context &__ctx, float x) { return x * x; }
f32x2 hashv(Context &__ctx, f32x2 p) {
  p = f32x2(dot(p, f32x2(127.1f, 311.7f)), dot(p, f32x2(269.5f, 183.3f)));
  return -1.0f + 2.0f * fract(sin(p) * 43758.5453123f);
}
float noise(Context &__ctx, f32x2 p) {
  const float K1 = 0.366025404f;
  const float K2 = 0.211324865f;
  f32x2 i = floor(p + (p.data[0] + p.data[1]) * K1);
  f32x2 a = p - i + (i.data[0] + i.data[1]) * K2;
  float m = step(a.data[1], a.data[0]);
  f32x2 o = f32x2(m, 1.0f - m);
  f32x2 b = a - o + K2;
  f32x2 c = a - 1.0f + 2.0f * K2;
  f32x3 h = max(0.5f - f32x3(dot(a, a), dot(b, b), dot(c, c)), 0.0f);
  f32x3 n = h * h * h * h *
            f32x3(dot(a, hashv(__ctx, i + 0.0f)), dot(b, hashv(__ctx, i + o)),
                  dot(c, hashv(__ctx, i + 1.0f)));
  return dot(n, f32x3(70.0f));
}
unsigned hash(Context &__ctx, unsigned x) {
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}
float hash(Context &__ctx, f32x2 vf) {
  unsigned offset = vf.data[0] < 0.0f ? 13u : 0u;
  u32x2 vi = u32x2(abs(vf));
  return float(hash(__ctx, vi.data[0] + (vi.data[1] << 16) + offset)) /
         float(0xffffffffU);
}
float vertex_height(Context &__ctx, f32x2 vf) {
  float offset = -2.0f + 2.0f * smoothstep(5.f, 8.f, abs(vf.data[0]));
  float amplitude = mix(0.5f, 2.f, smoothstep(7.f, 12.f, abs(vf.data[0])));
  return amplitude * hash(__ctx, vf) + offset +
         (JUMP ? 0.2f * abs(sin(4.f * iTime)) : 0.f);
}
float height_and_color_map(Context &__ctx, f32x2 proj) {
  proj /= GRID_DIM;
  proj.data[0] -= 0.5f;
  f32x2 d = fract(proj);
  proj -= d;
  bool gd = dot(d, f32x2(1)) < 1.f;
  f32x2 pt = proj + (gd ? 0.f : 1.f);
  float n2 = vertex_height(__ctx, proj + f32x2(1, 0));
  float n3 = vertex_height(__ctx, proj + f32x2(0, 1));
  float nt = vertex_height(__ctx, pt);
  float nmid = mix(n2, n3, d.data[1]);
  float nv, dx;
  if (gd) {
    nv = mix(nt, n3, d.data[1]);
    dx = d.data[0] / (1.f - d.data[1]);
  } else {
    nv = mix(n2, nt, d.data[1]);
    dx = (1.f - d.data[0]) / (d.data[1]);
  }
  return mix(nv, nmid, dx);
}
float sdf(Context &__ctx, f32x3 pos) {
  return pos.data[1] - height_and_color_map(__ctx, pos.swizzle<0, 2>());
}
void rayMarcher(Context &__ctx, f32x3 cameraPos, f32x3 lookDir, f32x2 screenDim,
                f32x2 uv, f32x3 &col) {
  f32x3 up = f32x3(0, 1, 0);
  f32x3 lookPerH = normalize(cross(lookDir, up));
  f32x3 lookPerV = normalize(cross(-lookDir, lookPerH));
  f32x3 screenCenter = cameraPos + lookDir;
  f32x3 screenPos = screenCenter +
                    0.5f * screenDim.data[0] * uv.data[0] * lookPerH +
                    0.5f * screenDim.data[1] * uv.data[1] * lookPerV;
  f32x3 rayDir = normalize(screenPos - cameraPos);
  float t = 0.0f;
  float dist;
  f32x3 pos;
  int iter;
  do {
    pos = cameraPos + t * rayDir;
    dist = sdf(__ctx, pos);
    t += 0.5f * dist;
    iter++;
  } while (iter < MAX_ITER && dist < MAX_DIST && dist > EPS);
  if (dist <= EPS) {
    f32x3 new_col = f32x3(0, 0, 0);
    f32x3 nml = normalize(f32x3(dist - sdf(__ctx, pos - f32x3(EPS, 0, 0)),
                                dist - sdf(__ctx, pos - f32x3(0, EPS, 0)),
                                dist - sdf(__ctx, pos - f32x3(0, 0, EPS))));
    float dx = mod(pos.data[0] - GRID_DIM / 2.0f, GRID_DIM);
    dx = min(dx, GRID_DIM - dx);
    float dz = mod(pos.data[2], GRID_DIM);
    dz = min(dz, GRID_DIM - dz);
    float col_t = smoothstep(4.f, 7.f, abs(pos.data[0]));
    f32x3 col_dark = mix(f32x3(0.3f, 0, 0.3f), f32x3(0.1f, 0.2f, 0.3f), col_t);
    f32x3 col_bright = mix(f32x3(1.f, 0.9f, 1.f), f32x3(0.9f, 1.f, 1.f), col_t);
    f32x3 lightDir = f32x3(0.0f, -0.5f, 1);
    new_col = mix(new_col, col_dark, clamp(dot(-lightDir, nml), 0.0f, 1.0f));
    new_col = mix(col_bright, new_col, smoothstep(0.02f, 0.04f, min(dx, dz)));
    col = mix(new_col, col,
              smoothstep(28.f, 30.f, cameraPos.data[2] - pos.data[2]));
  }
}
f32x4 sampleColor(Context &__ctx, f32x2 sampleCoord) {
  float aspectRatio = iResolution.data[0] / iResolution.data[1];
  float screenWidth = 2.0f * D * atan(0.5f * FOVH);
  f32x3 cameraPos = f32x3(0, 0, -2.f * iTime);
  f32x3 lookDir = f32x3(0, 0, -D);
  f32x2 screenDim = f32x2(screenWidth, screenWidth / aspectRatio);
  f32x2 uv = 2.0f * sampleCoord / iResolution.swizzle<0, 1>() - 1.0f;
  f32x2 uvf = 2.0f * (sampleCoord - iResolution.swizzle<0, 1>() / 2.0f) /
              iResolution.data[1];
  f32x3 col = BG;
  float noiseVal = noise(__ctx, 60.0f * uvf);
  float whiteIntensity = smoothstep(0.6f, 1.0f, noiseVal);
  col = mix(col, f32x3(1, 1, 1), whiteIntensity);
  const f32x2 sun_c = f32x2(0, 0.25f);
  const float sun_r = 0.4f;
  if (length(uvf - sun_c) < sun_r) {
    f32x3 sun_col = mix(f32x3(1, .4f, .8f), f32x3(1, 1, .3f),
                        (uv.data[1] - sun_c.data[1] + sun_r) / (2.f * sun_r));
    const float BAND_HEIGHT = 0.02f;
    float dy = mod(uvf.data[1], BAND_HEIGHT) / BAND_HEIGHT;
    col =
        mix(sun_col, f32x3(0, 0, 0), smoothstep(0.25f, 0.35f, abs(dy - 0.5f)));
  }
  rayMarcher(__ctx, cameraPos, lookDir, screenDim, uv, col);
  return f32x4(col, 1.0f);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x4 colSum = f32x4(0);
  for (int i = 0; i < FSAA; i++) {
    for (int j = 0; j < FSAA; j++) {
      colSum += sampleColor(__ctx, fragCoord + f32x2(float(i) / float(FSAA),
                                                     float(j) / float(FSAA)));
    }
  }
  fragColor = colSum / colSum.data[3];
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output) {
  iResolution = f32x3(p->resolution_time.data[0], p->resolution_time.data[1],
                      p->resolution_time.data[2]);
  gl_FragCoord =
      f32x4(frag_coord[0], frag_coord[1], frag_coord[2], frag_coord[3]);
  iTime = p->resolution_time.data[3];
  iMouse = f32x4(p->mouse.data[0], p->mouse.data[1], p->mouse.data[2],
                 p->mouse.data[3]);
  iDate =
      f32x4(p->date.data[0], p->date.data[1], p->date.data[2], p->date.data[3]);
  iTimeDelta = p->time_delta;
  iFrameRate = p->frame_rate;
  iFrame = p->frame;
  iSampleRate = p->sample_rate;
  for (unsigned i = 0; i < 4; ++i) {
    iChannelTime[i] = p->channel_time[i];
    iChannelResolution[i] = f32x3(p->channel_resolution[i].data[0],
                                  p->channel_resolution[i].data[1],
                                  p->channel_resolution[i].data[2]);
  }
  Context __ctx{};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
