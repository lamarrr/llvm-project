struct Item {
  float value;
  unsigned tag;
};
struct Params {
  __storage Item *data;
};
__compute
__workgroup_size(1, 1, 1)
void pointers(__push_constant const Params *p) {
  auto q = p->data;
  ++q;
  q++;
  --q;
  q--;
  auto distance = (q + 3) - q;
  bool same = q == p->data;
  bool different = q != q + 1;
  bool ordered = q < q + 1 && q + 1 > q;
  q->value = static_cast<float>(distance + same + different + ordered);
  q[1].tag = 99;
  (*q).tag = 12;
}
