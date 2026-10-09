// Port of https://www.shadertoy.com/view/dtBXDc; author: maras.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtBXDc.json.
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

float smin(Context &__ctx, float a, float b, float k) {
  float h = max(k - abs(a - b), 0.0f);
  return min(a, b) - h * h / (k * 4.0f);
}
float sdSphere(Context &__ctx, f32x3 pos, float rad) {
  return length(pos) - rad;
}
float sdMirror(Context &__ctx, f32x3 pos) {
  float mirror = max(pos.data[0] + 1.0f, pos.data[1] - 0.8f);
  mirror = max(mirror, abs(pos.data[2] + 0.7f) - 0.8f);
  mirror = max(mirror, -pos.data[0] - 1.25f);
  return mirror;
}
float sdFrame(Context &__ctx, f32x3 pos) {
  float frame = max(pos.data[0] + 1.001f, pos.data[1] - 0.9f);
  frame = max(frame, abs(pos.data[2] + 0.7f) - 0.9f);
  frame = max(frame, -pos.data[0] - 1.2499f);
  return frame;
}
float map(Context &__ctx, f32x3 pos, bool reflected, float &material) {
  float ground = pos.data[1] + 0.25f;
  float sphere = sdSphere(__ctx, pos + f32x3(-0.5f, -0.2f, 0.8f), 0.25f);
  float sphere2 =
      sdSphere(__ctx, pos + f32x3(-0.95f, -0.2f, 0.8f + sin(iTime)), 0.25f);
  float mirror = sdMirror(__ctx, pos);
  float frame = max(sdFrame(__ctx, pos), -mirror);
  float res;
  res = ground;
  material = 0.0f;
  float sphere_blend = smin(__ctx, sphere, sphere2, 0.25f);
  if (sphere_blend < res) {
    res = sphere_blend;
    material = 1.0f;
    if (sphere2 < sphere)
      material = 2.0f;
  }
  if (!reflected && frame < res) {
    res = frame;
    material = 3.0f;
  }
  if (!reflected && mirror < res) {
    res = mirror;
    material = 4.0f;
  }
  return res;
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x2 e = f32x2(0.0001f, 0.0f);
  float mat = 0.0f;
  return normalize(
      f32x3(map(__ctx, pos + e.swizzle<0, 1, 1>(), false, mat) -
                map(__ctx, pos - e.swizzle<0, 1, 1>(), false, mat),
            map(__ctx, pos + e.swizzle<1, 0, 1>(), false, mat) -
                map(__ctx, pos - e.swizzle<1, 0, 1>(), false, mat),
            map(__ctx, pos + e.swizzle<1, 1, 0>(), false, mat) -
                map(__ctx, pos - e.swizzle<1, 1, 0>(), false, mat)));
}
float castRay(Context &__ctx, f32x3 &ro, f32x3 rd, bool &reflected,
              float &material) {
  float t = 0.0f;
  int i = 0;
  for (; i < 100; i++) {
    float map_res = map(__ctx, ro, reflected, material);
    float h = map_res;
    ro += h * rd;
    if (h < 0.001f) {
      if (material > 3.5f) {
        if (!reflected) {
          f32x3 norm = calcNormal(__ctx, ro - 0.001f * rd);
          rd = rd - 2.0f * dot(rd, norm) * norm;
          reflected = true;
        }
        t += h;
        continue;
      }
      break;
    }
    t += h;
    if (t > 20.0f) {
      break;
    }
  }
  if (t > 20.0f)
    t = -1.0f;
  return t;
}
float castShadow(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 0.0f;
  f32x3 pos = ro;
  bool reflected = false;
  float material = 0.0f;
  for (int i = 0; i < 100; i++) {
    float map_res = map(__ctx, pos, reflected, material);
    float h = map_res;
    pos += h * rd;
    if (h < 0.0001f) {
      break;
    }
    t += h;
    if (t > 20.0f) {
      break;
    }
  }
  if (t > 20.0f)
    t = -1.0f;
  return t;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 p =
      (2.0f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  float an_x = 10.0f * iMouse.data[0] / iResolution.data[0] + 1.0f;
  float an_y = 1.0f * iMouse.data[1] / iResolution.data[1];
  f32x3 ta = f32x3(0.0f, 0.25f, 0.0f);
  f32x3 ro = ta + f32x3(1.5f * sin(an_x), 1.5f * sin(an_y), 1.5f * cos(an_x));
  ro *= 1.5f;
  f32x3 ww = normalize(ta - ro);
  f32x3 uu = normalize(cross(ww, f32x3(0, 1, 0)));
  f32x3 vv = normalize(cross(uu, ww));
  f32x3 rd = normalize(f32x3(p.data[0] * uu + p.data[1] * vv + 1.8f * ww));
  f32x3 col = f32x3(0.4f, 0.75f, 1.0f) - 0.7f * rd.data[1];
  col = mix(col, f32x3(0.7f, 0.75f, 0.8f), exp(-10.0f * rd.data[1]));
  bool reflected = false;
  float material = 0.0f;
  float tp = castRay(__ctx, ro, rd, reflected, material);
  float t = tp;
  f32x3 pos = ro;
  if (t > 0.0f) {
    f32x3 nor = calcNormal(__ctx, pos);
    float row = floor(mod(pos.data[0] * 2.0f, 2.0f));
    float column = floor(mod(pos.data[2] * 2.0f, 2.0f));
    f32x3 mate = f32x3(0.2f);
    if (material < 0.5f && (row < 0.5f || column < 0.5f) &&
        !(row < 0.5f && column < 0.5f))
      mate = f32x3(0.1f);
    if (material < 1.5f && material > 0.5f)
      mate = f32x3(0.3f, 0.2f, 0.1f);
    if (material < 2.5f && material > 1.5f)
      mate = f32x3(0.1f, 0.2f, 0.3f);
    if (material < 3.5f && material > 2.5f)
      mate = f32x3(0.01f);
    f32x3 sun_dir =
        normalize(f32x3(sin(iTime / 5.0f), 0.4f, cos(iTime / 5.0f)));
    float sun_dif = clamp(dot(nor, sun_dir), 0.0f, 1.0f);
    float sun_sha = step(castShadow(__ctx, pos + nor * 0.001f, sun_dir), -0.5f);
    float sky_dif =
        clamp(0.5f + 0.5f * dot(nor, f32x3(0.0f, 1.0f, 0.0f)), 0.0f, 1.0f);
    float bou_dif =
        clamp(0.5f + 0.5f * dot(nor, f32x3(0.0f, -1.0f, 0.0f)), 0.0f, 1.0f);
    col = mate * f32x3(7.0f, 5.0f, 3.0f) * sun_dif * sun_sha;
    col += mate * f32x3(0.5f, 0.8f, 0.9f) * sky_dif;
    col += mate * f32x3(0.7f, 0.3f, 0.2f) * bou_dif;
  }
  col = pow(col, f32x3(0.4545f));
  if (reflected) {
    col *= 0.75f;
  }
  fragColor = f32x4(col, 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output) {
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
