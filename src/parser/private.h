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
#ifndef __JASH_PARSER_PRIVATE__
#define __JASH_PARSER_PRIVATE__ 1
#include <parser/ast.h>
#include <parser/walker.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

  #if !DEVELOPER
  # define j_ast_dump(ast)
  #else // DEVELOPER

    static void j_ast_dump (JAst* ast, GString* pre)
    {
      JAst* child;

      const gchar* types [] =
        {
          "J_AST_TYPE_ARGUMENTS", "J_AST_TYPE_BUILTIN", "J_AST_TYPE_DATA", "J_AST_TYPE_DETACH",
          "J_AST_TYPE_EXPANSION", "J_AST_TYPE_IFCLOSURE", "J_AST_TYPE_IFCLOSURE_CONDITION",
          "J_AST_TYPE_IFCLOSURE_DIRECT", "J_AST_TYPE_IFCLOSURE_REVERSE", "J_AST_TYPE_INVOKE",
          "J_AST_TYPE_LOGICAL_AND", "J_AST_TYPE_LOGICAL_OR", "J_AST_TYPE_PIPE",
          "J_AST_TYPE_REDIRECT_INPUT", "J_AST_TYPE_REDIRECT_OUTPUT_APPEND", 
          "J_AST_TYPE_REDIRECT_OUTPUT_REPLACE", "J_AST_TYPE_SCOPE", "J_AST_TYPE_TARGET",
        };

      if (ast->parent != NULL && (j_ast_get_type (ast->parent) == J_AST_TYPE_DATA))
        g_printerr ("%snode - %s\n", pre->str, (gchar*) ast->data);
      else
        g_printerr ("%snode - %s\n", pre->str, types [j_ast_get_type (ast)]);

      g_string_append_c (pre, '|');
      g_string_append_c (pre, ' ');

      for (child = j_ast_get_first_child (ast);
          child != NULL;
          child = j_ast_get_next_sibling (child))
        j_ast_dump (child, pre);

      g_string_truncate (pre, pre->len - 2);
    }

  # define j_ast_dump(ast) ({ g_printerr ("(" G_STRLOC "): j_ast_dump()!\n"); GString* pre; (j_ast_dump) ((ast), pre = g_string_sized_new (64)); g_string_free (pre, FALSE); })
  #endif // !DEVELOPER

  #if !DEVELOPER
  # define j_walker_dump(walker)
  #else // DEVELOPER

    static void j_walker_dump (JWalker* walker)
    {
      GList* list;
      gchar* escp;
      guint i;

      const gchar* types [] =
        {
          "J_TOKEN_TYPE_BUILTIN", "J_TOKEN_TYPE_COMMENT",
          "J_TOKEN_TYPE_KEYWORD", "J_TOKEN_TYPE_LITERAL",
          "J_TOKEN_TYPE_OPERATOR", "J_TOKEN_TYPE_SEPARATOR",
          "J_TOKEN_TYPE_QUOTED",
        };

      G_STATIC_ASSERT (J_TOKEN_TYPE_BUILTIN == 0);
      G_STATIC_ASSERT (J_TOKEN_TYPE_QUOTED == G_N_ELEMENTS (types) - 1);

      for (list = g_queue_peek_head_link (&walker->queue), i = 0;
          list != NULL;
          list = list->next, ++i)
      {
        const JToken* token = list->data;
        const gchar* value = token->value;
        const guint type = token->type;

        g_printerr ("  walker[%i] = '%s' (%s)\n", i, escp = g_strescape (value, NULL), types [type]);
        g_free (escp);
      }
    }

  # define j_walker_dump(walker) ({ g_printerr ("(" G_STRLOC "): j_walker_dump()!\n"); (j_walker_dump) ((walker)); })
  #endif // !DEVELOPER

  #define j_ast_append(node,child) (({ JAst* __node = ((node)); JAst* __child = ((child)); g_node_append (__node, __child); }))
  #define j_ast_new(type) (({ guint __type = ((type)); (JAst*) g_node_new (GUINT_TO_POINTER (__type)); }))

  static inline JAst* j_ast_new_data (gconstpointer data)
  {
    JAst* wrapper = j_ast_new (J_AST_TYPE_DATA);
  return (g_node_append_data (wrapper, (gpointer) data), wrapper);
  }

  static inline JAst* j_ast_new_wrap (JAstType type, JAst* node)
  {
    JAst* wrapper = j_ast_new (type);
  return (j_ast_append (wrapper, node), wrapper);
  }

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_PARSER_PRIVATE__
