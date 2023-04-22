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
#include <codegen/codegen.h>
#include <codegen/context.h>
#include <codegen/walker.h>

static void walk_argument (Dst_DECL, JWalker* walker, JAst* ast, JArgument* argument);
static void walk_command (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe);
static void walk_detach (Dst_DECL, JWalker* walker, JAst* ast);
static void walk_ifclosure (Dst_DECL, JAst* ast);
static void walk_invoke (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe);
static void walk_pipe (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe);
static void walk_scope (Dst_DECL, JAst* ast);
static void walk_target (Dst_DECL, JWalker* walker, JAst* ast, JInvoke* invoke);

void j_context_generate (Dst_DECL, JAst* ast)
{
#if DEVELOPER == 1
  g_assert (j_ast_get_type (ast) == J_AST_TYPE_SCOPE);
#endif // DEVELOPER
  walk_scope (Dst, ast);
}

static void walk_argument (Dst_DECL, JWalker* walker, JAst* ast, JArgument* argument)
{
  switch (j_ast_get_type (ast))
    {
      case J_AST_TYPE_DATA:
        {
  #if DEVELOPER == 1
          g_assert (j_ast_n_children (ast) == 1);
  #endif // DEVELOPER
          JAst* data = j_ast_get_first_child (ast);

          argument->type = J_ARGUMENT_TYPE_DATA;
          argument->index = j_walker_add_argument (walker, data->data);
          break;
        }
      case J_AST_TYPE_EXPANSION:
        {
          argument->type = J_ARGUMENT_TYPE_EXPANSION;
          argument->index = j_walker_add_expansion (walker, ast);
          break;
        }
      default: g_assert_not_reached ();
    }
}

static void walk_command (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe)
{
  switch (j_ast_get_type (ast))
    {
      case J_AST_TYPE_INVOKE: walk_invoke (Dst, walker, ast, in_pipe, out_pipe); break;
      case J_AST_TYPE_PIPE: walk_pipe (Dst, walker, ast, in_pipe, out_pipe); break;
      default: g_assert_not_reached ();
    }
}

static void walk_detach (Dst_DECL, JWalker* walker, JAst* ast)
{
  walk_command (Dst, walker, ast, -1, -1);
  j_walker_mark_detach (walker);
}

static void walk_ifclosure (Dst_DECL, JAst* ast)
{
  g_assert_not_reached ();
}

static gint adjust_stdfile (union _JInvokeStdfile* file, JAst* redirect, gint pipe)
{
  if (redirect != NULL)
    {
  #if DEVELOPER == 1
      g_assert (j_ast_n_children (redirect) == 1);
  #endif // DEVELOPER
      JAst* data_f = j_ast_find_child (redirect, J_AST_TYPE_DATA);
  #if DEVELOPER == 1
      g_assert (data_f != NULL);
      g_assert (j_ast_n_children (data_f) == 1);
  #endif // DEVELOPER
      JAst* data_b = j_ast_get_first_child (data_f);

      file->filename = (gchar*) data_b->data;
      return J_INVOKE_STD_FILE_TYPE_FILE;
    }
  else if (pipe >= 0)
    {
      file->fd = pipe;
      return J_INVOKE_STD_FILE_TYPE_PIPE;
    }
  else
    {
      file->filename = NULL;
      return J_INVOKE_STD_FILE_TYPE_FILE;
    }
}

