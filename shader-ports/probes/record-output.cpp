// Reproducer discovered while porting Godot's fragment shaders.
struct Color {
  float r, g, b, a;
};

__fragment__
void record_output(__input__(0) Color color, __output__(0) Color *output) {
  *output = color;
}
