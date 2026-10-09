struct P {
  __storage__ float *data;
};
struct V {
  float x, y;
};
__compute__
__workgroup_size__(1, 1, 1)
void inline_asm(__push_constant__ const P *p) {
  float x = p->data[0], a, b;
  asm volatile("%0 = OpFAdd %t0 %2 %2\n%1 = OpFSub %t1 %2 %2"
               : "=r"(a), "=r"(b)
               : "r"(x));
  p->data[0] = a;
  p->data[1] = b;
  float c[2] = {x, x}, d[2];
  asm volatile("%0 = OpFAdd %t0 %1 %1" : "=v"(d) : "v"(c));
  p->data[2] = d[0];
  p->data[3] = d[1];
  V v{x, x}, o;
  asm volatile("%0 = OpFAdd %t0 %1 %1" : "=v"(o) : "v"(v));
  p->data[4] = o.x;
  p->data[5] = o.y;
  asm volatile("%0 = OpFAdd %t0 %0 %0" : "+r"(x));
  p->data[6] = x;
  asm volatile("%[value] = OpLoad %t[value] %[address] Aligned %a[address]"
               : [value] "=r"(a)
               : [address] "m"(p->data[6])
               : "memory");
  asm volatile("OpStore %[address] %[value] Aligned %a[address]"
               : [address] "=m"(p->data[7])
               : [value] "r"(a)
               : "memory");
  asm volatile("%%two = OpConstant %t0 %2\n%%temporary = OpFMul %t0 %1 "
               "%%two\n%0 = OpFAdd %t0 %%temporary %1"
               : "=r"(b)
               : "r"(x), "i"(2));
  p->data[8] = b;
  asm volatile("OpNop");
}
