// Port of https://www.shadertoy.com/view/cl2yzh; author: alphardex.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/cl2yzh.json.
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
struct Context {
  Texture2D channel0;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];

f32x2x2 rotation2d(Context &__ctx, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  return f32x2x2(c, -s, s, c);
}
f32x4x4 rotation3d(Context &__ctx, f32x3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.f - c;
  return f32x4x4(oc * axis.data[0] * axis.data[0] + c,
                 oc * axis.data[0] * axis.data[1] - axis.data[2] * s,
                 oc * axis.data[2] * axis.data[0] + axis.data[1] * s, 0.f,
                 oc * axis.data[0] * axis.data[1] + axis.data[2] * s,
                 oc * axis.data[1] * axis.data[1] + c,
                 oc * axis.data[1] * axis.data[2] - axis.data[0] * s, 0.f,
                 oc * axis.data[2] * axis.data[0] - axis.data[1] * s,
                 oc * axis.data[1] * axis.data[2] + axis.data[0] * s,
                 oc * axis.data[2] * axis.data[2] + c, 0.f, 0.f, 0.f, 0.f, 1.f);
}
f32x2 rotate(Context &__ctx, f32x2 v, float angle) {
  return rotation2d(__ctx, angle) * v;
}
f32x3 rotate(Context &__ctx, f32x3 v, f32x3 axis, float angle) {
  return (rotation3d(__ctx, axis, angle) * f32x4(v, 1.f)).swizzle<0, 1, 2>();
}
float sdBox(Context &__ctx, f32x2 p, f32x2 b) {
  f32x2 d = abs(p) - b;
  return length(max(d, 0.f)) + min(max(d.data[0], d.data[1]), 0.f);
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.f);
}
float opUnion(Context &__ctx, float d1, float d2) { return min(d1, d2); }
f32x2 opUnion(Context &__ctx, f32x2 d1, f32x2 d2) {
  return (d1.data[0] < d2.data[0]) ? d1 : d2;
}
f32x2 map(Context &__ctx, f32x3 p) {
  f32x2 d = f32x2(1e10f, 0.f);
  f32x2 m = (iMouse.swizzle<0, 1>() - iResolution.swizzle<0, 1>() * .5f) /
            iResolution.swizzle<0, 1>();
  f32x3 p1 = p;
  p1.data[2] -= .2f;
  float tilt = .5f;
  p1 = rotate(__ctx, p1, f32x3(0.f, 1.f, 0.f), m.data[0] * tilt);
  p1 = rotate(__ctx, p1, f32x3(1.f, 0.f, 0.f), m.data[1] * tilt);
  float d1 = sdBox(__ctx, p1, f32x3(.4f, .4f, .001f));
  d = opUnion(__ctx, d, f32x2(d1, 1.f));
  f32x3 p2 = p;
  float d2 = sdBox(__ctx, p2, f32x3(1.f, 1.f, .001f));
  d = opUnion(__ctx, d, f32x2(d2, 2.f));
  return d;
}
f32x3 calcNormal(Context &__ctx, f32x3 p) {
  const float h = .0001f;
  const f32x2 k = f32x2(1, -1);
  return normalize(
      k.swizzle<0, 1, 1>() * map(__ctx, p + k.swizzle<0, 1, 1>() * h).data[0] +
      k.swizzle<1, 1, 0>() * map(__ctx, p + k.swizzle<1, 1, 0>() * h).data[0] +
      k.swizzle<1, 0, 1>() * map(__ctx, p + k.swizzle<1, 0, 1>() * h).data[0] +
      k.swizzle<0, 0, 0>() * map(__ctx, p + k.swizzle<0, 0, 0>() * h).data[0]);
}
float gaussian(Context &__ctx, f32x2 d, float sigma) {
  return exp(-(d.data[0] * d.data[0] + d.data[1] * d.data[1]) /
             (2.f * sigma * sigma));
}
f32x4 gaussianBlur2D(Context &__ctx, sampler2D tex, f32x2 st, f32x2 offset,
                     const int kernelSize) {
  f32x4 accumColor = f32x4(0.f);
  float kernelSizef = float(kernelSize);
  float accumWeight = 0.f;
  const float k = .15915494f;
  f32x2 xy = f32x2(0.f);
  for (int j = 0; j < 20; j++) {
    if (j >= kernelSize)
      break;
    xy.data[1] = -.5f * (kernelSizef - 1.f) + float(j);
    for (int i = 0; i < 20; i++) {
      if (i >= kernelSize)
        break;
      xy.data[0] = -.5f * (kernelSizef - 1.f) + float(i);
      float weight = (k / kernelSizef) * gaussian(__ctx, xy, kernelSizef);
      accumColor += weight * texture(tex, st + xy * offset);
      accumWeight += weight;
    }
  }
  return accumColor / accumWeight;
}
f32x3 render(Context &__ctx, f32x2 uv) {
  f32x3 col = f32x3(0.f);
  f32x4 tex = texture(__ctx.channel0, uv);
  f32x4 blurTex = gaussianBlur2D(__ctx, __ctx.channel0, uv,
                                 1.f / iResolution.swizzle<0, 1>(), 10);
  uv = (uv - .5f) * 2.f;
  f32x3 ro = f32x3(0.f, 0.f, 1.f);
  f32x3 rd = normalize(f32x3(uv, 0.f) - ro);
  float depth = 0.f;
  for (int i = 0; i < 64; i++) {
    f32x3 p = ro + rd * depth;
    f32x2 t = map(__ctx, p);
    float d = t.data[0];
    float m = t.data[1];
    depth += d;
    if (d < .01f) {
      f32x3 normal = calcNormal(__ctx, p);
      f32x3 objectColor = f32x3(1.f);
      if (m == 1.f) {
        objectColor = blurTex.swizzle<0, 1, 2>();
      } else if (m == 2.f) {
        objectColor = tex.swizzle<0, 1, 2>();
      }
      f32x3 lightColor = f32x3(1.f, 1.f, 1.f);
      if (m == 1.f) {
        float ambIntensity = .1f;
        f32x3 ambient = lightColor * ambIntensity;
        col += ambient * objectColor;
        f32x3 lightPos = f32x3(20.f);
        f32x3 lightDir = normalize(lightPos - p);
        float diff = dot(normal, lightDir);
        diff = max(diff, 0.f);
        f32x3 diffuse = lightColor * diff;
        col += diffuse * objectColor;
        f32x3 reflectDir = reflect(-lightDir, normal);
        f32x3 viewDir = normalize(ro - p);
        f32x3 halfVec = normalize(lightDir + viewDir);
        float spec = dot(normal, halfVec);
        spec = max(spec, 0.f);
        float shininess = 32.f;
        spec = pow(spec, shininess);
        f32x3 specular = lightColor * spec;
        col += specular * objectColor;
      } else if (m == 2.f) {
        float ambIntensity = 1.f;
        f32x3 ambient = lightColor * ambIntensity;
        col += ambient * objectColor;
      }
      break;
    }
  }
  return col;
}
f32x3 getSceneColor(Context &__ctx, f32x2 fragCoord) {
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  f32x3 col = render(__ctx, uv);
  return col;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 tot = f32x3(0.f);
  float AA_size = 2.f;
  float count = 0.f;
  for (float aaY = 0.f; aaY < AA_size; aaY++) {
    for (float aaX = 0.f; aaX < AA_size; aaX++) {
      tot += getSceneColor(__ctx, fragCoord + f32x2(aaX, aaY) / AA_size);
      count += 1.f;
    }
  }
  tot /= count;
  fragColor = f32x4(tot, 1.f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0) {
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
  Context __ctx{{channel0, sampling0}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
