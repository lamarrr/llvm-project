// RUN: %clang_cc1 -triple spirv1.4-unknown-vulkan1.3 -std=c++20 -fspirv-shader -fsyntax-only -verify %s

void reads(tile_image<float, 0> color) {
  color.load(0.5f); // expected-error {{tile images support load()}}
  depth_attachment_read(
      0.5f); // expected-error@-1 {{attachment reads accept an optional integer sample index}}
  stencil_attachment_read(
      0u,
      1u); // expected-error@-2 {{attachment reads accept an optional integer sample index}}
}

__fragment
__fragment_interlock(
    0) // expected-error@-1 {{fragment interlock mode must be an interlock_mode value}}
void invalid_mode() {}

__fragment
__fragment_interlock(interlock_mode::sample_ordered)
__fragment_interlock(
    interlock_mode::
        pixel_ordered) // expected-error@-2 {{conflicting fragment interlock modes}}
void conflicting_mode() {}
