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
#include <config.h>
#include <codegen/codegen.h>
#include <codegen/context.h>
#include <codegen/walker.h>

static void walk_argument (Dst_DECL, JWalker* walker, JAst* ast, JArgument* argument);
static void walk_command (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe);
static void walk_expression (Dst_DECL, JAst* ast, const JTag* tag, const JTag* tag_next);
static void walk_detach (Dst_DECL, JAst* ast, const JTag* tag, const JTag* tag_next);
static void walk_ifclosure (Dst_DECL, JAst* ast, const JTag* tag, const JTag* tag_next);
static void walk_invoke (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe);
static void walk_logical (Dst_DECL, JAst* ast, const JTag* tag, const JTag* tag_next);
static void walk_pipe (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe);
static void walk_scope (Dst_DECL, JAst* ast, const JTag* tag, const JTag* tag_next);
static void walk_target (Dst_DECL, JWalker* walker, JAst* ast, JInvoke* invoke);

void j_context_generate (Dst_DECL, JAst* ast, const JTag* tag)
{
  JTag tag_last = {0};

  j_tag_init (Dst, &tag_last);
  walk_scope (Dst, ast, tag, &tag_last);
  j_context_emit_chain_last (Dst, &tag_last);
}

static void walk_argument (Dst_DECL, JWalker* walker, JAst* ast, JArgument* argument)
{
  switch (j_ast_get_ast_type (ast))
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
          JTag tag_head, tag_last;

          j_tag_init (Dst, &tag_head);
          j_tag_init (Dst, &tag_last);
          walk_scope (Dst, ast, &tag_head, &tag_last);
          j_context_emit_chain_last_and_report (Dst, 0, &tag_last);

          argument->type = J_ARGUMENT_TYPE_EXPANSION;
          argument->index = j_walker_add_expansion (walker, &tag_head);
          break;
        }
      default: g_assert_not_reached ();
    }
}

static void walk_command (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe)
{
  switch (j_ast_get_ast_type (ast))
    {
      case J_AST_TYPE_INVOKE: walk_invoke (Dst, walker, ast, in_pipe, out_pipe); break;
      case J_AST_TYPE_PIPE: walk_pipe (Dst, walker, ast, in_pipe, out_pipe); break;
      default: g_assert_not_reached ();
    }
}

static void walk_expression (Dst_DECL, JAst* ast, const JTag* tag, const JTag* tag_next)
{
  switch (j_ast_get_ast_type (ast))
  {
    case J_AST_TYPE_LOGICAL_AND:
    case J_AST_TYPE_LOGICAL_OR:
      walk_logical (Dst, ast, tag, tag_next);
      break;
    case J_AST_TYPE_INVOKE:
    case J_AST_TYPE_PIPE:
      {
        JWalker walker = J_WALKER_INIT;

        walk_command (Dst, &walker, ast, -1, -1);
        j_context_emit_chain_step (Dst, &walker, tag, tag_next);
        j_walker_clear (&walker);
        break;
      }
    default: g_assert_not_reached ();
  }
}

static void walk_detach (Dst_DECL, JAst* ast, const JTag* tag, const JTag* tag_next)
{
  guint index = g_queue_get_length (&Dst->detachables);

  j_context_emit_chain_step_detach (Dst, index, tag, tag_next);
  g_queue_push_tail (&Dst->detachables, ast);
}

