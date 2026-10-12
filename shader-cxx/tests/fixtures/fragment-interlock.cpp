__fragment
void fragment_interlock(__output(0) float *color) {
  begin_fragment_interlock();
  *color = 1.0f;
  end_fragment_interlock();
}
