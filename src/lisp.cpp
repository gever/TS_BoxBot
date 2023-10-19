#ifdef LISP_ENABLED
#include <iostream>
#include <string>
#include "lisp.h"

// setting up lisp runtime
typedef Lisp<8192, 2048> MySmallLisp;
MySmallLisp lisp;
#endif // LISP_ENABLED