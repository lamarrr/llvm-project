// Port of https://www.shadertoy.com/view/DlfcWN; author: Sleng.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlfcWN.json.
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

float mandelbulbDist(Context &__ctx, f32x3 pos);
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd);
f32x3 getNormal(Context &__ctx, f32x3 p);
float lightIntensity(Context &__ctx, f32x3 p, f32x3 lightDir);
f32x3x3 rotationMatrix(Context &__ctx, f32x3 axis, float angle);
float mandelbulbDist(Context &__ctx, f32x3 pos) {
  float angle = iTime / 10.f;
  f32x3x3 rot = rotationMatrix(__ctx, f32x3(0.0f, 1.0f, 0.0f), angle);
  pos = rot * pos;
  float scaleFactor = 0.85f;
  pos *= scaleFactor;
  f32x3 z = pos;
  float dr = 1.0f;
  float r = 0.0f;
  const int iterations = 5;
  const float bailout = 2.0f;
  const float power = 3.0f;
  for (int i = 0; i < iterations; i++) {
    r = length(z);
    if (r > bailout)
      return r;
    float theta = acos(z.data[2] / r);
    float phi = atan(z.data[1], z.data[0]);
    dr = pow(r, power - 1.0f) * power * dr + 1.0f;
    float zr = pow(r, power);
    theta *= power;
    phi *= power;
    z = zr * f32x3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    z += pos;
  }
  return 0.5f * log(r) * r / dr;
}
float rayMarch(Context &__ctx, f32x3 ro, f32x3 rd) {
  float totalDist = 0.0f;
  for (int i = 0; i < 128; i++) {
    f32x3 p = ro + rd * totalDist;
    float d = mandelbulbDist(__ctx, p);
    totalDist += d;
    if (d < 0.001f) {
      return totalDist;
    }
    if (totalDist > 4.0f)
      break;
  }
  return totalDist;
}
f32x3 getNormal(Context &__ctx, f32x3 p) {
  const f32x2 eps = f32x2(0.001f, 0.0f);
  f32x3 n;
  n.data[0] = mandelbulbDist(__ctx, p + eps.swizzle<0, 1, 1>()) -
              mandelbulbDist(__ctx, p - eps.swizzle<0, 1, 1>());
  n.data[1] = mandelbulbDist(__ctx, p + eps.swizzle<1, 0, 1>()) -
              mandelbulbDist(__ctx, p - eps.swizzle<1, 0, 1>());
  n.data[2] = mandelbulbDist(__ctx, p + eps.swizzle<1, 1, 0>()) -
              mandelbulbDist(__ctx, p - eps.swizzle<1, 1, 0>());
  return normalize(n);
}
float lightIntensity(Context &__ctx, f32x3 p, f32x3 lightDir) {
  f32x3 normal = getNormal(__ctx, p);
  return max(0.1f, dot(normal, lightDir));
}
f32x3x3 rotationMatrix(Context &__ctx, f32x3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0f - c;
  return f32x3x3(oc * axis.data[0] * axis.data[0] + c,
                 oc * axis.data[0] * axis.data[1] - axis.data[2] * s,
                 oc * axis.data[2] * axis.data[0] + axis.data[1] * s,
                 oc * axis.data[0] * axis.data[1] + axis.data[2] * s,
                 oc * axis.data[1] * axis.data[1] + c,
                 oc * axis.data[1] * axis.data[2] - axis.data[0] * s,
                 oc * axis.data[2] * axis.data[0] - axis.data[1] * s,
                 oc * axis.data[1] * axis.data[2] + axis.data[0] * s,
                 oc * axis.data[2] * axis.data[2] + c);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord / iResolution.swizzle<0, 1>()) * 2.0f - 1.0f;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  f32x3 color = f32x3(0.0f);
  f32x3 ro = f32x3(0.0f, 0.0f, -2.5f);
  f32x3 target = f32x3(0.0f, 0.0f, 0.0f);
  float angle = iTime;
  f32x3x3 rot = rotationMatrix(__ctx, f32x3(0.0f, 1.0f, 0.0f), angle);
  target = rot * target;
  f32x3 forward = normalize(target - ro);
  f32x3 right = cross(f32x3(0.0f, 1.0f, 0.0f), forward);
  f32x3 up = cross(forward, right);
  f32x3 c = ro + forward * 2.0f;
  f32x3 i = c + right * uv.data[0];
  f32x3 j = c + up * uv.data[1];
  f32x3 p = i + j;
  f32x3 rayDir = normalize(p - ro);
  float dist = rayMarch(__ctx, ro, rayDir);
  if (dist < 4.0f) {
    f32x3 lightDir = normalize(f32x3(0.5f, 1.0f, 0.8f));
    float diff = lightIntensity(__ctx, ro + rayDir * dist, lightDir);
    color = f32x3(1.0f, 0.0f, 0.0f) * diff;
  }
  fragColor = f32x4(color, 1.0f);
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
