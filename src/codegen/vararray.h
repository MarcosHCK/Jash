/* Copyright 2023 MarcosHCK
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
#ifndef __JASH_CODEGEN_VARARRAY__
#define __JASH_CODEGEN_VARARRAY__ 1
#include <glib.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

  #define J_VARARRAY_CLEAR(name) g_clear_pointer (& name##_dyn, g_free)
  
  #define J_VARARRAY_DECL(name,ctype,pre) \
    ctype* name = NULL; \
    ctype* name##_dyn = NULL; \
    ctype name##_stat [(pre)];
  
  #define J_VARARRAY_INIT(name,now) \
    G_STMT_START \
      { \
        gsize __now = ((now)); \
   ; \
        if (G_N_ELEMENTS (name##_stat) >= __now) \
          name = name##_stat; \
        else \
          { \
            name##_dyn = g_malloc (sizeof (*name) * __now); \
            name = name##_dyn; \
          } \
      } \
    G_STMT_END

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_VARARRAY__
