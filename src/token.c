/* Copyright 2023 MarcosHCK
 * Copyright 2023 DavierSB
 * This file is part of JASH.
 *
 * JASH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JASH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JASH. If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <token.h>

#define _DEFINE_INTERN_FULL(type,name,value) \
  const gchar* j_token_ ## type ## _ ## name ## _intern_string (void) \
    { \
      static gsize __value__ = 0; \
      if (g_once_init_enter (&__value__)) \
        { \
          const gchar* static_string = G_STRINGIFY (value); \
          const gchar* intern_string = g_intern_static_string (static_string); \
          g_once_init_leave (&__value__, GPOINTER_TO_SIZE (intern_string)); \
        } \
      return (gchar*) __value__; \
    }

#define _DEFINE_INTERN(type,name) _DEFINE_INTERN_FULL (type,name,name)

_DEFINE_INTERN (builtin, again);
_DEFINE_INTERN (builtin, cd);
_DEFINE_INTERN (builtin, exit);
_DEFINE_INTERN (builtin, false);
_DEFINE_INTERN (builtin, fg);
_DEFINE_INTERN (builtin, get);
_DEFINE_INTERN (builtin, help);
_DEFINE_INTERN (builtin, history);
_DEFINE_INTERN (builtin, jobs);
_DEFINE_INTERN (builtin, set);
_DEFINE_INTERN (builtin, true);
_DEFINE_INTERN (builtin, unset);
_DEFINE_INTERN (keyword, else);
_DEFINE_INTERN_FULL (keyword, end, fi);
_DEFINE_INTERN (keyword, if);
_DEFINE_INTERN (keyword, then);
_DEFINE_INTERN_FULL (operator, detach, &);
_DEFINE_INTERN_FULL (operator, expansion, `);
_DEFINE_INTERN_FULL (operator, logical_and, &&);
_DEFINE_INTERN_FULL (operator, logical_or, ||);
_DEFINE_INTERN_FULL (operator, pipe, |);
_DEFINE_INTERN_FULL (operator, redirection_append, >>);
_DEFINE_INTERN_FULL (operator, redirection_read, <);
_DEFINE_INTERN_FULL (operator, redirection_write, >);
_DEFINE_INTERN_FULL (separator, chain, ;);
_DEFINE_INTERN_FULL (separator, newline, \n);
