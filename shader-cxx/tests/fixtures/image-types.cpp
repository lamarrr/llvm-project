__compute
__workgroup_size(1, 1, 1)
void image_types(__binding(0, 0) texture_1d<float[1]> t0,
                 __binding(0, 1) storage_texture_1d<float, r32f> w0,
                 __binding(0, 2) texture_2d<float> t1,
                 __binding(0, 3) storage_texture_2d<float, r32f> w1,
                 __binding(0, 4) texture_3d<float> t2,
                 __binding(0, 5) storage_texture_3d<float, r32f> w2,
                 __binding(0, 6) texture_1d_layers<float> t3,
                 __binding(0, 7) storage_texture_1d_layers<float, r32f> w3,
                 __binding(0, 8) texture_2d_layers<float> t4,
                 __binding(0, 9) storage_texture_2d_layers<float, r32f> w4,
                 __binding(0, 10) texture_2d_ms<float> t5,
                 __binding(0, 11) storage_texture_2d_ms<float, r32f> w5,
                 __binding(0, 12) texture_2d_ms_layers<float> t6,
                 __binding(0, 13) storage_texture_2d_ms_layers<float, r32f> w6,
                 __binding(0, 14) texture_buffer<float> t7,
                 __binding(0, 15) storage_texture_buffer<float, r32f> w7) {
  auto one = t0.load(0);
  float v0 = one[0];
  auto d0 = t0.dimensions();
  w0.store(0, v0);
  v0 = w0.load(0);
  auto wd0 = w0.dimensions();
  float v1 = t1.load(0, 0);
  auto d1 = t1.dimensions();
  w1.store(0, 0, v1);
  v1 = w1.load(0, 0);
  auto wd1 = w1.dimensions();
  float v2 = t2.load(0, 0, 0);
  auto d2 = t2.dimensions();
  w2.store(0, 0, 0, v2);
  v2 = w2.load(0, 0, 0);
  auto wd2 = w2.dimensions();
  float v3 = t3.load(0, 0);
  auto d3 = t3.dimensions();
  w3.store(0, 0, v3);
  v3 = w3.load(0, 0);
  auto wd3 = w3.dimensions();
  float v4 = t4.load(0, 0, 0);
  auto d4 = t4.dimensions();
  w4.store(0, 0, 0, v4);
  v4 = w4.load(0, 0, 0);
  auto wd4 = w4.dimensions();
  float v5 = t5.load(0, 0);
  auto d5 = t5.dimensions();
  w5.store(0, 0, v5);
  v5 = w5.load(0, 0);
  auto wd5 = w5.dimensions();
  float v6 = t6.load(0, 0, 0);
  auto d6 = t6.dimensions();
  w6.store(0, 0, 0, v6);
  v6 = w6.load(0, 0, 0);
  auto wd6 = w6.dimensions();
  float v7 = t7.load(0);
  auto d7 = t7.dimensions();
  w7.store(0, v7);
  v7 = w7.load(0);
  auto wd7 = w7.dimensions();
}
