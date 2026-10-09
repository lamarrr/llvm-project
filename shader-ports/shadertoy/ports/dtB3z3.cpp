// Port of https://www.shadertoy.com/view/dtB3z3; author: FifthStateOfMatter.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/dtB3z3.json.
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
struct Context {};
f32x3 iResolution;
f32x4 iMouse, iDate, gl_FragCoord;
float iTime, iTimeDelta, iFrameRate, iSampleRate;
int iFrame;
float iChannelTime[4];
f32x3 iChannelResolution[4];
const float eyeDist = 0.5f;
const bool delag = false;
const f32x3 fogColor = f32x3(0.8f);
const float reflectiveness = 0.3f;
float random3(Context &__ctx, f32x3 p) {
  p = fract(p * 0.1031f);
  p += dot(p, p.swizzle<2, 1, 0>() + 31.32f);
  return (fract((p.data[0] + p.data[1]) * p.data[2]) * 2.0f) - 1.0f;
}
float perlinNoise(Context &__ctx, f32x3 p) {
  f32x3 cellPos = floor(p);
  f32x3 cellFract = fract(p);
  f32x3 cellMix = cellFract * cellFract * (3.0f - 2.0f * cellFract);
  float value;
  f32x3 blf = f32x3(random3(__ctx, (cellPos + f32x3(0, 0, 0)) * 1.0f));
  f32x3 brf = f32x3(random3(__ctx, (cellPos + f32x3(1, 0, 0)) * 1.0f));
  f32x3 trf = f32x3(random3(__ctx, (cellPos + f32x3(1, 1, 0)) * 1.0f));
  f32x3 tlf = f32x3(random3(__ctx, (cellPos + f32x3(0, 1, 0)) * 1.0f));
  f32x3 blb = f32x3(random3(__ctx, (cellPos + f32x3(0, 0, 1)) * 1.0f));
  f32x3 brb = f32x3(random3(__ctx, (cellPos + f32x3(1, 0, 1)) * 1.0f));
  f32x3 trb = f32x3(random3(__ctx, (cellPos + f32x3(1, 1, 1)) * 1.0f));
  f32x3 tlb = f32x3(random3(__ctx, (cellPos + f32x3(0, 1, 1)) * 1.0f));
  value = mix(mix(mix(dot(cellFract - f32x3(0, 0, 0), blf),
                      dot(cellFract - f32x3(1, 0, 0), brf), cellMix.data[0]),
                  mix(dot(cellFract - f32x3(0, 1, 0), tlf),
                      dot(cellFract - f32x3(1, 1, 0), trf), cellMix.data[0]),
                  cellMix.data[1]),
              mix(mix(dot(cellFract - f32x3(0, 0, 1), blb),
                      dot(cellFract - f32x3(1, 0, 1), brb), cellMix.data[0]),
                  mix(dot(cellFract - f32x3(0, 1, 1), tlb),
                      dot(cellFract - f32x3(1, 1, 1), trb), cellMix.data[0]),
                  cellMix.data[1]),
              cellMix.data[2]);
  return value;
}
struct material {
  f32x3 col;
  int type;
};
struct SDF {
  float dist;
  material mat;
};
SDF sphereSDF(Context &__ctx, f32x3 p, float r, material mat) {
  return SDF(length(p) - r, mat);
}
SDF boxSDF(Context &__ctx, f32x3 p, f32x3 s, material mat) {
  f32x3 q = abs(p) - s;
  return SDF(length(max(q, 0.0f)) +
                 min(max(q.data[0], max(q.data[1], q.data[2])), 0.0f),
             mat);
}
SDF infiniteSpherePatternSDF(Context &__ctx, f32x3 p, f32x3 c, material mat) {
  f32x3 q = mod(p + 0.5f * c, c) - 0.5f * c;
  return sphereSDF(__ctx, q, 0.25f, mat);
}
SDF infiniteBoxPatternSDF(Context &__ctx, f32x3 p, f32x3 c, material mat) {
  f32x3 q = mod(p + 0.5f * c, c) - 0.5f * c;
  return boxSDF(__ctx, q, f32x3(0.25f), mat);
}
SDF yPlaneSDF(Context &__ctx, f32x3 p, float y, material mat) {
  return SDF(p.data[1] - y, mat);
}
SDF cylinderSDF(Context &__ctx, f32x3 p, float h, float r, material mat) {
  f32x2 d = abs(f32x2(length(p.swizzle<0, 2>()), p.data[1])) - f32x2(r, h);
  return SDF(min(max(d.data[0], d.data[1]), 0.0f) + length(max(d, 0.0f)), mat);
}
SDF finiteCylinderPatternSDF(Context &__ctx, f32x3 p, float c, f32x3 l,
                             material mat) {
  f32x3 q = p - c * clamp(round(p / c), -l, l);
  return cylinderSDF(__ctx, q, 2.0f, 0.4f, mat);
}
SDF triPrismSDF(Context &__ctx, f32x3 p, f32x3 s, material mat) {
  p.data[0] = abs(p.data[0]);
  p.set_swizzle<0, 1>(p.swizzle<0, 1>() - (f32x2(s.data[0], -s.data[1])));
  f32x2 e = f32x2(-s.data[0], s.data[1] * 2.0f);
  f32x2 se = p.swizzle<0, 1>() -
             e * clamp(dot(p.swizzle<0, 1>(), e) / dot(e, e), 0.0f, 1.0f);
  float d1 = length(se);
  if (max(se.data[0], se.data[1]) < 0.0f) {
    d1 = -min(d1, p.data[1]);
  }
  float d2 = abs(p.data[2]) - s.data[2];
  return SDF(length(max(f32x2(d1, d2), 0.0f)) + min(max(d1, d2), 0.0f), mat);
}
SDF sceneSDF(Context &__ctx, f32x3 p) {
  float xr = 0.0f;
  float yr = iTime * 0.3f + (3.1415926535f / 4.0f) * 1.0f;
  float sxr = sin(xr);
  float syr = sin(yr);
  float cxr = cos(xr);
  float cyr = cos(yr);
  f32x3x3 xRot;
  xRot[0] = f32x3(1, 0, 0);
  xRot[1] = f32x3(0, cxr, -sxr);
  xRot[2] = f32x3(0, sxr, cxr);
  f32x3x3 yRot;
  yRot[0] = f32x3(cyr, 0, -syr);
  yRot[1] = f32x3(0, 1, 0);
  yRot[2] = f32x3(syr, 0, cyr);
  SDF pillars =
      finiteCylinderPatternSDF(__ctx, (p - f32x3(0, 0, 10)) * yRot, 2.0f,
                               f32x3(2, 0, 2), material(f32x3(0.6f), 0));
  SDF base1 = boxSDF(__ctx, (p - f32x3(0, -1.0f, 10)) * yRot,
                     f32x3(4.5f, 0.25f, 4.5f), material(f32x3(0.6f), 0));
  SDF base2 = boxSDF(__ctx, (p - f32x3(0, -1.55f, 10)) * yRot,
                     f32x3(5, 0.5f, 5), material(f32x3(0.6f), 0));
  SDF roof1 = boxSDF(__ctx, (p - f32x3(0, 2.0f, 10)) * yRot,
                     f32x3(4.75f, 0.25f, 4.75f), material(f32x3(0.6f), 0));
  SDF roof2 = triPrismSDF(__ctx, (p - f32x3(0, 3.25f, 10)) * yRot,
                          f32x3(4.75f, 1.0f, 4.75f), material(f32x3(0.6f), 0));
  SDF inter = boxSDF(__ctx, (p - f32x3(0, 0, 10)) * yRot,
                     f32x3(4.4f, 2.0f, 2.75f), material(f32x3(0.6f), 0));
  float closest =
      min(pillars.dist,
          min(base1.dist,
              min(base2.dist, min(roof1.dist, min(roof2.dist, inter.dist)))));
  if (closest == base1.dist) {
    return base1;
  } else if (closest == base2.dist) {
    return base2;
  } else if (closest == pillars.dist) {
    return pillars;
  } else if (closest == roof1.dist) {
    return roof1;
  } else if (closest == roof2.dist) {
    return roof2;
  } else {
    return inter;
  }
}
bool raymarch(Context &__ctx, f32x3 o, f32x3 d, float &t, material &mat, int ms,
              float eps) {
  t = 0.0f;
  for (int i = 0; i < ms && t < 30.0f; i++) {
    SDF s = sceneSDF(__ctx, o + d * t);
    t += s.dist;
    if (s.dist < eps && t >= 0.0f) {
      t -= s.dist;
      mat = s.mat;
      return true;
    }
  }
  return false;
}
f32x3 getNormal(Context &__ctx, f32x3 h) {
  return normalize(f32x3(
      sceneSDF(__ctx, f32x3(h.data[0] + 0.01f, h.swizzle<1, 2>())).dist -
          sceneSDF(__ctx, f32x3(h.data[0] - 0.01f, h.swizzle<1, 2>())).dist,
      sceneSDF(__ctx, f32x3(h.data[0], h.data[1] + 0.01f, h.data[2])).dist -
          sceneSDF(__ctx, f32x3(h.data[0], h.data[1] - 0.01f, h.data[2])).dist,
      sceneSDF(__ctx, f32x3(h.swizzle<0, 1>(), h.data[2] + 0.01f)).dist -
          sceneSDF(__ctx, f32x3(h.swizzle<0, 1>(), h.data[2] - 0.01f)).dist));
}
float getLighting(Context &__ctx, f32x3 h, f32x3 lpos) {
  f32x3 n = getNormal(__ctx, h);
  f32x3 lRay = normalize(lpos - h);
  float c = dot(n, lRay);
  float lt;
  material mat;
  bool rl = raymarch(__ctx, h + n * 0.01f, lRay, lt, mat, 30, 0.001f);
  if (rl) {
    c -= 0.4f;
  } else {
    c = dot(n, lRay);
  }
  return c;
}
f32x3 reflectRay(Context &__ctx, f32x3 h, f32x3 d) {
  f32x3 n = getNormal(__ctx, h);
  return d - 2.0f * n * dot(n, d);
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x3 col = f32x3(0);
  f32x2 uv =
      (fragCoord - 0.5f * iResolution.swizzle<0, 1>()) / iResolution.data[1];
  f32x3 lPos = f32x3(10, 10, -10.0f + iTime * 0.0f);
  f32x3 o = f32x3(0, 0.6f, iTime * 0.0f - 3.0f);
  f32x3 d = normalize(f32x3(uv, 1));
  bool red = mod(fragCoord.swizzle<0, 1>().data[0], 2.0f) > 0.5f;
  mod(fragCoord.swizzle<0, 1>().data[1], 2.0f) > 0.5f ? red = !red : red = red;
  if (delag) {
    if (red) {
      float tl;
      material matl;
      f32x3 ol = o - f32x3(eyeDist / 2.0f, 0, 0);
      bool rl = raymarch(__ctx, ol, d, tl, matl, 100, 0.01f);
      if (rl) {
        float cl = getLighting(__ctx, ol + d * tl, lPos);
        if (matl.type != 1) {
          float rtl;
          material rmatl;
          bool rrl = raymarch(
              __ctx, ol + d * tl + getNormal(__ctx, ol + d * tl) * 0.01f,
              normalize(reflectRay(__ctx, ol + d * tl, d)), rtl, rmatl, 30,
              0.01f);
          if (rrl) {
            float rcl = getLighting(
                __ctx,
                ol + d * tl + getNormal(__ctx, ol + d * tl) * 0.01f +
                    normalize(reflectRay(__ctx, ol + d * tl, d)) * rtl,
                lPos);
            col.data[0] +=
                mix(mix(matl.col.data[0], rmatl.col.data[0] * max(rcl, 0.1f),
                        reflectiveness) *
                        max(cl, 0.1f),
                    fogColor.data[0], min(tl / 20.0f, 1.0f));
          } else {
            col.data[0] +=
                mix(mix(matl.col.data[0], 1.0f, reflectiveness) * max(cl, 0.1f),
                    fogColor.data[0], min(tl / 20.0f, 1.0f));
          }
        } else {
          col.data[0] += mix(matl.col.data[0] * max(cl, 0.1f), fogColor.data[0],
                             min(tl / 20.0f, 1.0f));
        }
      } else {
        col.data[0] = fogColor.data[0];
      }
    } else {
      float tr;
      material matr;
      f32x3 or = o + f32x3(eyeDist / 2.0f, 0, 0);
      bool rr = raymarch(__ctx, or, d, tr, matr, 100, 0.01f);
      if (rr) {
        float cr = getLighting(__ctx, or +d * tr, lPos);
        if (matr.type != 1) {
          float rtr;
          material rmatr;
          bool rrr =
              raymarch(__ctx, or +d * tr + getNormal(__ctx, or +d * tr) * 0.01f,
                       normalize(reflectRay(__ctx, or +d * tr, d)), rtr, rmatr,
                       30, 0.01f);
          if (rrr) {
            float rcr = getLighting(
                __ctx,
                or +d * tr + getNormal(__ctx, or +d * tr) * 0.01f +
                        normalize(reflectRay(__ctx, or +d * tr, d)) * rtr,
                lPos);
            col.set_swizzle<1, 2>(
                col.swizzle<1, 2>() +
                (mix(mix(matr.col.swizzle<1, 2>(),
                         rmatr.col.swizzle<1, 2>() * max(rcr, 0.1f),
                         reflectiveness) *
                         max(cr, 0.1f),
                     fogColor.swizzle<1, 2>(), min(tr / 20.0f, 1.0f))));
          } else {
            col.set_swizzle<1, 2>(
                col.swizzle<1, 2>() +
                (mix(mix(matr.col.swizzle<1, 2>(), fogColor.swizzle<1, 2>(),
                         reflectiveness) *
                         max(cr, 0.1f),
                     fogColor.swizzle<1, 2>(), min(tr / 20.0f, 1.0f))));
          }
        } else {
          col.set_swizzle<1, 2>(
              col.swizzle<1, 2>() +
              (mix(matr.col.swizzle<1, 2>() * max(cr, 0.1f),
                   fogColor.swizzle<1, 2>(), min(tr / 20.0f, 1.0f))));
        }
      } else {
        col.set_swizzle<1, 2>(fogColor.swizzle<1, 2>());
      }
    }
  } else {
    float tl;
    material matl;
    f32x3 ol = o - f32x3(eyeDist / 2.0f, 0, 0);
    bool rl = raymarch(__ctx, ol, d, tl, matl, 100, 0.01f);
    if (rl) {
      float cl = getLighting(__ctx, ol + d * tl, lPos);
      if (matl.type != 1) {
        float rtl;
        material rmatl;
        bool rrl =
            raymarch(__ctx, ol + d * tl + getNormal(__ctx, ol + d * tl) * 0.01f,
                     normalize(reflectRay(__ctx, ol + d * tl, d)), rtl, rmatl,
                     50, 0.01f);
        if (rrl) {
          float rcl = getLighting(
              __ctx,
              ol + d * tl + getNormal(__ctx, ol + d * tl) * 0.01f +
                  normalize(reflectRay(__ctx, ol + d * tl, d)) * rtl,
              lPos);
          col.data[0] +=
              mix(mix(matl.col.data[0], rmatl.col.data[0] * max(rcl, 0.1f),
                      reflectiveness) *
                      max(cl, 0.1f),
                  fogColor.data[0], min(tl / 20.0f, 1.0f));
        } else {
          col.data[0] +=
              mix(mix(matl.col.data[0], fogColor.data[0], reflectiveness) *
                      max(cl, 0.1f),
                  fogColor.data[0], min(tl / 20.0f, 1.0f));
        }
      } else {
        col.data[0] += mix(matl.col.data[0] * max(cl, 0.1f), fogColor.data[0],
                           min(tl / 20.0f, 1.0f));
      }
    } else {
      col.data[0] = fogColor.data[0];
    }
    float tr;
    material matr;
    f32x3 or = o + f32x3(eyeDist / 2.0f, 0, 0);
    bool rr = raymarch(__ctx, or, d, tr, matr, 100, 0.01f);
    if (rr) {
      float cr = getLighting(__ctx, or +d * tr, lPos);
      if (matr.type != 1) {
        float rtr;
        material rmatr;
        bool rrr = raymarch(
            __ctx, or +d * tr + getNormal(__ctx, or +d * tr) * 0.01f,
            normalize(reflectRay(__ctx, or +d * tr, d)), rtr, rmatr, 50, 0.01f);
        if (rrr) {
          float rcr = getLighting(
              __ctx, or +d * tr + getNormal(__ctx, or +d * tr) * 0.01f +
                             normalize(reflectRay(__ctx, or +d * tr, d)) * rtr,
              lPos);
          col.set_swizzle<1, 2>(
              col.swizzle<1, 2>() +
              (mix(mix(matr.col.swizzle<1, 2>(),
                       rmatr.col.swizzle<1, 2>() * max(rcr, 0.1f),
                       reflectiveness) *
                       max(cr, 0.1f),
                   fogColor.swizzle<1, 2>(), min(tr / 20.0f, 1.0f))));
        } else {
          col.set_swizzle<1, 2>(
              col.swizzle<1, 2>() +
              (mix(mix(matr.col.swizzle<1, 2>(), fogColor.swizzle<1, 2>(),
                       reflectiveness) *
                       max(cr, 0.1f),
                   fogColor.swizzle<1, 2>(), min(tr / 20.0f, 1.0f))));
        }
      } else {
        col.set_swizzle<1, 2>(
            col.swizzle<1, 2>() +
            (mix(matr.col.swizzle<1, 2>() * max(cr, 0.1f),
                 fogColor.swizzle<1, 2>(), min(tr / 20.0f, 1.0f))));
      }
    } else {
      col.set_swizzle<1, 2>(fogColor.swizzle<1, 2>());
    }
  }
  fragColor = f32x4(pow(col, f32x3(1.0f / 2.2f)), 1);
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
