struct Output {
  __storage float *data;
};

__fragment
void input_attachment_test(
    __binding(0, 0) input_attachment<float, 1> attachment,
    __push_constant const Output *output) {
  output->data[0] = attachment.load();
}
