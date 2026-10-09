// Port of https://www.shadertoy.com/view/7tlfR4; author: Fraktoler.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/7tlfR4.json.
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
float mag(Context &__ctx, f32x2 z) { return length(z); }
float mag2(Context &__ctx, f32x2 z) { return dot(z, z); }
float arg(Context &__ctx, f32x2 z) { return atan(z.data[1], z.data[0]); }
f32x2 conj(Context &__ctx, f32x2 z) { return f32x2(z.data[0], -z.data[1]); }
f32x2 flip(Context &__ctx, f32x2 z) { return f32x2(-z.data[0], z.data[1]); }
f32x2 swap(Context &__ctx, f32x2 z) { return f32x2(z.data[1], z.data[0]); }
f32x2 rabs(Context &__ctx, f32x2 z) { return f32x2(abs(z.data[0]), z.data[1]); }
f32x2 iabs(Context &__ctx, f32x2 z) { return f32x2(z.data[0], abs(z.data[1])); }
f32x2 cis(Context &__ctx, float t) { return f32x2(cos(t), sin(t)); }
f32x2 mul(Context &__ctx, f32x2 z, f32x2 w) {
  return z.data[0] * w + z.data[1] * f32x2(-w.data[1], w.data[0]);
}
f32x2 mulI(Context &__ctx, f32x2 z) { return f32x2(-z.data[1], z.data[0]); }
f32x2 div(Context &__ctx, f32x2 z, f32x2 w) {
  return (w.data[0] * z + w.data[1] * f32x2(z.data[1], -z.data[0])) /
         mag2(__ctx, w);
}
f32x2 rec(Context &__ctx, f32x2 z) { return conj(__ctx, z) / mag2(__ctx, z); }
f32x2 squ(Context &__ctx, f32x2 z) {
  return f32x2(z.data[0] * z.data[0] - z.data[1] * z.data[1],
               2.0f * z.data[0] * z.data[1]);
}
f32x2 cube(Context &__ctx, f32x2 z) {
  return z * f32x2(z.data[0] * z.data[0] - 3.0f * z.data[1] * z.data[1],
                   3.0f * z.data[0] * z.data[0] - z.data[1] * z.data[1]);
}
f32x2 power(Context &__ctx, f32x2 z, float p) {
  return pow(mag2(__ctx, z), p * 0.5f) * cis(__ctx, p * arg(__ctx, z));
}
f32x2 powervec2(Context &__ctx, f32x2 z, f32x2 w) {
  float r = mag2(__ctx, z);
  float argz = arg(__ctx, z);
  float argzw = w.data[0] * argz + 0.5f * w.data[1] * log(r);
  return pow(r, w.data[0] * 0.5f) * exp(-w.data[1] * argz) * cis(__ctx, argzw);
}
f32x2 sqrtp(Context &__ctx, f32x2 z) {
  float r = mag(__ctx, z);
  return f32x2(1.0f, sign(z.data[1])) *
         sqrt(0.5f * f32x2(r + z.data[0], r - z.data[0]));
}
f32x2 powexp(Context &__ctx, f32x2 z) {
  return exp(z.data[0]) * cis(__ctx, z.data[1]);
}
f32x2 ln(Context &__ctx, f32x2 z) {
  return f32x2(log(mag2(__ctx, z)) * 0.5f, arg(__ctx, z));
}
f32x2 logb(Context &__ctx, f32x2 z, f32x2 b) {
  return div(__ctx, ln(__ctx, z), ln(__ctx, b));
}
f32x2 sine(Context &__ctx, f32x2 z) {
  return f32x2(sin(z.data[0]) * cosh(z.data[1]),
               cos(z.data[0]) * sinh(z.data[1]));
}
f32x2 cosine(Context &__ctx, f32x2 z) {
  return f32x2(cos(z.data[0]) * cosh(z.data[1]),
               -sin(z.data[0]) * sinh(z.data[1]));
}
f32x2 sub_tan(Context &__ctx, f32x2 z) {
  f32x2 squ_exp = powexp(__ctx, mulI(__ctx, z + z));
  return div(__ctx, squ_exp - f32x2(1.0f, 0.0f),
             mulI(__ctx, squ_exp + f32x2(1.0f, 0.0f)));
}
f32x2 tangent(Context &__ctx, f32x2 z) {
  if (z.data[1] < -21.0f) {
    return conj(__ctx, sub_tan(__ctx, conj(__ctx, z)));
  } else {
    return sub_tan(__ctx, z);
  }
}
f32x2 sub_cot(Context &__ctx, f32x2 z) {
  f32x2 squ_exp = powexp(__ctx, mulI(__ctx, z + z));
  return div(__ctx, mulI(__ctx, squ_exp + f32x2(1.0f, 0.0f)),
             squ_exp - f32x2(1.0f, 0.0f));
}
f32x2 cot(Context &__ctx, f32x2 z) {
  if (z.data[1] < -21.0f) {
    return conj(__ctx, sub_cot(__ctx, conj(__ctx, z)));
  } else {
    return sub_cot(__ctx, z);
  }
}
f32x2 sec(Context &__ctx, f32x2 z) { return rec(__ctx, cosine(__ctx, z)); }
f32x2 csc(Context &__ctx, f32x2 z) { return rec(__ctx, sine(__ctx, z)); }
f32x2 arcsin(Context &__ctx, f32x2 z) {
  return mulI(__ctx, ln(__ctx, sqrtp(__ctx, f32x2(1.0f, 0) - squ(__ctx, z)) -
                                   mulI(__ctx, z)));
}
f32x2 arccos(Context &__ctx, f32x2 z) {
  return -mulI(
      __ctx,
      ln(__ctx, mulI(__ctx, sqrtp(__ctx, f32x2(1.0f, 0) - squ(__ctx, z))) + z));
}
f32x2 arctan(Context &__ctx, f32x2 z) {
  return -0.5f * mulI(__ctx, ln(__ctx, div(__ctx, f32x2(0.0f, 1.0f) - z,
                                           f32x2(0.0f, 1.0f) + z)));
}
f32x2 arccot(Context &__ctx, f32x2 z) {
  return -0.5f * mulI(__ctx, ln(__ctx, div(__ctx, z + f32x2(0.0f, 1.0f),
                                           z - f32x2(0.0f, 1.0f))));
}
f32x2 arcsec(Context &__ctx, f32x2 z) { return arccos(__ctx, rec(__ctx, z)); }
f32x2 arccsc(Context &__ctx, f32x2 z) { return arcsin(__ctx, rec(__ctx, z)); }
f32x2 sinhyp(Context &__ctx, f32x2 z) {
  return f32x2(sinh(z.data[0]) * cos(z.data[1]),
               cosh(z.data[0]) * sin(z.data[1]));
}
f32x2 coshyp(Context &__ctx, f32x2 z) {
  return f32x2(cosh(z.data[0]) * cos(z.data[1]),
               sinh(z.data[0]) * sin(z.data[1]));
}
f32x2 sub_tanh(Context &__ctx, f32x2 z) {
  f32x2 squ_exp = powexp(__ctx, z + z);
  return div(__ctx, squ_exp - f32x2(1.0f, 0.0f), squ_exp + f32x2(1.0f, 0.0f));
}
f32x2 tanhyp(Context &__ctx, f32x2 z) {
  if (z.data[0] > 21.0f) {
    return flip(__ctx, sub_tanh(__ctx, flip(__ctx, z)));
  } else {
    return sub_tanh(__ctx, z);
  }
}
f32x2 sub_coth(Context &__ctx, f32x2 z) {
  f32x2 squ_exp = powexp(__ctx, z + z);
  return div(__ctx, squ_exp + f32x2(1.0f, 0.0f), squ_exp - f32x2(1.0f, 0.0f));
}
f32x2 coth(Context &__ctx, f32x2 z) {
  if (z.data[0] > 21.0f) {
    return flip(__ctx, sub_coth(__ctx, flip(__ctx, z)));
  } else {
    return sub_coth(__ctx, z);
  }
}
f32x2 sech(Context &__ctx, f32x2 z) { return rec(__ctx, coshyp(__ctx, z)); }
f32x2 csch(Context &__ctx, f32x2 z) { return rec(__ctx, sinhyp(__ctx, z)); }
f32x2 arcsinh(Context &__ctx, f32x2 z) {
  return ln(__ctx, z + sqrtp(__ctx, squ(__ctx, z) + f32x2(1.0f, 0.0f)));
}
f32x2 arccosh(Context &__ctx, f32x2 z) {
  return ln(__ctx, z + mul(__ctx, sqrtp(__ctx, z + f32x2(1.0f, 0.0f)),
                           sqrtp(__ctx, z - f32x2(1.0f, 0.0f))));
}
f32x2 arctanh(Context &__ctx, f32x2 z) {
  return 0.5f *
         ln(__ctx, div(__ctx, f32x2(1.0f, 0.0f) + z, f32x2(1.0f, 0.0f) - z));
}
f32x2 acoth(Context &__ctx, f32x2 z) {
  return 0.5f *
         ln(__ctx, div(__ctx, z + f32x2(1.0f, 0.0f), z - f32x2(1.0f, 0.0f)));
}
f32x2 asech(Context &__ctx, f32x2 z) { return arccosh(__ctx, rec(__ctx, z)); }
f32x2 acsch(Context &__ctx, f32x2 z) { return arcsinh(__ctx, rec(__ctx, z)); }
f32x2 linear(Context &__ctx, f32x2 z, f32x2 a, f32x2 b) {
  return mul(__ctx, z, a) + b;
}
f32x2 quadratic(Context &__ctx, f32x2 z, f32x2 a, f32x2 b, f32x2 c) {
  return mul(__ctx, squ(__ctx, z), a) + mul(__ctx, z, b) + c;
}
f32x2 cubic(Context &__ctx, f32x2 z, f32x2 a, f32x2 b, f32x2 c, f32x2 d) {
  return mul(__ctx, cube(__ctx, z), a) + mul(__ctx, squ(__ctx, z), b) +
         mul(__ctx, z, c) + d;
}
f32x2 linearroot(Context &__ctx, f32x2 z, f32x2 a, f32x2 b) {
  return div(__ctx, -b, a);
}
f32x4 quadraticroots(Context &__ctx, f32x2 a, f32x2 b, f32x2 c) {
  f32x2 sqrtdelta = sqrtp(__ctx, squ(__ctx, b) - 4.0f * mul(__ctx, a, c));
  f32x2 twoa = a + a;
  return f32x4(div(__ctx, sqrtdelta - b, twoa),
               -div(__ctx, b + sqrtdelta, twoa));
}

