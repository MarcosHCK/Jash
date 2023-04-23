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

# if __CODEGEN__
|.actionlist actions
|.externnames extern_names
|.globals globl_
|.globalnames globl_names
|.section aux, code, data

|.type Jc, JClosure
|.type gint, gint
|.type gpointer, gpointer

|.define RetContinue, J_CLOSURE_STATUS_CONTINUE
|.define RetRemove, J_CLOSURE_STATUS_REMOVE
#else
# define DASM_MAXSECTION (0)
# define globl_entry (J_CONTEXT_LABEL_MAIN)
# define globl__MAX (0)
static const char* const globl_names [];
static const char* const extern_names [];
static const unsigned char actions [];
# endif // __CODEGEN__

void j_context_init (Dst_DECL)
{
  Dst->labels = g_new0 (gpointer, globl__MAX);
  Dst->n_labels = globl__MAX;
  Dst->nextstage = 0;
  Dst->nextpc = 1;
  Dst->maxpc = 2;
  Dst->expansions = g_ptr_array_new ();
  Dst->symbols = g_hash_table_new (g_str_hash, g_str_equal);
  Dst->strtab = g_hash_table_new (g_str_hash, g_str_equal);

#if DEVELOPER == 1
  j_gdb_builder_init (&Dst->debug_builder);
#endif // DEVELOPER

  dasm_init (Dst, DASM_MAXSECTION);
  dasm_setupglobal (Dst, Dst->labels, Dst->n_labels);
  dasm_setup (Dst, actions);
  dasm_growpc (Dst, Dst->maxpc);

#if __CODEGEN__
  |.code
  |->entry:
  |.aux
  |->__aux_start:
  |.code
  |->__code_start:
  |.data
  |->__data_start:
  |.code
#endif // __CODEGEN__
}

void j_context_clear (Dst_DECL)
{
#if DEVELOPER == 1
  j_gdb_builder_clear (&Dst->debug_builder);
#endif // DEVELOPER
  g_free (Dst->labels);
  g_ptr_array_unref (Dst->expansions);
  g_hash_table_remove_all (Dst->symbols);
  g_hash_table_unref (Dst->symbols);
  g_hash_table_remove_all (Dst->strtab);
  g_hash_table_unref (Dst->strtab);
  dasm_free (Dst);
}

static inline void j_context_complete_common (Dst_DECL)
{
#if __CODEGEN__
  |.code
  |->__code_end:
  |.aux
  |->__aux_end:
  |.data
  |->__data_end:
#endif // __CODEGEN__
}
