// Port of https://www.shadertoy.com/view/slccDj; author: whkrmrgks0.
// Dataset license metadata: CC-BY-NC-SA-3.0; original retained in
// upstream/slccDj.json.
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
const float cus = 0.15f;
const float vstr = .05f;
const float marginv = .02f;
const float dts = 1.f;
const float AAz = 64.f;
const float vex = 2.f;
const float capa = 1.f;
const float capaiter = 5.0f;
const float capashape = 3.0f;
const float scanline = 240.f;
const float gma = 1.0f;
const float sling = 2.0f;
float sawtooth(Context &__ctx, float inp) { return inp - floor(inp); }
float square(Context &__ctx, float zed, float marchpoint, float floaz) {
  return step(sawtooth(__ctx, zed / floaz), marchpoint / floaz);
}
float triangle(Context &__ctx, float zed) {
  return abs(sawtooth(__ctx, zed + 0.5f) - .5f) * 2.f;
}
f32x2 hash22(Context &__ctx, f32x2 seed) {
  return fract(75.345f * sin(623.3754f * seed));
}
float grd(Context &__ctx, f32x2 uv, f32x2 disp) {
  uv += disp * dts;
  uv /= dts;
  return square(__ctx, uv.data[0], 2.f, 6.f) *
         square(__ctx, uv.data[1], 1.f, 2.f);
}
f32x3 tpscany(Context &__ctx, f32x3 bef, f32x3 ucj, f32x3 dcj, float temp) {
  f32x3 scan = f32x3(.0f);
  scan += max((triangle(__ctx, temp) - 1.f + (bef * sling)), .0f);
  scan += max((clamp(.0f, 1.f, temp * 2.f - 1.f) - 2.f) + (ucj * sling), .0f);
  scan +=
      max((clamp(.0f, 1.f, -(temp * 2.f - 1.f)) - 2.f) + (dcj * sling), .0f);
  return scan / (sling * 0.5f);
}
void pinc(Context &__ctx, f32x2 uv, f32x2 &uv2, float &mxbf, float &vign,
          float ar) {
  uv2 = (uv * f32x2(2.f) - f32x2(1.f)) *
        f32x2((1.f + marginv), (1.f + marginv * ar));
  uv2 = f32x2(
      uv2.data[0] / (cos(abs(uv2.data[1] * cus) * 6.28318530718f / 4.f)),
      uv2.data[1] / (cos(abs(uv2.data[0] * cus * ar) * 6.28318530718f / 4.f)));
  f32x2 uvbef = abs(uv2) - f32x2(1.f);
  mxbf = max(uvbef.data[0], uvbef.data[1]);
  vign = max(uvbef.data[0] * uvbef.data[1], .0f);
  uv2 = (uv2 + f32x2(1.f)) * f32x2(.5f);
}
float scimpresp(Context &__ctx, float range) {
  return sin(pow(range, capashape) * 6.28318530718f) + 1.f;
}
void mainImage(Context &__ctx, f32x4 &fragColor, f32x2 fragCoord) {
  f32x2 uv2;
  float mxbf, vign;
  f32x2 ratd = fragCoord;
  f32x2 uv = fragCoord / iResolution.swizzle<0, 1>();
  pinc(__ctx, uv, uv2, mxbf, vign, iResolution.data[0] / iResolution.data[1]);
  f32x2 nuv = uv2;
  f32x2 nuvyud = f32x2(floor(nuv.data[1] * scanline - 1.f) / scanline,
                       floor(nuv.data[1] * scanline + 1.f) / scanline);
  nuv.data[1] = floor(nuv.data[1] * scanline) / scanline;
  f32x3 bef = f32x3(.0f);
  f32x3 ucj = f32x3(.0f);
  f32x3 dcj = f32x3(.0f);
  float capatemp, capainteg = .0f;
  for (float i = -capaiter / 2.f; i < capaiter / 2.f; i++) {
    capatemp = scimpresp(__ctx, (i + capaiter / 2.f) / capaiter);
    capainteg += capatemp;
    bef += texture(__ctx.channel0,
                   f32x2(sawtooth(__ctx, nuv.data[0] - capa / scanline * i /
                                                           (capaiter / 2.f)),
                         nuv.data[1]))
               .swizzle<0, 1, 2>() *
           capatemp;
    ucj += texture(__ctx.channel0,
                   f32x2(sawtooth(__ctx, nuv.data[0] - capa / scanline * i /
                                                           (capaiter / 2.f)),
                         nuvyud.data[1]))
               .swizzle<0, 1, 2>() *
           capatemp;
    dcj += texture(__ctx.channel0,
                   f32x2(sawtooth(__ctx, nuv.data[0] - capa / scanline * i /
                                                           (capaiter / 2.f)),
                         nuvyud.data[0]))
               .swizzle<0, 1, 2>() *
           capatemp;
  }
  dcj /= capainteg;
  bef /= capainteg;
  ucj /= capainteg;
  f32x3 scan = f32x3(.0f);
  float temp;
  float snippet;
  float integral = .0f;
  for (float i = -AAz / 2.f; i <= AAz / 2.f; i++) {
    snippet = (AAz / 2.f - abs(i)) / AAz / 2.f;
    integral += snippet;
    temp = sawtooth(__ctx, uv2.data[1] * scanline);
    scan +=
        tpscany(__ctx, bef, ucj, dcj,
                temp + (i / AAz * 2.f) * vex / iResolution.data[1] * scanline) *
        snippet;
  }
  scan /= integral;
  float brd = step(mxbf, .0f);
  vign = pow(vign, vstr);
  f32x3 grid = f32x3(grd(__ctx, ratd, f32x2(4.f, 0.f)),
                     grd(__ctx, ratd, f32x2(0.f, 0.f)),
                     grd(__ctx, ratd, f32x2(2.f, 0.f)));
  grid += f32x3(grd(__ctx, ratd, f32x2(4.f, 0.f) + f32x2(3.f, 1.f)),
                grd(__ctx, ratd, f32x2(0.f, 0.f) + f32x2(3.f, 1.f)),
                grd(__ctx, ratd, f32x2(2.f, 0.f) + f32x2(3.f, 1.f)));
  float mask = brd * vign;
  scan /= sling;
  scan = pow(scan, f32x3(0.5f));
  scan = pow(scan, f32x3(1.f + 1.f / 3.f));
  scan = pow(scan, f32x3(gma));
  f32x3 grided = scan * grid * 3.f;
  f32x3 final = min(f32x3(mix(grided, scan, scan)), f32x3(1.f)) * mask;
  fragColor = f32x4(final, 1.f);
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