static void walk_invoke (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe)
{
  JAst* arguments = NULL;
  JAst* redirect_in = NULL;
  JAst* redirect_out = NULL;
  JAst* target = NULL;
  JAst* child;

  for (child = j_ast_get_first_child (ast);
       child;
       child = j_ast_get_next_sibling (child))
    {
      switch (j_ast_get_type (child))
        {
          case J_AST_TYPE_ARGUMENTS: arguments = child; break;
          case J_AST_TYPE_BUILTIN: target = child; break;
          case J_AST_TYPE_REDIRECT_INPUT: redirect_in = child; break;
          case J_AST_TYPE_REDIRECT_OUTPUT_APPEND: redirect_out = child; break;
          case J_AST_TYPE_REDIRECT_OUTPUT_REPLACE: redirect_out = child; break;
          case J_AST_TYPE_TARGET: target = child; break;
          default: g_assert_not_reached ();
        }
    }

#if DEVELOPER == 1
  g_assert (arguments != NULL);
  g_assert (target != NULL);
#endif // DEVELOPER

  gint count = j_ast_n_children (arguments);
  JInvoke* invoke = j_invoke_new (count);
  JArgument* targ = & invoke->target;
  JArgument* args = & invoke->first_argument;
  guint i;

  for (child = j_ast_get_first_child (arguments), i = 0;
       child;
       child = j_ast_get_next_sibling (child), ++i)
    walk_argument (Dst, walker, child, & invoke->first_argument + i);
    walk_target (Dst, walker, target, invoke);

  if (redirect_out != NULL)
  switch (j_ast_get_type (redirect_out))
    {
      case J_AST_TYPE_REDIRECT_OUTPUT_APPEND: invoke->stdout_mode = J_INVOKE_STD_FILE_MODE_APPEND; break;
      case J_AST_TYPE_REDIRECT_OUTPUT_REPLACE: invoke->stdout_mode = J_INVOKE_STD_FILE_MODE_REPLACE; break;
      default: g_assert_not_reached ();
    }

  G_STMT_START
    {
      invoke->stdin_type = adjust_stdfile (& invoke->stdin, redirect_in, in_pipe);
      invoke->stdout_type = adjust_stdfile (& invoke->stdout, redirect_out, out_pipe);
      j_walker_add_invoke (walker, invoke);
    }
  G_STMT_END;
}

static void walk_pipe (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe)
{
  JAst *child1, *child2;
  gint cur_pipe;

  child1 = j_ast_get_first_child (ast);
  child2 = j_ast_get_next_sibling (child1);
  cur_pipe = j_walker_add_pipe (walker);
#if DEVELOPER == 1
  g_assert (j_ast_n_children (ast) == 2);
#endif // DEVELOPER
  walk_command (Dst, walker, child1, in_pipe, cur_pipe);
  walk_command (Dst, walker, child2, cur_pipe, out_pipe);
}

static void walk_scope (Dst_DECL, JAst* ast)
{
  JWalker walker = J_WALKER_INIT;
  JAst* child = NULL;

  for (child = j_ast_get_first_child (ast);
       child;
       child = j_ast_get_next_sibling (child))
    {
      switch (j_ast_get_type (child))
        {
          case J_AST_TYPE_DETACH:
            walk_detach (Dst, &walker, child);
            break;
          case J_AST_TYPE_IFCLOSURE:
            walk_ifclosure (Dst, child);
            continue;
          case J_AST_TYPE_INVOKE:
          case J_AST_TYPE_PIPE:
            walk_command (Dst, &walker, child, -1, -1);
            break;
          default: g_assert_not_reached ();
        }

      j_context_emit (Dst, &walker);
      j_walker_clear (&walker);
    }
}

static void walk_target (Dst_DECL, JWalker* walker, JAst* ast, JInvoke* invoke)
{
  JAst* child = j_ast_get_first_child (ast);
#if DEVELOPER == 1
  g_assert (j_ast_n_children (ast) == 1);
#endif // DEVELOPER

  switch (j_ast_get_type (child))
    {
      case J_AST_TYPE_BUILTIN:
        {
  #if DEVELOPER == 1
          g_assert (j_ast_n_children (child) == 1);
  #endif // DEVELOPER
          JAst* data_f = j_ast_find_child (child, J_AST_TYPE_DATA);
  #if DEVELOPER == 1
          g_assert (data_f != NULL);
          g_assert (j_ast_n_children (data_f) == 1);
  #endif // DEVELOPER
          JAst* data_b = j_ast_get_first_child (data_f);

          invoke->target_type = J_INVOKE_TARGET_TYPE_BUILTIN;
          invoke->target.builtin = (gchar*) data_b->data;
          break;
        }
      default:
        {
          invoke->target_type = J_INVOKE_TARGET_TYPE_REGULAR;
          walk_argument (Dst, walker, child, & invoke->target);
          break;
        }
    }
}
