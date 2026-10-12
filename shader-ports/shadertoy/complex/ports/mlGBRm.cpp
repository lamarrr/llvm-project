// Port of https://www.shadertoy.com/view/mlGBRm; author: Real_NC.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlGBRm.json.
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
float sdDoubleSphere(Context &__ctx, f32x3 p, f32x4 s1, f32x4 s2) {
  float d1 = abs(distance(p, s1.swizzle<0, 1, 2>()) - s1.data[3]);
  float d2 = distance(p, s2.swizzle<0, 1, 2>()) - s2.data[3];
  float ringX = 0.5f * (s1.data[3] * s1.data[3] - s2.data[3] * s2.data[3]) /
                    distance(s1.swizzle<0, 1, 2>(), s2.swizzle<0, 1, 2>()) -
                0.5f * distance(s1.swizzle<0, 1, 2>(), s2.swizzle<0, 1, 2>());
  f32x3 ringPos =
      s2.swizzle<0, 1, 2>() +
      normalize(s2.swizzle<0, 1, 2>() - s1.swizzle<0, 1, 2>()) * ringX;
  float ringRad =
      sqrt(s1.data[3] * s1.data[3] - dot(s1.swizzle<0, 1, 2>() - ringPos,
                                         s1.swizzle<0, 1, 2>() - ringPos));
  f32x3 tnorm = normalize(s2.swizzle<0, 1, 2>() - s1.swizzle<0, 1, 2>());
  f32x3 p2 = p + tnorm * dot(ringPos.swizzle<0, 1, 2>() - p, tnorm);
  p2 = normalize(p2 - ringPos) * (ringRad) + ringPos;
  f32x3 closests1 = normalize(p - s1.swizzle<0, 1, 2>()) * s1.data[3];
  if (dot(p2 - s2.swizzle<0, 1, 2>(), p - p2) < 0.0f)
    return d1;
  return distance(p, p2);
}
float sdFlower(Context &__ctx, f32x3 p) {
  p.data[0] = abs(p.data[0]);
  float d = length(p);
  float r = 1.0f;
  for (r; r <= 20.0f; r *= 2.0f) {
    d = min(d, sdDoubleSphere(__ctx, p, f32x4(r, 0, 0, r), f32x4(0, 4, 0, 4)));
  }
  return d;
}
float map(Context &__ctx, f32x3 p) {
  return sdFlower(__ctx, p + f32x3(0, 4, 0)) - 0.1f;
}
f32x3 normal(Context &__ctx, f32x3 p) {
  f32x2 q = f32x2(0.001f, 0);
  return normalize(f32x3(map(__ctx, p + q.swizzle<0, 1, 1>()) -
                             map(__ctx, p - q.swizzle<0, 1, 1>()),
                         map(__ctx, p + q.swizzle<1, 0, 1>()) -
                             map(__ctx, p - q.swizzle<1, 0, 1>()),
                         map(__ctx, p + q.swizzle<1, 1, 0>()) -
                             map(__ctx, p - q.swizzle<1, 1, 0>())));
}
float checker(Context &__ctx, f32x3 p) {
  p = fract(p) - 0.5f;
  return 0.6f + 0.4f * sign(p.data[0] * p.data[1] * p.data[2]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 ro = f32x3(0, 0, -14);
  f32x3 rd = normalize(f32x3(uv, 1.7f));
  float yaw = iTime;
  if (iMouse.data[2] > 0.f)
    yaw = iMouse.data[0] / iResolution.data[0] * 6.28f - 3.14f;
  float pitch = iMouse.data[1] / iResolution.data[1] * 3.14f - 1.57f;
  f32x2x2 rot = f32x2x2(cos(yaw), sin(yaw), sin(yaw), -cos(yaw));
  f32x2x2 rotp = f32x2x2(cos(pitch), -sin(pitch), sin(pitch), cos(pitch));
  rd.set_swizzle<2, 1>(-(-rd.swizzle<2, 1>() * rotp));
  ro.set_swizzle<2, 1>(ro.swizzle<2, 1>() * (rotp));
  rd.set_swizzle<0, 2>(-(-rd.swizzle<0, 2>() * rot));
  ro.set_swizzle<0, 2>(ro.swizzle<0, 2>() * (rot));
  float d = 9.f;
  for (int i = 0; i <= 200; i++) {
    if (map(__ctx, ro + rd * d) < 0.001f)
      break;
    d += map(__ctx, ro + rd * d);
    if (d >= 18.1f) {
      d = -1.0f;
      break;
    }
  }
  f32x3 ri = ro + rd * d;
  fragColor = f32x4(0);
  fragColor.set_swizzle<0, 1, 2>(f32x3(checker(__ctx, ri * 1.f)));
  f32x3 norm = normal(__ctx, ri);
  f32x3 light = normalize(f32x3(1, 1, -1));
  fragColor *= 0.5f + 0.5f * dot(norm, light);
  fragColor *= 0.6f;
  fragColor += pow(max(dot(reflect(rd, norm), light), 0.f), 28.0f);
  if (d <= 0.f)
    fragColor *= 0.f;
  f32x3 ps = ro - rd * dot(rd, ro);
  float ds = -dot(rd, ro) - sqrt(16.0f - dot(ps, ps));
  ri = ro + rd * ds;
  float sphereMesh;
  if (dot(ps, ps) <= 16.0f) {
    float d2 = -dot(rd + rd, ro) - ds;
    f32x3 ri2 = ro + rd * d2;
    if (d <= 0.f || d2 < d) {
      f32x3 ri2 = ro + rd * (-dot(rd + rd, ro) - ds);
      sphereMesh = smoothstep(
          0.15f, 0.1f,
          length(ri2.swizzle<0, 2>()) *
              abs(fract(atan(ri2.data[0], ri2.data[2]) * 4.77464829276f) -
                  0.5f));
      sphereMesh = mix(sphereMesh, 1.0f,
                       smoothstep(0.15f, 0.1f,
                                  3.14f * abs(fract(acos(ri2.data[1] / 4.0f) *
                                                    4.77464829276f) -
                                              0.5f)));
      fragColor = mix(fragColor, f32x4(0.6f, 0.2f, 0.1f, 1), sphereMesh * 0.4f);
    }
    sphereMesh = smoothstep(
        0.15f, 0.1f,
        length(ri.swizzle<0, 2>()) *
            abs(fract(atan(ri.data[0], ri.data[2]) * 4.77464829276f) - 0.5f));
    sphereMesh = mix(
        sphereMesh, 1.0f,
        smoothstep(0.15f, 0.1f,
                   3.14f * abs(fract(acos(ri.data[1] / 4.0f) * 4.77464829276f) -
                               0.5f)));
    if (ds < d || d <= 0.f)
      fragColor = mix(fragColor, f32x4(0.8f, 0, 0, 1), sphereMesh * 0.4f);
  }
  fragColor = pow(fragColor, f32x4(1.f / 2.2f));
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
