AC_INIT([gpvdm], [4.40])
AC_CONFIG_SRCDIR([./src/main.c])
AC_CANONICAL_TARGET
AC_CANONICAL_HOST

AS_CASE([$target_os],
  [mingw32], [
AC_MSG_NOTICE([********************************])
AC_MSG_NOTICE([*Setting up config for windows *])
AC_MSG_NOTICE([********************************])

AC_SUBST(LINK_FLAG, "-posix")
AC_SUBST(COMPILE_FLAG, " -D windows -posix")
AC_SUBST(SLIB_EXT, "a")
AC_SUBST(DLIB_EXT, "dll")
AC_SUBST(EXE_EXT, "exe")

AC_CHECK_HEADERS([umfpack.h],[],[AC_MSG_ERROR([umfpack.h is needed but not found])])
AC_SUBST(DLL_LINKER, ["-L/home/rod/windll/compiled_dlls/ -Wl,--out-implib,plugin.a -lgpvdm_light -lgpvdm_lib -lgpvdm_dump -lgpvdm_measure -lgpvdm_dos  -lgpvdm_light -lzip-2 -lumfpack -lgpvdm_mesh "])
AC_SUBST(DLL_COMPILE, ["-DBUILDING_EXAMPLE_DLL"])
AC_SUBST(DLL_DEBUG, ["-Wall"])
AC_SUBST(EXE_LINK,["-L/home/rod/windll/compiled_dlls/ -lumfpack -lzip-2"])

],
  [linux-gnu], [

AC_MSG_NOTICE([*****************************])
AC_MSG_NOTICE([*Setting up config for linux*])
AC_MSG_NOTICE([*****************************])
AC_SUBST(LINK_FLAG, "")

AC_SUBST(COMPILE_FLAG, "`pkg-config --cflags dbus-1` -D enable_server -D dbus -g")
AC_CHECK_LIB(zip, [main], [], [AC_MSG_ERROR([libzip is needed but not found.])])
AC_CHECK_HEADERS([suitesparse/umfpack.h],[],[AC_MSG_ERROR([suitesparse/umfpack.h is needed but not found])])
AC_CHECK_LIB(matheval, [main], [], [AC_MSG_ERROR([matheval is needed but not found.])])
AC_SUBST(SLIB_EXT, "a")
AC_SUBST(DLIB_EXT, "so")
AC_SUBST(EXE_EXT, "o")
AC_SUBST(EXE_LINK,["`pkg-config --libs dbus-1` -lumfpack -lm -lgsl -lgslcblas -rdynamic -export-dynamic -ldl -lzip -lz -lmatheval -lefence -g"])


AC_SUBST(DLL_LINKER, ["`dpkg-buildflags --get LDFLAGS` -Wl,-soname,plugin.so"])
AC_SUBST(DLL_COMPILE, ["-fPIC"])
AC_SUBST(DLL_DEBUG, ["-Wall -g"])

],
  [AC_MSG_ERROR("No assembler code for CPU $target_os")]
)


AC_PROG_RANLIB
AM_PROG_AR
AM_INIT_AUTOMAKE
AC_PROG_CC

AM_PATH_PYTHON([2.7], [], [AC_MSG_ERROR([You need python installed])])
AC_CHECK_HEADERS([zip.h],[],[AC_MSG_ERROR([zip.h is needed but not found])])
AC_CHECK_HEADERS([gsl/gsl_multimin.h],[],[AC_MSG_ERROR([gsl/gsl_multimin.h is needed but not found])])

AC_CHECK_PROG([PKG_CONFIG], pkg-config, pkg-config,no)
if test "$PKG_CONFIG" = "no"; then
	AC_MSG_ERROR([pkg-config is not installed.])
fi

AC_CHECK_PROG([PKG_HELP2MAN], help2man, help2man,no)
if test "$PKG_HELP2MAN" = "no"; then
	AC_MSG_ERROR([help2man is not installed.])
fi

AC_CHECK_PROG([PKG_RSGV], rsvg-convert, rsvg-convert,no)
if test "$PKG_RSGV" = "no"; then
	AC_MSG_ERROR([rsvg-convert is not installed.])
fi

#i?86
#x86_64

m4_include([make_files.m4])
m4_include([local_link.m4])

AC_CHECK_HEADERS(math.h)
AC_FUNC_MALLOC
AC_CHECK_FUNCS([sqrt])


#AC_CONFIG_FILES
#images/Makefile

m4_include([config_files.m4])

AC_OUTPUT

AC_MSG_NOTICE([gpvdm build finished.])