static void walk_ifclosure (Dst_DECL, JAst* ast, const JTag* tag, const JTag* tag_next)
{
  JAst* condition = j_ast_find_child (ast, J_AST_TYPE_IFCLOSURE_CONDITION);
  JAst* direct = j_ast_find_child (ast, J_AST_TYPE_IFCLOSURE_DIRECT);
  JAst* reverse = j_ast_find_child (ast, J_AST_TYPE_IFCLOSURE_REVERSE);
  JTag tag_condition, tag_direct, tag_reverse, tag_test;
#if DEVELOPER == 1
  g_assert (condition != NULL);
#endif // DEVELOPER

  j_tag_init (Dst, &tag_condition);
  j_tag_init (Dst, &tag_direct);
  j_tag_init (Dst, &tag_reverse);
  j_tag_init (Dst, &tag_test);

  walk_scope (Dst, condition, tag, &tag_condition);
  j_context_emit_test (Dst, &tag_condition, &tag_direct, &tag_reverse);

  if (direct != NULL) walk_scope (Dst, direct, &tag_direct, tag_next);
  else j_context_emit_chain_empty (Dst, &tag_direct, tag_next);
  if (reverse != NULL) walk_scope (Dst, reverse, &tag_reverse, tag_next);
  else j_context_emit_chain_empty (Dst, &tag_reverse, tag_next);
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
      switch (j_ast_get_ast_type (child))
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
    walk_argument (Dst, walker, child, args + i);
    walk_target (Dst, walker, target, invoke);

  if (redirect_out != NULL)
  switch (j_ast_get_ast_type (redirect_out))
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

static void walk_logical (Dst_DECL, JAst* ast, const JTag* tag, const JTag* tag_next)
{
#if DEVELOPER == 1
  g_assert (j_ast_n_children (ast) == 2);
#endif // DEVELOPER
  JAst* child1 = j_ast_get_first_child (ast);
  JAst* child2 = j_ast_get_next_sibling (child1);
  JTag tag_condition, tag_direct, tag_reverse, tag_test;

  j_tag_init (Dst, &tag_condition);
  j_tag_init (Dst, &tag_direct);
  j_tag_init (Dst, &tag_reverse);
  j_tag_init (Dst, &tag_test);

  walk_expression (Dst, child1, tag, &tag_condition);
  j_context_emit_test (Dst, &tag_condition, &tag_direct, &tag_reverse);

  switch (j_ast_get_ast_type (ast))
  {
    case J_AST_TYPE_LOGICAL_AND:
      j_context_emit_chain_empty (Dst, &tag_reverse, tag_next);
      walk_expression (Dst, child2, &tag_direct, tag_next);
      break;
    case J_AST_TYPE_LOGICAL_OR:
      j_context_emit_chain_empty (Dst, &tag_direct, tag_next);
      walk_expression (Dst, child2, &tag_reverse, tag_next);
      break;
    default: g_assert_not_reached ();
  }
}

static void walk_pipe (Dst_DECL, JWalker* walker, JAst* ast, gint in_pipe, gint out_pipe)
{
#if DEVELOPER == 1
  g_assert (j_ast_n_children (ast) == 2);
#endif // DEVELOPER
  JAst* child1 = j_ast_get_first_child (ast);
  JAst* child2 = j_ast_get_next_sibling (child1);
  gint cur_pipe = j_walker_add_pipe (walker);

  walk_command (Dst, walker, child1, in_pipe, cur_pipe);
  walk_command (Dst, walker, child2, cur_pipe, out_pipe);
}

static void walk_scope (Dst_DECL, JAst* ast, const JTag* tag_head, const JTag* tag_last)
{
  JTag tag = {0};
  JTag tag_next = {0};
  JAst* child = NULL;

  j_tag_copy (tag_head, &tag);

  for (child = j_ast_get_first_child (ast);
       child;
       child = j_ast_get_next_sibling (child))
    {
      if (j_ast_get_next_sibling (child) != NULL)
        j_tag_init (Dst, &tag_next);
      else
        j_tag_copy (tag_last, &tag_next);

      switch (j_ast_get_ast_type (child))
        {
          case J_AST_TYPE_DETACH:
            walk_detach (Dst, child, &tag, &tag_next);
            break;
          case J_AST_TYPE_IFCLOSURE:
            walk_ifclosure (Dst, child, &tag, &tag_next);
            break;
          case J_AST_TYPE_INVOKE:
          case J_AST_TYPE_LOGICAL_AND:
          case J_AST_TYPE_LOGICAL_OR:
          case J_AST_TYPE_PIPE:
            walk_expression (Dst, child, &tag, &tag_next);
            break;
          default: g_assert_not_reached ();
        }

      j_tag_copy (&tag_next, &tag);
    }
}

static void walk_target (Dst_DECL, JWalker* walker, JAst* ast, JInvoke* invoke)
{
  JAst* child = j_ast_get_first_child (ast);
#if DEVELOPER == 1
  g_assert (j_ast_n_children (ast) == 1);
#endif // DEVELOPER

  switch (j_ast_get_ast_type (ast))
    {
      case J_AST_TYPE_BUILTIN:
        {
  #if DEVELOPER == 1
          g_assert (j_ast_get_ast_type (child) == J_AST_TYPE_DATA);
          g_assert (j_ast_n_children (child) == 1);
  #endif // DEVELOPER
          invoke->target_type = J_INVOKE_TARGET_TYPE_BUILTIN;
          invoke->target.builtin = j_ast_get_first_child (child)->data;
          break;
        }
      case J_AST_TYPE_TARGET:
        {
          invoke->target_type = J_INVOKE_TARGET_TYPE_REGULAR;
          walk_argument (Dst, walker, child, & invoke->target);
          break;
        }
      default: g_assert_not_reached ();
    }
}
