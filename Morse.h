#include <stdio.h>
#ifdef SPLINT_FIX
typedef float GLfloat;
#else
#include <GLUT/glut.h>
#endif
#include <math.h>

#define PLUS_INFINITY MAXFLOAT

#include "list.h"
#include "simplex.h"
#include "cancel.h"
