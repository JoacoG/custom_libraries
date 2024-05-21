//  For debugging...

#define COMMA ,           //  Para los macros de debugging

#define DEBUG 1           //  Indica si se está haciendo debugging

//  Todas las lineas de debugging tienen que ser de la forma D(...)
#if DEBUG
  #define D(x) x
#else
  #define D(x)
#endif

//  For custom library directories...

#define PROJECT_ROOT C:\Users\jguti\Chibi\Librerias

#define TO_STRING(s) #s
#define ABSOLUTE_PATH(root, relative_path) TO_STRING(root\relative_path)
#define RELATIVE_PATH(library) ABSOLUTE_PATH(PROJECT_ROOT, library)