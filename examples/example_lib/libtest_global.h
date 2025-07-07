#ifndef LIBTEST_GLOBAL_H
#define LIBTEST_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBTEST_LIBRARY)
#  define LIBTEST_EXPORT Q_DECL_EXPORT
#else
#  define LIBTEST_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBTEST_GLOBAL_H
