// Port of https://www.shadertoy.com/view/DlGXzh; author: jakel101.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/DlGXzh.json.
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
f32x2 sim2d(Context &__ctx, f32x2 p, float s) {
  f32x2 ret = p;
  ret = p + s / 2.0f;
  ret = fract(ret / s) * s - s / 2.0f;
  return ret;
}
f32x3 stepspace(Context &__ctx, f32x3 p, float s) {
  f32x3 grid = p - mod(p - s / 2.0f, s);
  return grid;
}
float obj(Context &__ctx, f32x3 p) {
  f32x3 fp = stepspace(__ctx, p, 2.0f);
  ;
  float d = 2.0f + sin(iTime);
  d = d * 1.0f / length(.1f * p.swizzle<0, 2>()) + 2.0f;
  if (p.data[0] > 1.0f)
    d = -5.0f;
  p.data[1] = p.data[1] + d;
  p.set_swizzle<0, 2>(sim2d(__ctx, p.swizzle<0, 2>(), 2.0f));
  float c1 = length(max(abs(p) - f32x3(0.7f, 0.7f, 0.7f), 0.0f)) - 0.15f;
  float c2 = length(max(abs(p) - f32x3(0.3f, 0.9f, 0.4f), 0.0f)) - 0.15f;
  float cf = sin(iTime) * 0.5f + 0.5f;
  return mix(c1, c2, cf);
}
f32x3 obj_c(Context &__ctx, f32x3 p) {
  f32x2 fp = sim2d(__ctx, p.swizzle<0, 2>() - 15.0f, 16.0f);
  if (fp.data[1] > 4.0f)
    fp.data[0] = -fp.data[0];
  if (fp.data[0] > 0.0f)
    return f32x3(0.0f, 1.0f, 0.0f);
  else
    return f32x3(0.0f, 0.1f, 0.0f);
}
float PI = 3.14159265f;
f32x3 phong(Context &__ctx, f32x3 pt, f32x3 prp, f32x3 normal, f32x3 light,
            f32x3 color, float spec, f32x3 ambLight) {
  f32x3 lightv = normalize(light - pt);
  float diffuse = dot(normal, lightv);
  f32x3 refl = -reflect(lightv, normal);
  f32x3 viewv = normalize(prp - pt);
  float specular = pow(max(dot(refl, viewv), 0.0f), spec);
  return (max(diffuse, 0.0f) + ambLight) * color + specular;
}
float raymarching(Context &__ctx, f32x3 prp, f32x3 scp, int maxite,
                  float precis, float startf, float maxd, int &objfound) {
  const f32x3 e = f32x3(0.1f, 0, 0.0f);
  float s = startf;
  f32x3 c, p, n;
  float f = startf;
  objfound = 1;
  for (int i = 0; i < 256; i++) {
    if (abs(s) < precis || f > maxd || i > maxite)
      break;
    f += s;
    p = prp + scp * f;
    s = obj(__ctx, p);
  }
  if (f > maxd)
    objfound = -1;
  return f;
}
f32x3 camera(Context &__ctx, f32x3 prp, f32x3 vrp, f32x3 vuv, float vpd,
             f32x2 fragCoord) {
  f32x2 vPos =
      -1.0f + 2.0f * fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x3 vpn = normalize(vrp - prp);
  f32x3 u = normalize(cross(vuv, vpn));
  f32x3 v = cross(vpn, u);
  f32x3 scrCoord =
      prp + vpn * vpd +
      vPos.data[0] * u * iResolution.data[0] / iResolution.data[1] +
      vPos.data[1] * v;
  return normalize(scrCoord - prp);
}
f32x3 normal(Context &__ctx, f32x3 p) {
  const float n_er = 0.01f;
  float v1 =
      obj(__ctx, f32x3(p.data[0] + n_er, p.data[1] - n_er, p.data[2] - n_er));
  float v2 =
      obj(__ctx, f32x3(p.data[0] - n_er, p.data[1] - n_er, p.data[2] + n_er));
  float v3 =
      obj(__ctx, f32x3(p.data[0] - n_er, p.data[1] + n_er, p.data[2] - n_er));
  float v4 =
      obj(__ctx, f32x3(p.data[0] + n_er, p.data[1] + n_er, p.data[2] + n_er));
  return normalize(
      f32x3(v4 + v1 - v3 - v2, v3 + v4 - v1 - v2, v2 + v4 - v3 - v1));
}
f32x3 render(Context &__ctx, f32x3 prp, f32x3 scp, int maxite, float precis,
             float startf, float maxd, f32x3 background, f32x3 light,
             float spec, f32x3 ambLight, f32x3 &n, f32x3 &p, float &f,
             int &objfound) {
  objfound = -1;
  f = raymarching(__ctx, prp, scp, maxite, precis, startf, maxd, objfound);
  if (objfound > 0) {
    p = prp + scp * f;
    f32x3 c = obj_c(__ctx, p);
    n = normal(__ctx, p);
    f32x3 cf = phong(__ctx, p, prp, n, light, c, spec, ambLight);
    return f32x3(cf);
  }
  f = maxd;
  return f32x3(background);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 vuv = f32x3(0, 1, 0);
  f32x3 vrp = f32x3(0.0f, 0.0f, 0.0f);
  float mx = iMouse.data[0] / iResolution.data[0] * PI * 2.0f + (iTime * 0.3f);
  float my = iMouse.data[1] / iResolution.data[1] * PI / 2.01f;
  if ((iMouse.data[0] <= 0.0f) || (iMouse.data[1] <= 0.0f)) {
    mx = 1.0f, my = 0.05f;
  };
  f32x3 prp =
      vrp + f32x3(cos(my) * cos(mx), sin(my), cos(my) * sin(mx)) * 12.0f;
  float vpd = 1.5f;
  f32x3 light = prp + f32x3(5.0f, 0, 5.0f);
  f32x3 scp = camera(__ctx, prp, vrp, vuv, vpd, fragCoord);
  f32x3 n, p;
  float f;
  int o;
  const float maxe = 0.01f;
  const float startf = 0.1f;
  const f32x3 backc = f32x3(0.0f, 0.0f, 0.0f);
  const float spec = 8.0f;
  const f32x3 ambi = f32x3(0.1f, 0.1f, 0.1f);
  f32x3 c1 = render(__ctx, prp, scp, 256, maxe, startf, 60.0f, backc, light,
                    spec, ambi, n, p, f, o);
  c1 = c1 * max(1.0f - f * .015f, 0.0f);
  f32x3 c2 = backc;
  if (o > 0) {
    scp = reflect(scp, n);
    c2 = render(__ctx, p + scp * 0.05f, scp, 32, maxe, startf, 10.0f, backc,
                light, spec, ambi, n, p, f, o);
  }
  c2 = c2 * max(1.0f - f * .1f, 0.0f);
  fragColor = f32x4(
      c1.swizzle<0, 1, 2>() * 0.75f + c2.swizzle<0, 1, 2>() * 0.25f, 1.0f);
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
