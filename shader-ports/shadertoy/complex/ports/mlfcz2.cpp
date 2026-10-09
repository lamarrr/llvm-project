// Port of https://www.shadertoy.com/view/mlfcz2; author: jarble.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/mlfcz2.json.
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
f32x2 triwave(Context &__ctx, f32x2 uv) {
  return max(abs(fract(uv) - .5f), abs(fract(uv.swizzle<1, 0>() - .01f) - .5f));
}
f32x2x2 rotate2D(Context &__ctx, float r) {
  return f32x2x2(cos(r), sin(r), -sin(r), cos(r));
}
float fbm(Context &__ctx, f32x2 uv, int octaves) {
  float value = 0.f, value1 = value, amplitude = 2.f;
  uv /= 32.f / 4.f;
  f32x2 t1 = f32x2(0.f);
  f32x2x2 r = rotate2D(__ctx, 12.f);
  f32x2 uv1 = uv;
  for (int i = 0; i < octaves; i++) {
    t1 = triwave(__ctx, uv - triwave(__ctx, uv1 * r / 2.15f)) -
         t1.swizzle<1, 0>();
    value1 = sqrt(value1 * value1 + value * value);
    value = ((t1.data[0] - t1.data[1]) * amplitude - value);
    uv1 = uv;
    amplitude /= 2.15f;
    uv = (uv.swizzle<1, 0>() * 2.15f + t1) * r;
  }
  return value1;
}
float fbm(Context &__ctx, f32x2 uv) { return fbm(__ctx, uv, 8); }
float f(Context &__ctx, f32x3 p, int iters) {
  float h = fbm(__ctx, p.swizzle<0, 2>(), iters);
  return h;
}
float f(Context &__ctx, f32x3 p) {
  float h = fbm(__ctx, p.swizzle<0, 2>(), 12);
  return h;
}
f32x3 getNormal(Context &__ctx, f32x3 p, float t) {
  f32x3 eps = f32x3(.001f * t, .0f, .0f);
  f32x3 n = f32x3(f(__ctx, p - eps.swizzle<0, 1, 1>()) -
                      f(__ctx, p + eps.swizzle<0, 1, 1>()),
                  2.f * eps.data[0],
                  f(__ctx, p - eps.swizzle<1, 1, 0>()) -
                      f(__ctx, p + eps.swizzle<1, 1, 0>()));
  return normalize(n);
}
float rayMarching(Context &__ctx, f32x3 ro, f32x3 rd, float tMin, float tMax) {
  float t = tMin;
  for (int i = 0; i < 300; i++) {
    f32x3 pos = ro + t * rd;
    float f1 = f(__ctx, pos, 8);
    float h = pos.data[1] - f1;
    if (abs(h) < (0.0015f * t) || t > tMax)
      break;
    t += 0.4f * h;
  }
  return t;
}
f32x3 lighting(Context &__ctx, f32x3 p, f32x3 normal, f32x3 L, f32x3 V) {
  f32x3 sunColor = f32x3(1.f, .956f, .839f);
  f32x3 albedo = f32x3(1.f);
  f32x3 diff = max(dot(normal, L) * albedo, 0.f);
  f32x3 refl = normalize(reflect(L, normal));
  float spec = max(dot(refl, -normalize(V)), 0.f);
  spec = pow(spec, 18.f);
  spec = clamp(spec, 0.f, 1.f);
  float sky = max(0.0f, dot(f32x3(0.f, 1.f, 0.f), normal));
  f32x3 col = diff * sunColor;
  col += spec * sunColor;
  col += sky * f32x3(0.f, .6f, 1.f) * .1f;
  return col;
}
f32x3x3 lookAt(Context &__ctx, f32x3 origin, f32x3 target, float roll) {
  f32x3 rr = f32x3(sin(roll), cos(roll), 0.0f);
  f32x3 ww = normalize(target - origin);
  f32x3 uu = normalize(cross(ww, rr));
  f32x3 vv = normalize(cross(uu, ww));
  return f32x3x3(uu, vv, ww);
}
f32x3 camerapath(Context &__ctx, float t) {
  f32x3 p =
      f32x3(-13.0f + 3.5f * cos(t), 3.3f, -1.1f + 2.4f * cos(2.4f * t + 2.0f));
  return p;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv =
      (fragCoord - iResolution.swizzle<0, 1>() * .5f) / iResolution.data[1];
  f32x3 lightDir = normalize(f32x3(-.8f, .15f, -.3f));
  f32x3 camStep =
      f32x3(lightDir.data[0], 0.f, lightDir.data[2]) * (iTime + 12.f) / 4.f;
  f32x3 camPos = f32x3(8.f, 2.f, 5.f) + camStep;
  f32x3 camTarget = f32x3(1.f, 1.f, 4.f) + camStep;
  f32x3x3 mat = lookAt(__ctx, camPos, camTarget, 0.0f);
  f32x3 ro = camPos;
  ro.data[1] += fbm(__ctx, ro.swizzle<0, 2>(), 8) - 1.8f;
  f32x3 rd = normalize(mat * f32x3(uv.swizzle<0, 1>(), 1.0f));
  if (length(iMouse.swizzle<0, 1>()) > 40.0f) {
    rd.set_swizzle<1, 0>(
        rd.swizzle<1, 0>() *
        (f32x2x2(
            cos(3.14f * 0.5f - iMouse.data[1] / iResolution.data[1] * 3.14f),
            sin(3.14f * 0.5f - iMouse.data[1] / iResolution.data[1] * 3.14f),
            -sin(3.14f * 0.5f - iMouse.data[1] / iResolution.data[1] * 3.14f),
            cos(3.14f * 0.5f - iMouse.data[1] / iResolution.data[1] * 3.14f))));
    rd.set_swizzle<0, 2>(
        rd.swizzle<0, 2>() *
        (f32x2x2(cos(3.14f - iTime / 8.f -
                     iMouse.data[0] / iResolution.data[0] * 3.14f * 2.0f),
                 sin(3.14f - iTime / 8.f -
                     iMouse.data[0] / iResolution.data[0] * 3.14f * 2.0f),
                 -sin(3.14f - iTime / 8.f -
                      iMouse.data[0] / iResolution.data[0] * 3.14f * 2.0f),
                 cos(3.14f - iTime / 8.f -
                     iMouse.data[0] / iResolution.data[0] * 3.14f * 2.0f))));
  }
  float tMin = .1f;
  float tMax = 20.f;
  float t = rayMarching(__ctx, ro, rd, tMin, tMax);
  f32x3 col = f32x3(0.f);
  if (t > tMax) {
    float sundot = clamp(dot(rd, lightDir), 0.0f, 1.0f);
    col = f32x3(0.3f, 0.5f, 0.85f) - rd.data[1] * rd.data[1] * 0.5f;
    col = mix(col, 0.85f * f32x3(0.7f, 0.75f, 0.85f),
              pow(1.0f - max(rd.data[1], 0.0f), 4.0f));
    col += 0.25f * f32x3(1.0f, 0.7f, 0.4f) * pow(sundot, 5.0f);
    col += 0.25f * f32x3(1.0f, 0.8f, 0.6f) * pow(sundot, 64.0f);
    col += 0.2f * f32x3(1.0f, 0.8f, 0.6f) * pow(sundot, 512.0f);
    f32x2 sc = ro.swizzle<0, 2>() + rd.swizzle<0, 2>() *
                                        ((250.0f) * 1000.0f - ro.data[1]) /
                                        rd.data[1];
    col =
        mix(col, f32x3(1.0f, 0.95f, 1.0f),
            0.5f * smoothstep(0.5f, 0.8f, fbm(__ctx, 0.0005f * sc / (250.0f))));
    col = mix(col, 0.68f * f32x3(0.4f, 0.65f, 1.0f),
              pow(1.0f - max(rd.data[1], 0.0f), 16.0f));
  } else {
    f32x3 p = ro + rd * t;
    f32x3 normal = getNormal(__ctx, p, t);
    f32x3 viewDir = normalize(ro - p);
    col = lighting(__ctx, p, normal, lightDir, viewDir);
    float fo = 1.0f - exp(-pow(30.f * t / (250.0f), 1.5f));
    f32x3 fco = 0.65f * f32x3(0.4f, 0.65f, 1.0f);
    col = mix(col, fco, fo);
  }
  col = pow(clamp(col, 0.f, 1.f), f32x3(.45f));
  fragColor = f32x4(f32x3(col), 1.0f);
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