f32x3 HSLtoRGB(Context &__ctx, f32x3 hsl) {
  float chroma = hsl.data[1] * (1.0f - abs(2.0f * hsl.data[2] - 1.0f));
  float h_prime = hsl.data[0] / 60.0f;
  float x = chroma * (1.0f - abs(mod(h_prime, 2.0f) - 1.0f));
  f32x3 rgb;
  if (0.0f <= h_prime && h_prime < 1.0f) {
    rgb = f32x3(chroma, x, 0.0f);
  } else if (1.0f <= h_prime && h_prime < 2.0f) {
    rgb = f32x3(x, chroma, 0.0f);
  } else if (2.0f <= h_prime && h_prime < 3.0f) {
    rgb = f32x3(0.0f, chroma, x);
  } else if (3.0f <= h_prime && h_prime < 4.0f) {
    rgb = f32x3(0.0f, x, chroma);
  } else if (4.0f <= h_prime && h_prime < 5.0f) {
    rgb = f32x3(x, 0.0f, chroma);
  } else {
    rgb = f32x3(chroma, 0.0f, x);
  }
  float m = hsl.data[2] - 0.5f * chroma;
  return rgb + m;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  float zoom = 3.0f;
  f32x2 center = f32x2(0.0f);
  f32x2 z = (2.0f * fragCoord.swizzle<0, 1>() - iResolution.swizzle<0, 1>()) /
                iResolution.data[0] * zoom +
            center;
  float exponent = 2.0f + iTime * 0.2f;
  float iter = 0.0f;
  const float max_iter = 256.0f;
  while (iter < max_iter) {
    f32x2 oldz = z;
    f32x2 f = power(__ctx, z, exponent) - f32x2(1.0f, 0.0f);
    f32x2 df = exponent * power(__ctx, z, exponent - 1.0f);
    z = z - div(__ctx, f, df);
    float r = mag2(__ctx, z - oldz);
    if (r < 1.0e-6f) {
      break;
    }
    iter += 1.0f;
  }
  if (iter >= max_iter) {
    fragColor = f32x4(f32x3(0.0f), 1.0f);
  } else {
    float h = arg(__ctx, z) / 3.1415926535897932384626433832795028841f;
    if (h < 0.0f) {
      h = h + 2.0f;
    }
    f32x3 col = HSLtoRGB(__ctx, f32x3(h * 180.0f, 1.0f, 0.5f));
    fragColor = f32x4(col, 1.0f);
  }
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
