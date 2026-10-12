struct Output {
  __storage__ unsigned *data;
};

__compute__
__workgroup_size_id__(0, 4, 1, 2, 2, 1)
void workgroup_size_id(__push_constant__ const Output *output) {
  output->data[0] = workgroup_size[0] + workgroup_size[1] + workgroup_size[2];
}
