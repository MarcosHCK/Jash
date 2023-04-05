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
#ifndef __JASH_CODE__
#define __JASH_CODE__ 1
#include <glib.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum _JCodeType
  {
    PIPE,
    PSI,
    PSO,
    FSI,
    FSO,
    PAS,
    PAP,
    GET,
    SET,
    USET,
    DUMP,
    EXEC,
    SYNC,
    IF,
    IFN,
    END,
    LT,
    LF,
    MAX_CODE,
  } JCodeType;

  typedef struct _JCode
  {
    guint ref_count;
    guint type : 5;
    guint size : sizeof (guint) * 8 - 5;

    union
    {
      gpointer argument;
      gchar* string_argument;
      guintptr uintptr_argument;
      gint int_argument;
    };
  } JCode;

  G_GNUC_INTERNAL JCode* j_code_new (JCodeType type, gsize argument_size);
  G_GNUC_INTERNAL JCode* j_code_new_string (JCodeType type, const gchar* value);
  G_GNUC_INTERNAL JCode* j_code_ref (JCode* code);
  G_GNUC_INTERNAL void j_code_unref (JCode* code);

  #define j_code_new_int(type,value) \
    (G_GNUC_EXTENSION ({ \
        JCodeType __type = ((type)); \
        gint __value = ((value)); \
        JCode* __code = j_code_new (__type, 0); \
          __code->uintptr_argument = (guintptr) __value; \
          __code; \
      }))

  #define j_code_new_uint(type,value) \
    (G_GNUC_EXTENSION ({ \
        JCodeType __type = ((type)); \
        gint __value = ((value)); \
        JCode* __code = j_code_new (__type, 0); \
          __code->uintptr_argument = (guintptr) __value; \
          __code; \
      }))

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODE__
