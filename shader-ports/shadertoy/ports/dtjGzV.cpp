// Port of https://www.shadertoy.com/view/dtjGzV; author: A_Toaster.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtjGzV.json.
#include "../support.h"
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
  Texture2D channel1;
};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const float fog_density = 0.75f;
const float fog_falloff = 4.f;
const float atmo_density = 0.001f;
const float atmo_falloff = 0.02f;
const f32x3 moon_dir = normalize(f32x3(0.3f, 0.5f, 1.f));
const f32x3 moon_col = f32x3(0.20f, 0.27f, 0.75f) * 0.1f;
const f32x3 orb_col = f32x3(1.f, 0.6f, 0.1f);
const float moon_size = 0.1f;
const f32x3 flat_albedo = f32x3(0.9f, 0.9f, 0.9f);
const f32x3 wall_albedo = f32x3(0.3f, 0.15f, 0.15f);
const f32x3 low_albedo = f32x3(0.7f, 0.7f, 0.7f);
const f32x3 pyr_albedo = f32x3(0.05f, 0.05f, 0.05f);
const f32x3 pyr_top_albedo = f32x3(1.f, 1.f, 0.5f);
f32x2 hash2(Context &__ctx, f32x2 p) {
  return textureLod(__ctx.channel0, (p + 0.5f) / 256.0f, 0.0f).swizzle<0, 1>();
}
float mountain(Context &__ctx, f32x2 x) {
  f32x2 n1 = texture(__ctx.channel0, x * 0.01f).swizzle<0, 1>();
  f32x2 n2 = texture(__ctx.channel0, (x + n1) * 0.005f).swizzle<0, 1>();
  float n3 = texture(__ctx.channel0, (x - n2 * 1.f) * 0.2f).data[2];
  float n4 = texture(__ctx.channel0, (x - n2 * 0.5f) * 0.4f).data[1];
  float n5 = texture(__ctx.channel0, x * 0.8f).data[0];
  x.data[1] = x.data[1] * 0.5f;
  x = x + n2 * 0.25f;
  f32x2 p = floor(x);
  f32x2 f = fract(x);
  f32x2 res = f32x2(8.0f);
  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++) {
      f32x2 b = f32x2(i, j);
      f32x2 r = f32x2(b) - f + hash2(__ctx, p + b);
      float d = dot(r, r);
      if (d < res.data[0]) {
        res.data[1] = res.data[0];
        res.data[0] = d;
      } else if (d < res.data[1]) {
        res.data[1] = d;
      }
    }
  float h = smoothstep(0.f, 1.75f, -res.data[0] + res.data[1]);
  h = h + (h * n3 * 0.05f) + n4 * 0.005f + n5 * 0.005f;
  return h;
}
float mountain_ring(Context &__ctx, f32x2 x) {
  x = x * 0.2f;
  float circle = smoothstep(2.f * 2.f, 10.f * 10.f, dot(x, x)) + 0.05f;
  float m = mountain(__ctx, x * 0.3f) * 1.0f;
  return m * circle * 5.f;
}
float sdPyramid(Context &__ctx, f32x3 p, float h) {
  float m2 = h * h + 0.25f;
  p.set_swizzle<0, 2>(abs(p.swizzle<0, 2>()));
  p.set_swizzle<0, 2>((p.data[2] > p.data[0]) ? p.swizzle<2, 0>()
                                              : p.swizzle<0, 2>());
  p.set_swizzle<0, 2>(p.swizzle<0, 2>() - (0.5f));
  f32x3 q = f32x3(p.data[2], h * p.data[1] - 0.5f * p.data[0],
                  h * p.data[0] + 0.5f * p.data[1]);
  float s = max(-q.data[0], 0.0f);
  float t = clamp((q.data[1] - 0.5f * p.data[2]) / (m2 + 0.25f), 0.0f, 1.0f);
  float a = m2 * (q.data[0] + s) * (q.data[0] + s) + q.data[1] * q.data[1];
  float b = m2 * (q.data[0] + 0.5f * t) * (q.data[0] + 0.5f * t) +
            (q.data[1] - m2 * t) * (q.data[1] - m2 * t);
  float d2 = min(q.data[1], -q.data[0] * m2 - q.data[1] * 0.5f) > 0.0f
                 ? 0.0f
                 : min(a, b);
  return sqrt((d2 + q.data[2] * q.data[2]) / m2) *
         sign(max(q.data[2], -p.data[1]));
}
float sdBox(Context &__ctx, f32x3 p, f32x3 b) {
  f32x3 q = abs(p) - b;
  return length(max(q, 0.0f)) +
         min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f);
}
float sdOctahedron(Context &__ctx, f32x3 p, float s) {
  p = abs(p);
  float m = (p.data[0] + p.data[1] + p.data[2] - s) / 3.0f;
  f32x3 o = p - m;
  f32x3 k = min(o, 0.0f);
  o = o + (k.data[0] + k.data[1] + k.data[2]) * 0.5f - k * 1.5f;
  o = clamp(o, 0.0f, s);
  return length(p - o) * sign(m);
}
float map(Context &__ctx, f32x3 p) {
  float mtn_dist = p.data[1] - mountain_ring(__ctx, p.swizzle<0, 2>());
  if (dot(p, p) < 20.f) {
    return min(min(p.data[1] - mountain_ring(__ctx, p.swizzle<0, 2>()),
                   min(sdOctahedron(__ctx,
                                    abs(p) * f32x3(1.f, 0.5f, 1.f) -
                                        f32x3(2.5f, 0.4f, 1.1f),
                                    0.15f),
                       sdOctahedron(__ctx,
                                    abs(p) * f32x3(1.f, 0.5f, 1.f) -
                                        f32x3(1.1f, 0.4f, 2.5f),
                                    0.15f))),
               min(sdPyramid(__ctx, p * 0.75f, 0.75f),
                   sdBox(__ctx, p, f32x3(0.8f, 0.05f, 0.8f))));
  }
  return mtn_dist;
}
bool mat_pyramid(Context &__ctx, f32x3 p) {
  if (dot(p, p) > 20.f)
    return false;
  float mtn_dist = p.data[1] - mountain_ring(__ctx, p.swizzle<0, 2>());
  float pyr_dist = min(
      min(sdOctahedron(__ctx,
                       abs(p) * f32x3(1.f, 0.5f, 1.f) - f32x3(2.5f, 0.4f, 1.1f),
                       0.15f),
          sdOctahedron(__ctx,
                       abs(p) * f32x3(1.f, 0.5f, 1.f) - f32x3(1.1f, 0.4f, 2.5f),
                       0.15f)),
      min(sdPyramid(__ctx, p * 0.75f, 0.75f),
          sdBox(__ctx, p, f32x3(0.8f, 0.05f, 0.8f))));
  return pyr_dist < mtn_dist;
}
void CamPolar(Context &__ctx, f32x3 &pos, f32x3 &ray, f32x3 origin,
              f32x2 rotation, float distance, float zoom, f32x2 fragCoord) {
  f32x2 c = f32x2(cos(rotation.data[0]), cos(rotation.data[1]));
  f32x4 s;
  s.set_swizzle<0, 1>(f32x2(sin(rotation.data[0]), sin(rotation.data[1])));
  s.set_swizzle<2, 3>(-s.swizzle<0, 1>());
  ray.set_swizzle<0, 1>(fragCoord.swizzle<0, 1>() -
                        iResolution.swizzle<0, 1>() * .5f);
  ray.data[2] = iResolution.data[1] * zoom;
  ray = normalize(ray);
  ray.set_swizzle<1, 2>(ray.swizzle<1, 2>() * c.swizzle<0, 0>() +
                        ray.swizzle<2, 1>() * s.swizzle<2, 0>());
  ray.set_swizzle<0, 2>(ray.swizzle<0, 2>() * c.swizzle<1, 1>() +
                        ray.swizzle<2, 0>() * s.swizzle<1, 3>());
  pos = origin - distance * f32x3(c.data[0] * s.data[1], s.data[2],
                                  c.data[0] * c.data[1]);
}
f32x3 calcNormal(Context &__ctx, f32x3 pos, float dist) {
  float h = max(0.01f, 0.001f * dist);
  f32x3 n = f32x3(0.0f);
  for (int i = (min(iFrame, 0)); i < 4; i++) {
    f32x3 e =
        0.5773f *
        (2.0f * f32x3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0f);
    n += e * map(__ctx, pos + e * h);
  }
  return normalize(n);
}
float softshadow(Context &__ctx, f32x3 ro, f32x3 rd, float mint, float k) {
  float res = 1.0f;
  float t = mint;
  float h = 1.0f;
  for (int i = 0; i < 32; i++) {
    h = map(__ctx, ro + rd * t);
    res = min(res, k * h / t);
    t += clamp(h, 0.01f, 1.f);
  }
  return clamp(res, 0.0f, 1.0f);
}
float softshadow_point(Context &__ctx, f32x3 ro, f32x3 rd, float mint, float k,
                       float max_dist) {
  float res = 1.0f;
  float t = mint;
  float h = 1.0f;
  for (int i = 0; i < 64; i++) {
    f32x3 p = ro + rd * t;
    h = map(__ctx, p);
    res = min(res, k * h / t);
    t = t + clamp(h, 0.02f + 0.5f * step(20.f, dot(p, p)), 2.f);
    if (t > max_dist) {
      break;
    }
  }
  return clamp(res, 0.0f, 1.0f);
}
float calcAO(Context &__ctx, f32x3 pos, f32x3 nor) {
  float occ = 0.0f;
  float sca = 1.0f;
  for (int i = 0; i < 5; i++) {
    float h = 0.01f + 0.12f * float(i) / 4.0f;
    float d = map(__ctx, pos + h * nor);
    occ += (h - d) * sca;
    sca *= 0.95f;
    if (occ > 0.35f)
      break;
  }
  return clamp(1.0f - occ, 0.0f, 1.0f);
}
bool intersect_precise(Context &__ctx, f32x3 ro, f32x3 rd, float &dist,
                       f32x3 &norm, int &i) {
  float h = 1.0f;
  dist = 0.0f;
  for (i = 0; i < 256; i++) {
    if (h < 0.00001f * dist) {
      norm = calcNormal(__ctx, ro + rd * dist, dist);
      return true;
    }
    h = map(__ctx, ro + rd * dist);
    dist += h;
    if (dist > 1500.f)
      return false;
  }
  norm = calcNormal(__ctx, ro + rd * dist, dist);
  return dot(norm, rd) < 0.2f;
}
f32x2 sphDistances(Context &__ctx, f32x3 ro, f32x3 rd, f32x3 ce, float ra) {
  f32x3 oc = ro - ce;
  float b = dot(oc, rd);
  float c = dot(oc, oc) - ra * ra;
  float h = b * b - c;
  if (h < 0.0f)
    return f32x2(-1.0f);
  h = sqrt(h);
  return f32x2(-b - h, -b + h);
}
f32x3 applyFog(Context &__ctx, f32x3 rgb, float dist, f32x3 rayOri,
               f32x3 rayDir) {
  float fogAmount =
      (fog_density / fog_falloff) * exp(-rayOri.data[1] * fog_falloff) *
      (1.0f - exp(-dist * rayDir.data[1] * fog_falloff)) / rayDir.data[1];
  float atmosAmount =
      (atmo_density / atmo_falloff) * exp(-rayOri.data[1] * atmo_falloff) *
      (1.0f - exp(-dist * rayDir.data[1] * atmo_falloff)) / rayDir.data[1];
  float amount = fogAmount + atmosAmount;
  return mix(moon_col, rgb, exp(-amount));
}
f32x3 sky(Context &__ctx, f32x3 rgb, f32x3 rayDir) {
  float yi = 1.f / clamp(rayDir.data[1], 0.01f, 1.f);
  f32x2 cloud_uv = rayDir.swizzle<0, 2>() * yi;
  float cloud =
      max(0.f, texture(__ctx.channel1, cloud_uv * f32x2(0.1f, 0.02f)).data[0] -
                   0.3f) *
      texture(__ctx.channel1, cloud_uv * f32x2(0.2f, 0.1f)).data[0] *
      rayDir.data[1] * 0.1f;
  float amount = (fog_density / fog_falloff + atmo_density / atmo_falloff) * yi;
  return mix(moon_col, rgb, exp(-amount)) + f32x3(1.f) * cloud;
}
float pow5(Context &__ctx, float x) {
  float x2 = x * x;
  return x2 * x2 * x;
}
float glare(Context &__ctx, f32x2 uv, f32x2 pos, float size,
            float origin_size) {
  float dist = length(pos - uv);
  float strength = clamp((size - dist) / size, 0.f, 1.f);
  strength = pow5(__ctx, strength);
  float angle = atan(uv.data[1] - pos.data[1], uv.data[0] - pos.data[0]);
  float noise =
      mix(0.5f, texture(__ctx.channel0, f32x2(0.f, angle * 0.07f)).data[0],
          dist / origin_size);
  return strength * noise;
}
f32x3 encodeSRGB(Context &__ctx, f32x3 linearRGB) {
  f32x3 a = 12.92f * linearRGB;
  f32x3 b = 1.055f * pow(linearRGB, f32x3(1.0f / 2.4f)) - 0.055f;
  f32x3 c = step(f32x3(0.0031308f), linearRGB);
  return mix(a, b, c);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv = (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) /
             iResolution.swizzle<0, 0>();
  f32x2 camRot = f32x2(.5f, .5f) +
                 f32x2(-.6f, 4.5f) *
                     (iMouse.swizzle<1, 0>() / iResolution.swizzle<1, 0>());
  f32x3 pos, ray;
  CamPolar(__ctx, pos, ray, f32x3(0.f, 1.2f, 0.f), camRot, 15.0f, 1.0f,
           fragCoord);
  float orb_seq = pow(fract(iTime * 0.08f), 2.f);
  float orb_radius = 0.02f / (orb_seq * orb_seq) + 0.01f;
  orb_radius += max(0.f, (orb_seq - 0.5f) * 100.f);
  float orb_intensity = max(0.f, 0.5f / (orb_radius * orb_radius) - 0.005f);
  if (orb_seq > 0.5f) {
    orb_intensity = 120.f * pow(1.f - smoothstep(0.3f, 1.f, orb_seq), 3.f);
  }
  f32x2 orb = sphDistances(
      __ctx, pos, ray, f32x3(0.f, 1.2f + 0.25f * orb_radius, 0.f), orb_radius);
  f32x2 orb2 =
      sphDistances(__ctx, pos, ray, f32x3(0.f, 1.2f + 0.25f * orb_radius, 0.f),
                   orb_radius - 0.05f);
  float orb_power = glare(__ctx, uv, f32x2(0.f), 0.75f, orb_radius * 20.f) *
                    orb_intensity * 0.1f;
  float point_light_r = max(20.f / orb_radius, 5.f);
  float point_light_height = 1.2f + orb_radius * 0.25f;
  float point_light_power = pow(orb_intensity * orb_radius * 10.f, 1.f);
  float dist;
  int i;
  f32x3 normal;
  bool hit = intersect_precise(__ctx, pos, ray, dist, normal, i);
  f32x3 col;
  if (hit) {
    f32x3 hp = pos + ray * dist;
    f32x3 albedo;
    float spec;
    if (mat_pyramid(__ctx, hp)) {
      float tip = smoothstep(0.8f, 0.805f, hp.data[1]);
      spec = tip * 0.9f;
      albedo = mix(pyr_albedo, pyr_top_albedo, tip);
    } else {
      float wall_amount =
          smoothstep(0.7f, 0.9f, dot(normal, f32x3(0.f, 1.f, 0.f)));
      float low_amount = smoothstep(0.0f, 0.02f, hp.data[1]);
      albedo = mix(wall_albedo, mix(low_albedo, flat_albedo, low_amount),
                   wall_amount);
      spec = mix(0.4f, 0.0f, wall_amount);
    }
    float ao = calcAO(__ctx, hp, normal);
    float shadow = softshadow(__ctx, hp, moon_dir, 0.01f, 20.f);
    float diffuse = smoothstep(-1.f, 1.f, dot(normal, moon_dir));
    float spec_brightness = pow5(__ctx, dot(reflect(ray, normal), moon_dir));
    f32x3 moonlit_col =
        shadow * ao *
        mix(albedo * moon_col * diffuse, moon_col * spec_brightness, spec);
    f32x3 pointlit_col = f32x3(0.f);
    f32x3 point_light_col = orb_col * point_light_power;
    if (point_light_power > 0.f) {
      f32x3 point_light_ro = f32x3(0.f, point_light_height, 0.f);
      f32x3 point_light_dir = point_light_ro - hp;
      float point_light_dist = length(point_light_dir);
      float distance_atten = 1.f / (point_light_dist);
      point_light_dir = point_light_dir / point_light_dist;
      float point_shadow =
          softshadow_point(__ctx, hp, point_light_dir, 0.01f, point_light_r,
                           point_light_dist - 1.f);
      float pt_diffuse = smoothstep(-1.f, 1.f, dot(normal, point_light_dir));
      float pt_spec_brightness =
          pow5(__ctx, dot(reflect(ray, normal), point_light_dir));
      pointlit_col = distance_atten * point_shadow *
                     mix(albedo * point_light_col * pt_diffuse,
                         point_light_col * pt_spec_brightness, spec);
    }
    float orb_occ =
        (smoothstep(orb.data[0], orb.data[0] + 0.3f * orb_radius, dist) *
             smoothstep(0.f, 2.f, orb.data[0]) +
         smoothstep(orb.data[1], orb.data[1] + 0.3f * orb_radius, dist)) *
        0.5f;
    orb_power +=
        max(0.f, orb.data[1] - orb.data[0] - orb2.data[1] + orb2.data[0]) *
        orb_intensity * 10.f * orb_occ;
    col = moonlit_col + pointlit_col;
    col = applyFog(__ctx, col, dist, pos, ray);
  } else {
    col = sky(__ctx, f32x3(0.f), ray);
    orb_power +=
        max(0.f, orb.data[1] - orb.data[0] - orb2.data[1] + orb2.data[0]) *
        orb_intensity * 10.f;
  }
  col += orb_col * orb_power;
  fragColor = f32x4(encodeSRGB(__ctx, col), 1.0f);
}

__fragment__
void shadertoy(__push_constant__ const Params *p, __output__(0) f32x4 *output,
               __binding__(0, 0) texture_2d<f32x4> channel0,
               __binding__(1, 0) sampler sampling0,
               __binding__(0, 1) texture_2d<f32x4> channel1,
               __binding__(1, 1) sampler sampling1) {
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
  Context __ctx{{channel0, sampling0}, {channel1, sampling1}};
  f32x4 color;
  mainImage(__ctx, color, f32x2(frag_coord[0], frag_coord[1]));
  *output = color;
}
