__input__(2) extern const float incoming;
__output__(3) extern float outgoing;
__fragment__ void global_interfaces() { outgoing = incoming; }
