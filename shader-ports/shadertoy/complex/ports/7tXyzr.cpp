// Port of https://www.shadertoy.com/view/7tXyzr; author: fl0a1e.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tXyzr.json.
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
float sdSphere(Context &__ctx, f32x3 p, float r) { return length(p) - r; }
float map(Context &__ctx, f32x3 pos) {
  float res = sdSphere(__ctx, pos - f32x3(0.0f, 0.0f, 0.0f), 0.1f);
  return res;
}
float raycast(Context &__ctx, f32x3 cameraPos, f32x3 rayDir) {
  float res = -1.0f;
  float minDist = 1.0f;
  float maxDist = 30.0f;
  float dist = minDist;
  for (int i = 0; i < 40 && dist < maxDist; i++) {
    float temp = map(__ctx, cameraPos + rayDir * dist);
    if (abs(temp) < 0.0001f) {
      res = dist;
      break;
    }
    dist += temp;
  }
  return res;
}
f32x3 calcNormal(Context &__ctx, f32x3 pos) {
  f32x3 n = f32x3(0.0f);
  for (int i = 0; i < 4; i++) {
    f32x3 e =
        0.5773f *
        (2.0f * f32x3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0f);
    n += e * map(__ctx, pos + 0.0005f * e);
  }
  return normalize(n);
}
f32x3 render(Context &__ctx, f32x3 cameraPos, f32x3 rayDir, f32x2 uv) {
  f32x3 col;
  float res = raycast(__ctx, cameraPos, rayDir);
  if (res > 0.f) {
    f32x3 pos = cameraPos + rayDir * res;
    f32x3 nor = calcNormal(__ctx, pos);
    {
      f32x3 sunPos = f32x3(15.0f * cos(iTime), 15.0f, 10.0f * sin(iTime));
      f32x2 polarUV =
          f32x2(atan(pos.data[0] * 800.f, pos.data[2] * 450.f) / 3.1415926f,
                4.f * pos.data[1]);
      f32x3 t = texture(__ctx.channel0, polarUV).swizzle<0, 1, 2>();
      f32x3 ambientColor = 2.f * t;
      f32x3 diffuseColor = 2.f * t;
      f32x3 specColor = f32x3(0.5f, 0.5f, 0.5f);
      f32x3 lightColor = f32x3(1.0f, 1.0f, 1.0f);
      f32x3 lightDir = normalize(sunPos - pos);
      float lambertian = max(dot(lightDir, nor), 0.0f);
      float shininess = 16.0f;
      float specular = 0.0f;
      float lightPower = 15.0f;
      f32x3 halfDir = normalize(lightDir - rayDir);
      float specAngle = max(dot(halfDir, nor), 0.0f);
      specular = pow(specAngle, shininess);
      col = ambientColor +
            diffuseColor * lambertian * lightColor * lightPower /
                length(sunPos - pos) * length(sunPos - pos) +
            specColor * specular * lightColor * lightPower /
                length(sunPos - pos) * length(sunPos - pos);
    }
  } else {
    col = f32x3(0.01f);
  }
  return col;
}
f32x3x3 setCamera(Context &__ctx, f32x3 ro, f32x3 ta, float cr) {
  f32x3 cw = normalize(ta - ro);
  f32x3 cu = normalize(cross(cw, f32x3(sin(cr), cos(cr), 0.0f)));
  f32x3 cv = (cross(cu, cw));
  return f32x3x3(cu, cv, cw);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 tot;
  f32x3 cameraPos = f32x3(3.f * sin(iTime), 0.5f, 3.0f * cos(iTime));
  f32x3 cameraOrit = f32x3(0.f, 0.f, 0.2f);
  f32x3x3 camera = setCamera(__ctx, cameraPos, cameraOrit, 0.f);
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 2; j++) {
      f32x2 offset = (f32x2(i, j) / float(2)) - 0.5f;
      f32x2 uv = ((fragCoord + offset) - 0.5f * iResolution.swizzle<0, 1>()) /
                 iResolution.data[0];
      f32x3 rayDir = camera * normalize(f32x3(uv, 3.5f));
      f32x3 col = render(__ctx, cameraPos, rayDir, uv);
      col = pow(col, f32x3(0.454545f));
      tot += col;
      tot /= float(2 * 2);
    }
  fragColor = f32x4(tot, 1.0f);
}

__fragment
void shadertoy(__push_constant const Params *p, __output(0) f32x4 *output,
               __binding(0, 0) texture_2d<f32x4> channel0,
               __binding(1, 0) sampler sampling0) {
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
