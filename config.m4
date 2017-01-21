dnl $Id$
dnl config.m4 for extension stata

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(stata, for stata support,
dnl Make sure that the comment is aligned:
dnl [  --with-stata             Include stata support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(stata, whether to enable stata support,
Make sure that the comment is aligned:
 [  --enable-stata           Enable stata support])

if test "$PHP_STATA" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-stata -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/stata.h"  # you most likely want to change this
  dnl if test -r $PHP_STATA/$SEARCH_FOR; then # path given as parameter
  dnl   STATA_DIR=$PHP_STATA
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for stata files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       STATA_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$STATA_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the stata distribution])
  dnl fi

  dnl # --with-stata -> add include path
  dnl PHP_ADD_INCLUDE($STATA_DIR/include)

  dnl # --with-stata -> check for lib and symbol presence
  dnl LIBNAME=stata # you may want to change this
  dnl LIBSYMBOL=stata # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $STATA_DIR/$PHP_LIBDIR, STATA_SHARED_LIBADD)
  AC_DEFINE(HAVE_STATA,1,[Whether you have Stata])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong stata lib version or lib not found])
  dnl ],[
  dnl   -L$STATA_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(STATA_SHARED_LIBADD)

  PHP_NEW_EXTENSION(stata, stataread.c statawrite.c stata.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
