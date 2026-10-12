struct Output {
  __storage__ float *data;
};

__fragment__
void input_attachment_test(
    __binding__(0, 0) input_attachment<float, 1> attachment,
    __push_constant__ const Output *output) {
  output->data[0] = attachment.load();
}
