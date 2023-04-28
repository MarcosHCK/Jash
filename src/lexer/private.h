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
#ifndef __JASH_LEXER_PRIVATE__
#define __JASH_LEXER_PRIVATE__ 1
#include <lexer/token.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JTokens
  {
    guint ref_count;
    GStringChunk* chunk;

    union
    {
      GArray g_array;

      struct
      {
        JToken* elements;
        guint count;
      };
    } *array;
  };

  G_GNUC_INTERNAL JTokens* _j_tokens_new ();

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_LEXER_PRIVATE__
