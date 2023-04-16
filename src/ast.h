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
#ifndef __JASH_AST__
#define __JASH_AST__ 1
#include <glib.h>

typedef struct _GNode *Ast;

#define ast_node(type) (({ guint __type = ((type)); (Ast) g_node_new (GUINT_TO_POINTER (__type)); }))
#define ast_free(node) ((_g_node_destroy0 (node)))
#define ast_retype(type,node) (({ guint __type = ((type)); Ast __node = ((node)); __node->data = GUINT_TO_POINTER (__type); __node; }))
#define ast_append(parent,node) ({ Ast __parent = ((parent)); Ast __node = ((node)); g_node_append (__parent, __node); })
#define ast_unlink(node) (({ Ast __node = ((node)); g_node_unlink (__node); }))
#define ast_type(node) (({ Ast __node = ((node)); (AstType) GPOINTER_TO_INT (__node->data); }))
#define ast_parent(node) (({ Ast __node = ((node)); (Ast) ((GNode*) __node)->parent; }))
#define ast_first(node) (({ Ast __node = ((node)); (Ast) g_node_first_child (__node); }))
#define ast_next(node) (({ Ast __node = ((node)); (Ast) g_node_next_sibling (__node); }))
#define ast_prev(node) (({ Ast __node = ((node)); (Ast) g_node_prev_sibling (__node); }))
#define ast_find(node,type) (({ Ast __node = ((node)); guint __type = ((type)); g_node_find (__node, G_PRE_ORDER, G_TRAVERSE_ALL, GUINT_TO_POINTER (__type)); }))
#define ast_find_child(node,type) (({ Ast __node = ((node)); guint __type = ((type)); g_node_find_child (__node, G_TRAVERSE_ALL, GUINT_TO_POINTER (__type)); }))

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum
  {
    AST_ARGUMENTS,
    AST_BUILTIN,
    AST_DATA,
    AST_DETACH,
    AST_EXPANSION,
    AST_IFCLOSURE,
    AST_IFCLOSURE_CONDITION,
    AST_IFCLOSURE_DIRECT,
    AST_IFCLOSURE_REVERSE,
    AST_INVOKE,
    AST_LOGICAL_AND,
    AST_LOGICAL_OR,
    AST_PIPE,
    AST_REDIRECT_INPUT,
    AST_REDIRECT_OUTPUT_APPEND,
    AST_REDIRECT_OUTPUT_REPLACE,
    AST_SCOPE,
    AST_TARGET,
  } AstType;

  static inline Ast ast_data (gconstpointer data)
  {
    Ast wrapper = ast_node (AST_DATA);
    Ast node = g_node_new ((gpointer) data);
  return (ast_append (wrapper, node), wrapper);
  }

  static inline Ast ast_wrap (AstType type, Ast node)
  {
    Ast wrapper = ast_node (type);
  return (ast_append (wrapper, node), wrapper);
  }

  #if !DEVELOPER
  # define dumpast(ast)
  #else // DEVELOPER

    static void dumpast (Ast ast, GString* pre)
    {
      Ast child;

      const gchar* types [] =
        {
          "AST_ARGUMENTS", "AST_BUILTIN", "AST_DATA", "AST_DETACH",
          "AST_EXPANSION", "AST_IFCLOSURE", "AST_IFCLOSURE_CONDITION",
          "AST_IFCLOSURE_DIRECT", "AST_IFCLOSURE_REVERSE", "AST_INVOKE",
          "AST_LOGICAL_AND", "AST_LOGICAL_OR", "AST_PIPE",
          "AST_REDIRECT_INPUT", "AST_REDIRECT_OUTPUT_APPEND", 
          "AST_REDIRECT_OUTPUT_REPLACE", "AST_SCOPE", "AST_TARGET",
        };

      if (ast->parent != NULL && (ast_type (ast->parent) == AST_DATA))
        g_printerr ("%snode - %s\n", pre->str, (gchar*) ast->data);
      else
        g_printerr ("%snode - %s\n", pre->str, types [ast_type (ast)]);

      g_string_append_c (pre, '|');
      g_string_append_c (pre, ' ');

      for (child = ast_first (ast);
          child != NULL;
          child = ast_next (child))
        dumpast (child, pre);

      g_string_truncate (pre, pre->len - 2);
    }

  # define dumpast(ast) ({ g_printerr ("(" G_STRLOC "): dumpast()!\n"); GString* pre; (dumpast) ((ast), pre = g_string_sized_new (64)); g_string_free (pre, FALSE); })
  #endif // !DEVELOPER

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_AST__
