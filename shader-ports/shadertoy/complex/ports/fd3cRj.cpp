// Port of https://www.shadertoy.com/view/fd3cRj; author: vow.
// Dataset license metadata: cc-by-nc-sa-3.0; original retained in
// upstream/fd3cRj.json.
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
float sdSegment(Context &__ctx, f32x2 p, f32x2 a, f32x2 b) {
  f32x2 pa = p - a, ba = b - a;
  return length(pa - ba * clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f));
}
float segment_dist(Context &__ctx, f32x2 p, f32x4 s) {
  return sdSegment(__ctx, p, s.swizzle<0, 1>(), s.swizzle<2, 3>());
}
float abc[26];
void init_abc(Context &__ctx) {
  abc[0] = .949951171875f;
  abc[1] = .863037109375f;
  abc[2] = .80859375f;
  abc[3] = .800537109375f;
  abc[4] = .93359375f;
  abc[5] = .88671875f;
  abc[6] = .871337890625f;
  abc[7] = .199951171875f;
  abc[8] = .7998046875f;
  abc[9] = .051513671875f;
  abc[10] = .1367950439453125f;
  abc[11] = .05859375f;
  abc[12] = .01263427734375f;
  abc[13] = .0125885009765625f;
  abc[14] = .809326171875f;
  abc[15] = .94970703125f;
  abc[16] = .8093414306640625f;
  abc[17] = .9497222900390625f;
  abc[18] = .992431640625f;
  abc[19] = .7529296875f;
  abc[20] = .059326171875f;
  abc[21] = .011810302734375f;
  abc[22] = .0124969482421875f;
  abc[23] = .0002288818359375f;
  abc[24] = .19677734375f;
  abc[25] = .796966552734375f;
}
float get_abc(Context &__ctx, float i) {
  return i < 16.f
             ? (i < 8.f
                    ? (i < 4.f ? (i < 2.f ? (i < 1.f ? abc[18] : abc[7])
                                          : (i < 3.f ? abc[0] : abc[12]))
                               : (i < 6.f ? (i < 5.f ? abc[0] : abc[13])
                                          : (i < 7.f ? abc[0] : abc[0])))
                    : (i < 12.f ? (i < 10.f ? (i < 9.f ? abc[18] : abc[7])
                                            : (i < 11.f ? abc[0] : abc[12]))
                                : (i < 14.f ? (i < 13.f ? abc[0] : abc[13])
                                            : (i < 15.f ? abc[0] : abc[0]))))
             : (i < 24.f
                    ? (i < 20.f ? (i < 18.f ? (i < 17.f ? abc[18] : abc[7])
                                            : (i < 19.f ? abc[0] : abc[12]))
                                : (i < 22.f ? (i < 21.f ? abc[0] : abc[13])
                                            : (i < 23.f ? abc[0] : abc[0])))
                    : (i < 25.f ? abc[0] : abc[0]));
}
f32x4 sgmt[16];
f32x2 sk = f32x2(2.f);
void init_segments(Context &__ctx, f32x4 r) {
  f32x2 a = r.swizzle<0, 1>(), c = r.swizzle<2, 1>(), b = (a + c) / 2.f,
        g = r.swizzle<0, 3>(), d = (a + g) / sk.data[0], i = r.swizzle<2, 3>(),
        e = (a + i) / sk.data[1], f = (c + i) / 2.f, h = (g + i) / 2.f;
  sgmt[0] = f32x4(a, b);
  sgmt[1] = f32x4(b, c);
  sgmt[2] = f32x4(d, e);
  sgmt[3] = f32x4(e, f);
  sgmt[4] = f32x4(g, h);
  sgmt[5] = f32x4(h, i);
  sgmt[6] = f32x4(a, d);
  sgmt[7] = f32x4(d, g);
  sgmt[8] = f32x4(b, e);
  sgmt[9] = f32x4(e, h);
  sgmt[10] = f32x4(c, f);
  sgmt[11] = f32x4(f, i);
  sgmt[12] = f32x4(a, e);
  sgmt[13] = f32x4(c, e);
  sgmt[14] = f32x4(e, g);
  sgmt[15] = f32x4(e, i);
}
void init_segments(Context &__ctx, f32x2 r) {
  init_segments(__ctx, f32x4(0.f, r.data[1], r.data[0], 0.f));
}
f32x3 segment_display(Context &__ctx, f32x2 p, float a, float line_width,
                      f32x3 color) {
  float d = iResolution.data[0], s = .5f, pixel_size = 1.f,
        c = mod(iTime * 3.f / 26.f, 3.f);
  for (int i = 0; i < 16; i++) {
    if (a >= s) {
      d = min(d, segment_dist(__ctx, p, sgmt[i]));
      a -= s;
    }
    s *= .5f;
  }
  if (c <= 1.f) {
    return mix(f32x3(0.f), color,
               1.f - (d - line_width) * 2.f / (line_width * pixel_size));
  } else if (c <= 2.f) {
    return mix(f32x3(0.f), color,
               1.f - (d - line_width) * 2.f / (line_width * pixel_size)) +
           color * 1e3f / (d * d);
  } else {
    return color * 2e1f * line_width * line_width / (d * d);
  }
}
f32x3 my_main(Context &__ctx, f32x2 p) {
  f32x2 gr = f32x2(2.f, 1.f + sqrt(5.f));
  float m = .1f * iResolution.data[1];
  init_abc(__ctx);
  init_segments(__ctx, floor(m * gr));
  return segment_display(
      __ctx, floor(p + m - iResolution.swizzle<0, 1>() / 2.f),
      get_abc(__ctx, mod(iTime * 3.f, 26.f)), floor(m / 1e1f),
      f32x3(1.f, fract(p / 3.f) * f32x2(.35f, .55f)));
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  fragColor.set_swizzle<0, 1, 2>(
      my_main(__ctx, floor(fragCoord.swizzle<0, 1>())));
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
