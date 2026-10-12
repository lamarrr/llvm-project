struct Output {
  __storage unsigned *data;
};

__compute
__workgroup_size_id(0, 4, 1, 2, 2, 1)
void workgroup_size_id(__push_constant const Output *output) {
  output->data[0] = workgroup_size[0] + workgroup_size[1] + workgroup_size[2];
}
