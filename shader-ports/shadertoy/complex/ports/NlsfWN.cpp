// Port of https://www.shadertoy.com/view/NlsfWN; author: LongJohnny.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/NlsfWN.json.
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
f32x3 rayDir(Context &__ctx, f32x3 ro, f32x3 origin, f32x2 uv) {
  f32x3 d = normalize(origin - ro);
  f32x3 r = normalize(cross(f32x3(0.f, 1.f, 0.f), d));
  f32x3 u = cross(d, r);
  return normalize(d + r * uv.data[0] + u * uv.data[1]);
}
f32x2x2 rot(Context &__ctx, float a) {
  float s = sin(a), c = cos(a);
  return f32x2x2(c, -s, s, c);
}
bool intersectSphere(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 p, float r) {
  f32x3 oc = ro - p;
  float b = dot(oc, rd);
  float c = dot(oc, oc) - r * r;
  float h = b * b - c;
  if (h < 0.0f)
    return false;
  return true;
}
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float sdTorus(Context &__ctx, f32x3 p, f32x2 t) {
  f32x2 q = f32x2(length(p.swizzle<0, 2>()) - t.data[0], p.data[1]);
  return length(q) - t.data[1];
}
f32x4 sdScene(Context &__ctx, f32x3 p);
f32x4 rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  f32x4 res = f32x4(-1.f);
  float t = 0.001f;
  for (int i = 0; i < 50; ++i) {
    f32x4 ds = sdScene(__ctx, ro + rd * t);
    if (ds.data[3] < 0.01f) {
      res = f32x4(ds.swizzle<0, 1, 2>(), t);
      break;
    }
    if (ds.data[3] > 10.f) {
      res = f32x4(0.0f, 0.0f, 0.0f, -1);
      break;
    }
    t += ds.data[3];
  }
  return res;
}
f32x4 sdScene(Context &__ctx, f32x3 p) {
  float t = sdSphere(__ctx, p, 1.f);
  ;
  float ds;
  p.set_swizzle<1, 2>(p.swizzle<1, 2>() * (rot(__ctx, 3.14159265359f / 2.f)));
  float mrgs = 23.f;
  float rgs = floor((mrgs - 5.f) / 2.f);
  for (float i = 5.f; i < mrgs; i++) {
    if (mod(i, 2.f) == 0.f) {
      p.set_swizzle<0, 2>(
          p.swizzle<0, 2>() *
          (rot(__ctx, pow(sin(iTime * .07f), 2.f) * sin(iTime * .01f) * 10.f)));
      ds = sdTorus(__ctx, p, f32x2(i * .3f, .1f * min(3.f, i / rgs)));
      if (ds < t)
        t = ds;
    } else {
      p.set_swizzle<2, 1>(
          p.swizzle<2, 1>() *
          (rot(__ctx, pow(cos(iTime * .07f), 2.f) * cos(iTime * .01f) * 10.f)));
    }
  }
  return f32x4(f32x3(0.f), t);
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const float h = 0.0001f;
  const f32x2 k = f32x2(1, -1);
  return normalize(k.swizzle<0, 1, 1>() *
                       sdScene(__ctx, p + k.swizzle<0, 1, 1>() * h).data[3] +
                   k.swizzle<1, 1, 0>() *
                       sdScene(__ctx, p + k.swizzle<1, 1, 0>() * h).data[3] +
                   k.swizzle<1, 0, 1>() *
                       sdScene(__ctx, p + k.swizzle<1, 0, 1>() * h).data[3] +
                   k.swizzle<0, 0, 0>() *
                       sdScene(__ctx, p + k.swizzle<0, 0, 0>() * h).data[3]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (2.f * fragCoord - iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 origin = f32x3(0.f);
  f32x3 ro = f32x3(0.f, 0.f, -7.f);
  ro.data[2] -= (sin(iTime) * .5f + .5f) * 2.f;
  f32x3 rd = rayDir(__ctx, ro, origin, uv);
  f32x4 obj = f32x4(0.f);
  if (intersectSphere(__ctx, ro, rd, f32x3(0.f), 6.93f)) {
    obj = rayMarch(__ctx, ro, rd);
  }
  f32x3 col;
  if (obj.data[3] > 0.f) {
    f32x3 p = ro + rd * obj.data[3];
    f32x3 n = calcNormal(__ctx, p);
    float R0 = pow((1.f - 0.46094f) / (1.f + 0.46094f), 2.f);
    {
      float dif = -n.data[1] * .5f + .5f;
      float spe = -reflect(rd, n).data[1];
      spe = smoothstep(0.3f, 0.9f, spe);
      spe *= R0 + (1.f - R0) * pow(1.f - dot(rd, n), 5.f) * 0.3f;
      col += f32x3(227.f / 255.f, 147.f / 255.f, 64.f / 255.f) * dif * 0.02f;
      col +=
          f32x3(227.f / 255.f, 147.f / 255.f, 64.f / 255.f) * spe * dif * .8f;
    }
    {
      float dif = n.data[1] * .5f + .5f;
      float spe = reflect(rd, n).data[1];
      spe = smoothstep(0.3f, 0.9f, spe);
      spe *= R0 + (1.f - R0) * pow(1.f - dot(rd, n), 5.f) * 10.f;
      col += f32x3(.01f, 0.01f, .05f) * dif;
      col += f32x3(.01f, 0.01f, .05f) * spe * dif * .8f;
    }
    {
      f32x3 ld = normalize(f32x3(1.0f, -0.4f, 0.3f));
      float dif = max(0.f, dot(n, ld));
      f32x3 hlf = normalize(-rd + ld);
      float spe = pow(max(0.f, dot(n, hlf)), 16.f);
      spe *= R0 + (1.f - R0) * pow(1.f - dot(rd, n), 5.f);
      col += dif * f32x3(227.f / 255.f, 147.f / 255.f, 64.f / 255.f) * .5f;
      col +=
          spe * f32x3(227.f / 255.f, 147.f / 255.f, 64.f / 255.f) * dif * .8f;
    }
    {
      f32x3 ld = normalize(-f32x3(1.0f, -0.7f, 0.3f));
      float dif = max(0.f, dot(n, ld));
      f32x3 hlf = normalize(-rd + ld);
      float spe = pow(max(0.f, dot(n, hlf)), 16.f);
      spe *= R0 + (1.f - R0) * pow(1.f - dot(rd, n), 5.f);
      col += dif * f32x3(.01f, 0.01f, .05f) * .5f;
      col += spe * f32x3(.01f, 0.01f, .05f) * dif * 6.f * .8f;
    }
  } else {
    col = f32x3(.01f, 0.01f, .05f) *
          min(fragCoord.data[1] / iResolution.data[1] + .15f, 1.f);
  }
  col = pow(col, f32x3(.4545f));
  col = clamp(col, 0.f, 1.f);
  col = smoothstep(0.f, 1.f, col);
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
