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
#ifndef __JASH_PARSER_AST__
#define __JASH_PARSER_AST__ 1
#include <glib-object.h>

#define J_TYPE_AST (j_ast_get_type ())
typedef struct _GNode JAst;

#define j_ast_copy(node) (({ JAst* __node = ((node)); (JAst*) g_node_copy (__node); }))
#define j_ast_find(node,type) (({ JAst* __node = ((node)); guint __type = ((type)); (JAst*) g_node_find (__node, G_PRE_ORDER, G_TRAVERSE_ALL, GUINT_TO_POINTER (__type)); }))
#define j_ast_find_child(node,type) (({ JAst* __node = ((node)); guint __type = ((type)); (JAst*) g_node_find_child (__node, G_TRAVERSE_ALL, GUINT_TO_POINTER (__type)); }))
#define j_ast_free(node) (({ JAst* __node = ((node)); g_node_destroy (__node); }))
#define j_ast_get_ast_type(node) (({ JAst* __node = ((node)); (JAstType) GPOINTER_TO_UINT (__node->data); }))
#define j_ast_get_first_child(node) (({ JAst* __node = ((node)); (JAst*) g_node_first_child (__node); }))
#define j_ast_get_next_sibling(node) (({ JAst* __node = ((node)); (JAst*) g_node_next_sibling (__node); }))
#define j_ast_get_parent(node) (({ JAst* __node = ((node)); (JAst*) ((GNode*) __node)->parent; }))
#define j_ast_get_prev_sibling(node) (({ JAst* __node = ((node)); (JAst*) g_node_prev_sibling (__node); }))
#define j_ast_n_children(node) (({ JAst* __node = ((node)); (guint) g_node_n_children (__node); }))
#define j_ast_unlink(node) (({ JAst* __node = ((node)); g_node_unlink (__node); }))

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum
  {
    J_AST_TYPE_ARGUMENTS,
    J_AST_TYPE_BUILTIN,
    J_AST_TYPE_DATA,
    J_AST_TYPE_DETACH,
    J_AST_TYPE_EXPANSION,
    J_AST_TYPE_IFCLOSURE,
    J_AST_TYPE_IFCLOSURE_CONDITION,
    J_AST_TYPE_IFCLOSURE_DIRECT,
    J_AST_TYPE_IFCLOSURE_REVERSE,
    J_AST_TYPE_INVOKE,
    J_AST_TYPE_LOGICAL_AND,
    J_AST_TYPE_LOGICAL_OR,
    J_AST_TYPE_PIPE,
    J_AST_TYPE_REDIRECT_INPUT,
    J_AST_TYPE_REDIRECT_OUTPUT_APPEND,
    J_AST_TYPE_REDIRECT_OUTPUT_REPLACE,
    J_AST_TYPE_SCOPE,
    J_AST_TYPE_TARGET,
  } JAstType;

  G_GNUC_INTERNAL GType j_ast_get_type (void) G_GNUC_CONST;

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_PARSER_AST__
