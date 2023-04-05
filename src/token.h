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
#ifndef __JASH_TOKEN__
#define __JASH_TOKEN__ 1
#include <glib.h>

typedef struct _JToken JToken;
typedef enum _JTokenType JTokenType;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JToken
  {
    guint64 type : 4;
    guint64 line : (sizeof (guint64) * 8 - 4) / 2;
    guint64 column : (sizeof (guint64) * 8 - 4) / 2;
    const gchar* value;
  };

  enum _JTokenType
  {
    J_TOKEN_TYPE_BUILTIN,
    J_TOKEN_TYPE_COMMENT,
    J_TOKEN_TYPE_KEYWORD,
    J_TOKEN_TYPE_LITERAL,
    J_TOKEN_TYPE_OPERATOR,
    J_TOKEN_TYPE_SEPARATOR,
    J_TOKEN_TYPE_QUOTED,
  };

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_TOKEN__
