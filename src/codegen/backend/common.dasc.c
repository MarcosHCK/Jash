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

typedef gint JPipe [2];

# if __CODEGEN__
|.actionlist actions
|.externnames extern_names
|.globals globl_
|.globalnames globl_names
|.section aux, code, data

|.type gpointer, gpointer
|.type JClosure, JClosure
|.type JPipe, JPipe

|.define RetContinue, J_CLOSURE_STATUS_CONTINUE
|.define RetRemove, J_CLOSURE_STATUS_REMOVE
#else
# define DASM_MAXSECTION (0)
# define globl_entry (J_CONTEXT_LABEL_MAIN)
# define globl___aux_start (0)
# define globl___code_start (0)
# define globl___data_start (0)
# define globl___code_end (0)
# define globl___aux_end (0)
# define globl___data_end (0)
# define globl__MAX (0)
static const char* const globl_names [];
static const char* const extern_names [];
static const unsigned char actions [];
# endif // __CODEGEN__

G_STATIC_ASSERT (J_CONTEXT_FIRST_STAGE == 0);

void j_context_init (Dst_DECL)
{
  Dst->labels = g_new0 (gpointer, globl__MAX);
  Dst->n_labels = globl__MAX;
  Dst->nextstage = 0;
  Dst->nextpc = 1;
  Dst->maxpc = 2;
  Dst->expansions = g_ptr_array_new ();
  Dst->onces = g_hash_table_new (g_str_hash, g_str_equal);
  Dst->strtab = g_hash_table_new (g_str_hash, g_str_equal);
#if DEVELOPER == 1
  Dst->symbols = g_hash_table_new (g_str_hash, g_str_equal);
  j_gdb_builder_init (&Dst->debug_builder);
#endif // DEVELOPER

  dasm_init (Dst, DASM_MAXSECTION);
  dasm_setupglobal (Dst, Dst->labels, Dst->n_labels);
  dasm_setup (Dst, actions);
  dasm_growpc (Dst, Dst->maxpc);

#if __CODEGEN__
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
  g_hash_table_remove_all (Dst->symbols);
  g_hash_table_unref (Dst->symbols);
#endif // DEVELOPER
  g_free (Dst->labels);
  g_ptr_array_unref (Dst->expansions);
  g_hash_table_remove_all (Dst->onces);
  g_hash_table_unref (Dst->onces);
  g_hash_table_remove_all (Dst->strtab);
  g_hash_table_unref (Dst->strtab);
  dasm_free (Dst);
}

static inline void j_context_complete_common (Dst_DECL)
{
#if __CODEGEN__
  |.aux
  |->__aux_end:
  |.code
  |->__code_end:
  |.data
  |->__data_end:
#endif // __CODEGEN__
}

#if DEVELOPER == 1

void j_context_debug_build (Dst_DECL)
{
  JGdbSection* aux = NULL;
  gpointer aux_start = Dst->labels [globl___aux_start];
  gpointer aux_end = Dst->labels [globl___aux_end];
  gssize aux_size = aux_end - aux_start;
  JGdbSection* code = NULL;
  gpointer code_start = Dst->labels [globl___code_start];
  gpointer code_end = Dst->labels [globl___code_end];
  gssize code_size = code_end - code_start;
  JGdbSection* data = NULL;
  gpointer data_start = Dst->labels [globl___data_start];
  gpointer data_end = Dst->labels [globl___data_end];
  gssize data_size = data_end - data_start;
  JGdbSymbol* symbol;
  guint i;

  g_assert (aux_start < aux_end);
  g_assert (code_start < code_end);
  g_assert (data_start < data_end);

  aux = j_gdb_builder_decl_section_as_code (&Dst->debug_builder, "aux", aux_start, aux_size);
  code = j_gdb_builder_decl_section_as_code (&Dst->debug_builder, "code", code_start, code_size);
  data = j_gdb_builder_decl_section_as_data (&Dst->debug_builder, "data", data_start, data_size);

  for (i = 0; i < globl__MAX; ++i)
    {
      gpointer base = (gpointer) Dst->labels [i];
      gpointer name = (gpointer) globl_names [i];

      if (base == NULL) continue;
      else if (aux_end >= base && base >= aux_start) j_gdb_builder_decl_function (&Dst->debug_builder, name, base, aux);
      else if (code_end >= base && base >= code_start) j_gdb_builder_decl_function (&Dst->debug_builder, name, base, code);
      else if (data_end >= base && base >= data_start) j_gdb_builder_decl_object (&Dst->debug_builder, name, base, 0, data);
      else g_assert_not_reached ();
    }

  j_gdb_builder_fill_section (&Dst->debug_builder, aux_start, aux_size, aux);
  j_gdb_builder_fill_section (&Dst->debug_builder, code_start, code_size, code);
  j_gdb_builder_fill_section (&Dst->debug_builder, data_start, data_size, data);
}

#endif // DEVELOPER
