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
#include <codegen_common.h>

#ifdef __CODEGEN__
|.arch x64
|.section code

|.actionlist actions
|.externnames extern_names
|.globals globl_
|.globalnames globl_names

|.define Jash, [rsp + (sizeof (gpointer) * 0)]
|.define Error, [rsp + (sizeof (gpointer) * 1)]
|.define RetContinue, 1
|.define RetRemove, 0
#else
# define DASM_MAXSECTION (0)
# define globl_main (J_CODEGEN_LABEL_MAIN)
# define globl__MAX (0)
static const unsigned char actions[];
static const char* const globl_names[];
static const char* const extern_names[];
#endif // __CODEGEN__
#include <dynasm/dasm_x86.h>

#define stacksz_r \
  ( \
    0 \
    + sizeof (gpointer) /* jash */ \
    + sizeof (gpointer) /* error */ \
  )

static void walk_command (Dst_DECL, Ast ast);
static void walk_ifclosure (Dst_DECL, Ast ast);
static void walk_invoke (Dst_DECL, Ast ast);
static void walk_logical (Dst_DECL, Ast ast);
static void walk_pipe (Dst_DECL, Ast ast);
static void walk_scope (Dst_DECL, Ast ast);
G_STATIC_ASSERT (J_CODEGEN_LABEL_MAIN == globl_main);

static void dumpbuffer (Dst_DECL, gconstpointer data, gsize buffsz)
{
  gsize n_dwords = (buffsz / 4);
  gsize n_bytes = (buffsz % 4);
  gsize i;

  const guint32* dwords = & G_STRUCT_MEMBER (guint32, data, 0);
  const guint8* bytes = & G_STRUCT_MEMBER (guint8, data, n_dwords * 4);

  for (i = 0; i < n_dwords; i++)
    {
#if __CODEGEN__
      |.dword (dwords [i])
#endif // __CODEGEN__
    }

  for (i = 0; i < n_bytes; i++)
    {
#if __CODEGEN__
      |.byte (bytes [i])
#endif // __CODEGEN__
    }
}

void j_codegen_init (Dst_DECL)
{
  Dst->n_labels = globl__MAX;
  Dst->labels = g_new (gpointer, globl__MAX);
  Dst->nextpc = 0;
  Dst->maxpc = 2;

  dasm_init (Dst, DASM_MAXSECTION);
  dasm_setupglobal (Dst, Dst->labels, globl__MAX);
  dasm_setup (Dst, actions);
  dasm_growpc (Dst, Dst->maxpc);
}

void j_codegen_absjump (Dst_DECL, gconstpointer value)
{
#if __CODEGEN__
  |.code
  |->main:
  | mov64 rax, ((guint64) value)
  | jmp rax
#endif // __CODEGEN__
}

void j_codegen_prologue (Dst_DECL)
{
#if __CODEGEN__
  |.code
  |->main:
  | sub rsp, (stacksz_r)
# ifdef G_OS_UNIX
  | mov Jash, rdi
  | mov Error, rsi
# else // !G_OS_UNIX
  | mov Jash, rcx
  | mov Error, rdx
# endif // G_OS_UNIX
#endif // __CODEGEN__
}

void j_codegen_epilogue (Dst_DECL)
{
#if __CODEGEN__
  | add rsp, (stacksz_r)
  | mov rax, RetRemove
  | ret
#endif // __CODEGEN__
}

static void walk_command (Dst_DECL, Ast ast)
{
  switch (ast_type (ast))
  {
    case AST_INVOKE: walk_invoke (Dst, ast); break;
    case AST_LOGICAL_AND: walk_logical (Dst, ast); break;
    case AST_LOGICAL_OR: walk_logical (Dst, ast); break;
    case AST_PIPE: walk_pipe (Dst, ast); break;
    default: g_assert_not_reached ();
  }
}

static void walk_ifclosure (Dst_DECL, Ast ast)
{
  Ast condition;
  Ast direct;
  Ast reverse;

  int directpc = j_codegen_allocpc (Dst);
  int reversepc = j_codegen_allocpc (Dst);
  int completepc = j_codegen_allocpc (Dst);

  if ((condition = ast_find_child (ast, AST_IFCLOSURE_CONDITION)) == NULL)
    g_assert_not_reached ();
  if ((direct = ast_find_child (ast, AST_IFCLOSURE_DIRECT)) == NULL)
    g_assert_not_reached ();
  if ((reverse = ast_find_child (ast, AST_IFCLOSURE_REVERSE)) == NULL)
    g_assert_not_reached ();
  if (ast_first (condition) == NULL || ast_next (ast_first (condition)) != NULL)
    g_assert_not_reached ();

  walk_command (Dst, ast_first (condition));

#if __CODEGEN__
  |=>(directpc):
#endif // __CODEGEN__

  walk_scope (Dst, direct);

#if __CODEGEN__
  | jmp =>completepc
  |=>(reversepc):
#endif // __CODEGEN__

  walk_scope (Dst, reverse);

#if __CODEGEN__
  |=>(completepc):
#endif // __CODEGEN__
}

static void walk_invoke (Dst_DECL, Ast ast)
{
  Ast arguments;
  Ast redirect_input;
  Ast redirect_output;
  Ast target;

  guint got = 2;

  if ((arguments = ast_find_child (ast, AST_ARGUMENTS)) == NULL)
    g_assert_not_reached ();
  if ((redirect_input = ast_find_child (ast, AST_REDIRECT_INPUT)) != NULL) ++got;
  if ((redirect_output = ast_find_child (ast, AST_REDIRECT_OUTPUT_APPEND)) != NULL) ++got; else
  if ((redirect_output = ast_find_child (ast, AST_REDIRECT_OUTPUT_REPLACE)) != NULL) ++got;
  if ((target = ast_find_child (ast, AST_BUILTIN)) == NULL)
  if ((target = ast_find_child (ast, AST_TARGET)) == NULL)
    g_assert_not_reached ();

  if (got != ast_n_children (ast))
    g_assert_not_reached ();
}

static void walk_logical (Dst_DECL, Ast ast)
{
  Ast child1;
  Ast child2;

  int complete = j_codegen_allocpc (Dst);

  if ((child1 = ast_first (ast)) == NULL)
    g_assert_not_reached ();
  if ((child2 = ast_next (child1)) == NULL)
    g_assert_not_reached ();
  if (ast_next (child2) != NULL)
    g_assert_not_reached ();

  walk_command (Dst, child1);
  walk_command (Dst, child2);

#if __CODEGEN__
  |=>(complete):
#endif // __CODEGEN__
}

static void walk_pipe (Dst_DECL, Ast ast)
{
  Ast child1;
  Ast child2;

  if ((child1 = ast_first (ast)) == NULL)
    g_assert_not_reached ();
  if ((child2 = ast_next (child1)) == NULL)
    g_assert_not_reached ();
  if (ast_n_children (ast) != 2)
    g_assert_not_reached ();
}

static void walk_scope (Dst_DECL, Ast ast)
{
  Ast child = NULL;

  for (child = ast_first (ast);
       child != NULL;
       child = ast_next (child))
  switch (ast_type (child))
    {
      case AST_INVOKE:
      case AST_LOGICAL_AND:
      case AST_LOGICAL_OR:
      case AST_PIPE:
        walk_command (Dst, child);
        break;
      case AST_IFCLOSURE:
        walk_ifclosure (Dst, child);
        break;
      default: g_assert_not_reached ();
    }
}

void j_codegen_generate (Dst_DECL, Ast ast)
{
  g_assert (ast_type (ast) == AST_SCOPE);
  walk_scope (Dst, ast);
}
