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

#ifndef __INTELLISENSE__
|.actionlist actions
|.externnames extern_names
|.globals globl_
|.globalnames globl_names
|.section aux, code, data
|
|.type JClosure, JClosure
|.type JPipe, JPipe
|.type JPipeEnd, JPipeEnd
|.type gpointer, gpointer
|
|.define RetContinue, J_CLOSURE_STATUS_CONTINUE
|.define RetRemove, J_CLOSURE_STATUS_REMOVE
||
||void j_context_init (Dst_DECL)
||{
||  Dst->labels = g_new0 (gpointer, globl__MAX);
||  Dst->n_labels = globl__MAX;
||  Dst->nextpc = 0;
||  Dst->maxpc = 2;
||  Dst->max_expansions = 0;
||  Dst->symbols = g_hash_table_new (g_str_hash, g_str_equal);
||  Dst->strtab = g_hash_table_new (g_str_hash, g_str_equal);
#if DEVELOPER == 1
||  Dst->debug_info = g_hash_table_new (g_str_hash, g_str_equal);
||  j_gdb_builder_init (&Dst->debug_builder);
#endif // DEVELOPER
||
||  dasm_init (Dst, DASM_MAXSECTION);
||  dasm_setupglobal (Dst, Dst->labels, Dst->n_labels);
||  dasm_setup (Dst, actions);
||  dasm_growpc (Dst, Dst->maxpc);
||  g_queue_init (&Dst->detachables);
||
|   .code
|->__code_start:
|   .data
|->__data_start:
|   .code
||}
||
||void j_context_clear (Dst_DECL)
||{
#if DEVELOPER == 1
||  j_gdb_builder_clear (&Dst->debug_builder);
||  g_hash_table_remove_all (Dst->debug_info);
||  g_hash_table_unref (Dst->debug_info);
#endif // DEVELOPER
||  g_free (Dst->labels);
||  g_queue_clear (&Dst->detachables);
||  g_hash_table_remove_all (Dst->symbols);
||  g_hash_table_unref (Dst->symbols);
||  g_hash_table_remove_all (Dst->strtab);
||  g_hash_table_unref (Dst->strtab);
||  dasm_free (Dst);
||}
||
||/* Defined at the bottom of the file */
||void j_context_emit_debuginfo (Dst_DECL);
||
||static void finish_code_und (Dst_DECL, GHashTableIter* iter)
||{
||  JTag tag;
||  JOnceID* once;
||  const JOnceInit* init;
||
||  while (g_hash_table_iter_next (iter, (gpointer*) &once, &tag))
||  {
|=>(j_tag_as_pc (&tag)):
||
||    if ((init = j_once_lookup (once, strlen (once))) != NULL)
||      init->callback (Dst);
||    else
||      g_error ("(" G_STRLOC "): Unknown once '%s'", once);
||  }
||}
||
||static void finish_data_und (Dst_DECL, GHashTableIter* iter)
||{
||  JTag tag;
||  JOnceID* once;
||
||  while (g_hash_table_iter_next (iter, (gpointer*) &once, &tag))
||  {
|=>(j_tag_as_pc (&tag)):
||    j_context_store (Dst, once, strlen (once) + 1);
||  }
||}
||
||void j_context_finish (Dst_DECL)
||{
||  GHashTableIter iter;
||
|.code
||  g_hash_table_iter_init (&iter, Dst->symbols);
||  finish_code_und (Dst, &iter);
|->__code_end:
||
|.data
||  g_hash_table_iter_init (&iter, Dst->strtab);
||  finish_data_und (Dst, &iter);
|->__data_end:
||}
||
||void j_context_store (Dst_DECL, gconstpointer buffer, gsize bufsz)
||{
||  const gsize n_dwords = (bufsz / 4);
||  const guint32* dwords = & G_STRUCT_MEMBER (guint32, buffer, 0);
||  const gsize n_bytes = (bufsz % 4);
||  const guint8* bytes = & G_STRUCT_MEMBER (guint8, buffer, n_dwords * 4);
||  gsize i;
||
||  for (i = 0; i < n_dwords; i++)
||    {
|       .dword (GUINT32_TO_LE (dwords [i]))
||    }
||
||  for (i = 0; i < n_bytes; i++)
||    {
|       .byte (bytes [i])
||    }
||}
||
#if DEVELOPER == 1
||
||void j_context_emit_debuginfo (Dst_DECL)
||{
||  JGdbSection* code = NULL;
||  gpointer code_start = Dst->labels [globl___code_start];
||  gpointer code_end = Dst->labels [globl___code_end];
||  gsize code_size = code_end - code_start;
||  JGdbSection* data = NULL;
||  gpointer data_start = Dst->labels [globl___data_start];
||  gpointer data_end = Dst->labels [globl___data_end];
||  gsize data_size = data_end - data_start;
||  JGdbSymbol* symbol;
||  guint i;
||
||  g_assert (code_end >= code_start);
||  g_assert (data_end >= data_start);
||
||  code = j_gdb_builder_decl_section_as_code (&Dst->debug_builder, "code", code_start, code_size);
||  data = j_gdb_builder_decl_section_as_data (&Dst->debug_builder, "data", data_start, data_size);
||
||  for (i = 0; i < globl__MAX; ++i) switch (i)
||    {
||      case globl___code_start: j_gdb_builder_decl_section_symbol (&Dst->debug_builder, "__code_start", code_start, code); break;
||      case globl___data_start: j_gdb_builder_decl_section_symbol (&Dst->debug_builder, "__data_start", data_start, data); break;
||      case globl___code_end: j_gdb_builder_decl_symbol (&Dst->debug_builder, "__code_end", code_start + code_size, code); break;
||      case globl___data_end: j_gdb_builder_decl_symbol (&Dst->debug_builder, "__data_end", data_start + data_size, data); break;
||
||      default:
||        {
||          gpointer base = (gpointer) Dst->labels [i];
||          gpointer name = (gpointer) globl_names [i];
||
||          if (base == NULL) continue;
||          else if (base >= code_start && code_end > base) j_gdb_builder_decl_function (&Dst->debug_builder, name, base, code);
||          else if (base >= data_start && data_end > base) j_gdb_builder_decl_object (&Dst->debug_builder, name, base, 0, data);
||          else g_assert_not_reached ();
||          break;
||        }
||    }
||
||  j_gdb_builder_fill_section (&Dst->debug_builder, code_start, code_size, code);
||  j_gdb_builder_fill_section (&Dst->debug_builder, data_start, data_size, data);
||}
||
#endif // DEVELOPER
#endif // __INTELLISENSE__
