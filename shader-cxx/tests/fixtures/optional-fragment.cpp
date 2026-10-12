__fragment
void optional_fragment(__output(0) float *color) {
  *color = bary_coord[0] + bary_coord[1] + bary_coord_noperspective[2] +
           static_cast<float>(shading_rate);
}
