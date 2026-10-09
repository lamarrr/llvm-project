// Port of https://www.shadertoy.com/view/slGSR3; author: rogo1965.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/slGSR3.json.
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
float stime, ctime;
void ry(Context &__ctx, f32x3 &p, float a) {
  float c, s;
  f32x3 q = p;
  c = cos(a);
  s = sin(a);
  p.data[0] = c * q.data[0] + s * q.data[2];
  p.data[2] = -s * q.data[0] + c * q.data[2];
}
float pixel_size = 0.0f;
f32x3 mb(Context &__ctx, f32x3 p) {
  p.set_swizzle<0, 1, 2>(p.swizzle<0, 2, 1>());
  f32x3 z = p;
  f32x3 dz = f32x3(0.0f);
  float power = 8.0f;
  float r, theta, phi;
  float dr = 1.0f;
  float t0 = 1.0f;
  for (int i = 0; i < 9; ++i) {
    r = length(z);
    if (r > 2.0f)
      continue;
    theta = atan(z.data[1] / z.data[0]);
    phi = asin(z.data[2] / r) + iTime * 0.2f;
    dr = pow(r, power - 1.0f) * dr * power + 1.0f;
    r = pow(r, power);
    theta = theta * power;
    phi = phi * power;
    z = r * f32x3(cos(theta) * cos(phi), sin(theta) * cos(phi), sin(phi)) + p;
    t0 = min(t0, r);
  }
  return f32x3(0.5f * log(r) * r / dr, t0, 0.0f);
}
f32x3 f(Context &__ctx, f32x3 p) {
  ry(__ctx, p, iTime * 0.2f);
  return mb(__ctx, p);
}
float softshadow(Context &__ctx, f32x3 ro, f32x3 rd, float k) {
  float akuma = 1.0f, h = 0.0f;
  float t = 0.01f;
  for (int i = 0; i < 50; ++i) {
    h = f(__ctx, ro + rd * t).data[0];
    if (h < 0.001f)
      return 0.02f;
    akuma = min(akuma, k * h / t);
    t += clamp(h, 0.01f, 2.0f);
  }
  return akuma;
}
f32x3 nor(Context &__ctx, f32x3 pos) {
  f32x3 eps = f32x3(0.001f, 0.0f, 0.0f);
  return normalize(f32x3(f(__ctx, pos + eps.swizzle<0, 1, 1>()).data[0] -
                             f(__ctx, pos - eps.swizzle<0, 1, 1>()).data[0],
                         f(__ctx, pos + eps.swizzle<1, 0, 1>()).data[0] -
                             f(__ctx, pos - eps.swizzle<1, 0, 1>()).data[0],
                         f(__ctx, pos + eps.swizzle<1, 1, 0>()).data[0] -
                             f(__ctx, pos - eps.swizzle<1, 1, 0>()).data[0]));
}
f32x3 intersect(Context &__ctx, f32x3 ro, f32x3 rd) {
  float t = 1.0f;
  float res_t = 0.0f;
  float res_d = 1000.0f;
  f32x3 c, res_c;
  float max_error = 1000.0f;
  float d = 1.0f;
  float pd = 100.0f;
  float os = 0.0f;
  float step = 0.0f;
  float error = 1000.0f;
  for (int i = 0; i < 96; i++) {
    if (error < pixel_size * 0.5f || t > 20.0f) {
    } else {
      c = f(__ctx, ro + rd * t);
      d = c.data[0];
      if (d > os) {
        os = 0.4f * d * d / pd;
        step = d + os;
        pd = d;
      } else {
        step = -os;
        os = 0.0f;
        pd = 100.0f;
        d = 1.0f;
      }
      error = d / t;
      if (error < max_error) {
        max_error = error;
        res_t = t;
        res_c = c;
      }
      t += step;
    }
  }
  if (t > 20.0f)
    res_t = -1.0f;
  return f32x3(res_t, res_c.data[1], res_c.data[2]);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 q = fragCoord.swizzle<0, 1>() / iResolution.swizzle<0, 1>();
  f32x2 uv = -1.0f + 2.0f * q;
  uv.data[0] *= iResolution.data[0] / iResolution.data[1];
  pixel_size = 1.0f / (iResolution.data[0] * 3.0f);
  stime = 0.7f + 0.3f * sin(iTime * 0.4f);
  ctime = 0.7f + 0.3f * cos(iTime * 0.4f);
  f32x3 ta = f32x3(0.0f, 0.0f, 0.0f);
  f32x3 ro = f32x3(0.01f, 2.2f, 0.01f);
  f32x3 cf = normalize(ta - ro);
  f32x3 cs = normalize(cross(cf, f32x3(0.0f, 1.0f, 0.0f)));
  f32x3 cu = normalize(cross(cs, cf));
  f32x3 rd = normalize(uv.data[0] * cs + uv.data[1] * cu + 5.0f * cf);
  f32x3 sundir = normalize(f32x3(0.1f, 0.8f, 0.6f));
  f32x3 sun = f32x3(1.64f, 1.27f, 0.2f);
  f32x3 skycolor = f32x3(0.6f, 1.5f, 1.0f);
  f32x3 bg = exp(uv.data[1] - 2.0f) * f32x3(0.4f, 1.6f, 1.0f);
  float halo =
      clamp(dot(normalize(f32x3(-ro.data[0], -ro.data[1], -ro.data[2])), rd),
            0.0f, 1.0f);
  f32x3 col = bg + f32x3(1.0f, 0.8f, 0.4f) * pow(halo, 17.0f);
  float t = 0.0f;
  f32x3 p = ro;
  f32x3 res = intersect(__ctx, ro, rd);
  if (res.data[0] > 0.0f) {
    p = ro + res.data[0] * rd;
    f32x3 n = nor(__ctx, p);
    float shadow = softshadow(__ctx, p, sundir, 10.0f);
    float dif = max(0.0f, dot(n, sundir));
    float sky = 0.6f + 0.4f * max(0.0f, dot(n, f32x3(0.0f, 1.0f, 0.0f)));
    float bac = max(
        0.3f + 0.7f * dot(f32x3(-sundir.data[0], -1.0f, -sundir.data[2]), n),
        0.0f);
    float spe =
        max(0.0f, pow(clamp(dot(sundir, reflect(rd, n)), 0.0f, 1.0f), 10.0f));
    f32x3 lin = 4.5f * sun * dif * shadow;
    lin += 0.8f * bac * sun;
    lin += 0.6f * sky * skycolor * shadow;
    lin += 3.0f * spe * shadow;
    res.data[1] = pow(clamp(res.data[1], 0.0f, 1.0f), 0.95f);
    f32x3 tc0 = 0.5f + 0.5f * cos(-6.0f * iTime + 9.2f * res.data[1] +
                                  f32x3(0.9f, 0.0f, 0.0f));
    col = lin * f32x3(0.9f, 0.8f, 0.6f) * 0.2f * tc0;
    col = mix(col, bg, 1.0f - exp(-0.001f * res.data[0] * res.data[0]));
  }
  col = pow(clamp(col, 0.0f, 1.0f), f32x3(0.45f));
  col = col * 0.6f + 0.4f * col * col * (3.0f - 2.0f * col);
  col = mix(col, f32x3(dot(col, f32x3(0.33f))), -0.5f);
  fragColor = f32x4(col.swizzle<0, 1, 2>(),
                    smoothstep(0.55f, .76f, 1.f - res.data[0] / 5.f));
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
