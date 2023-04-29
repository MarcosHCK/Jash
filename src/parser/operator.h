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
#ifndef __JASH_PARSER_OPERATOR__
#define __JASH_PARSER_OPERATOR__ 1
#include  <glib.h>
#include <parser/ast.h>

typedef struct _JOperator JOperator;

typedef enum
{
  J_OPERATOR_ASSOC_LEFT,
  J_OPERATOR_ASSOC_RIGHT,
} JOperatorAssoc;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JOperator
  {
    int name;
    guint precedence;
    gboolean unary;
    JAstType ast_type;
    JOperatorAssoc assoc;
  };

  G_GNUC_INTERNAL const JOperator* j_operator_lookup (const gchar* name, size_t length);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_PARSER_OPERATOR__
