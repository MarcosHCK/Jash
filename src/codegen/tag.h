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
#ifndef __JASH_CODEGEN_TAG__
#define __JASH_CODEGEN_TAG__ 1
#include <glib.h>

typedef gpointer JTag;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  #define j_tag_as_pc(src) \
    (({ \
        const JTag* __src = ((src)); \
          GPOINTER_TO_UINT (*__src); \
      }))
  #define j_tag_copy(src,dst) \
    (({ \
        const JTag* __src = ((src)); \
        JTag* __dst = ((dst)); \
          *__dst = *__src; \
      }))

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_TAG__
