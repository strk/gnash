dnl
dnl  Copyright (C) 2005, 2006 Free Software Foundation, Inc.
dnl
dnl  This program is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU General Public License as published by
dnl  the Free Software Foundation; either version 2 of the License, or
dnl  (at your option) any later version.
dnl
dnl  This program is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl  GNU General Public License for more details.
dnl
dnl  You should have received a copy of the GNU General Public License
dnl  along with this program; if not, write to the Free Software
dnl  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

AC_DEFUN([GNASH_HASHMAP],
[
  AC_LANG_PUSH(C++)
  AC_CHECK_HEADER(ext/hash_map, [
    AC_DEFINE([GNU_HASH_MAP], [1], [Using GNU ext/hash_map.h])
    AC_DEFINE([HASH_MAP_NS], [__gnu_cxx], [Using GNU __gnu_cxx::])
    ], [
    AC_CHECK_HEADER(hash_map, [
      AC_DEFINE([WIN32_HASH_MAP], [1], [Using Win32 hash_map.h])
      AC_DEFINE([HASH_MAP_NS], [stdext], [Using win32 stdext::])
    ])
  ])
  AC_LANG_POP(C++)
])

