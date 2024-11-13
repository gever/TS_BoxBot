// Work around the lack of 'inline' for the single definition rule in c++11 which is available in c++17.
#ifndef DEFINE_GLOBALS
#undef GLOBAL
#undef GLOBAL_EXTERN
#define GLOBAL(_type, _name, _value) extern _type _name
#define GLOBAL_EXTERN extern
#else
#undef GLOBAL
#undef GLOBAL_EXTERN
#define GLOBAL(_type, _name, _value) _type _name = _value
#define GLOBAL_EXTERN
#endif

