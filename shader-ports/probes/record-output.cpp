// Reproducer discovered while porting Godot's fragment shaders.
struct Color {
  float r, g, b, a;
};

__fragment
void record_output(__input(0) Color color, __output(0) Color *output) {
  *output = color;
}
