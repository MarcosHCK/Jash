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
#include <bfd.h>
#include <codegen/debug/gdb.h>

G_GNUC_INTERNAL JGdb* _j_gdb_new (bfd* abfd);
#define builder_abfd(builder) (G_STRUCT_MEMBER (bfd*, ((builder)), G_STRUCT_OFFSET (JGdbBuilder, _private1_)))
#define builder_symbols(builder) (G_STRUCT_MEMBER (GPtrArray*, ((builder)), G_STRUCT_OFFSET (JGdbBuilder, _private2_)))
#define _bfd_close0(var) ((var == NULL) ? NULL : (var = (bfd_close (var), NULL)))
#define _g_ptr_array_unref0(var) ((var == NULL) ? NULL : (var = (g_ptr_array_unref (var), NULL)))
#define bfd_lasterr() (bfd_errmsg (bfd_get_error ()))

static bfd* _bfd_get_template (void) G_GNUC_CONST;
static bfd* _bfd_get_template (void)
{
  static bfd* __template__ = NULL;
  if (g_once_init_enter (&__template__))
    {
      static bfd templ = {0};
  
      if (bfd_init () != BFD_INIT_MAGIC)
        g_error ("(" G_STRLOC "): bfd_init ()!: %s\n", bfd_lasterr ());
      if (bfd_find_target ("default", &templ) == NULL)
        g_error ("(" G_STRLOC "): bfd_find_target ()!: %s\n", bfd_lasterr ());

      g_once_init_leave (&__template__, &templ);
    }
return __template__;
}

void j_gdb_builder_init (JGdbBuilder* builder)
{
  builder_abfd (builder) = bfd_create ("(stdin)", _bfd_get_template ());
  builder_symbols (builder) = g_ptr_array_new ();

  bfd* abfd = builder_abfd (builder);
  flagword file_flags = HAS_DEBUG | HAS_SYMS;
  flagword applicable_flags = file_flags & bfd_applicable_file_flags (abfd);

  if (!bfd_make_writable (abfd))
    g_error ("(" G_STRLOC "): bfd_make_writable ()!: %s", bfd_lasterr ());
  if (!bfd_set_arch_mach (abfd, j_gdb_default_arch, j_gdb_default_mach))
    g_error ("(" G_STRLOC "): bfd_set_arch_mach ()!: %s", bfd_lasterr ());
  if (!bfd_set_file_flags (abfd, applicable_flags))
    g_error ("(" G_STRLOC "): bfd_set_file_flags ()!: %s", bfd_lasterr ());
  if (!bfd_set_format (abfd, bfd_object))
    g_error ("(" G_STRLOC "): bfd_set_format ()!: %s", bfd_lasterr ());
}

void j_gdb_builder_clear (JGdbBuilder* builder)
{
  _bfd_close0 (builder_abfd (builder));
  _g_ptr_array_unref0 (builder_symbols (builder));
}

JGdbSymbol* j_gdb_builder_decl_function (JGdbBuilder* builder, const gchar* name, gpointer address, JGdbSection* section)
{
  bfd* abfd = builder_abfd (builder);
  GPtrArray* symbols = builder_symbols (builder);
  gsize length = strlen (name);
  gchar* symname = NULL;
  asymbol* sym = NULL;

  if ((sym = bfd_make_empty_symbol (abfd)) == NULL)
    g_error ("(" G_STRLOC "): bfd_make_empty_symbol ()!: %s", bfd_lasterr ());

  g_snprintf (symname = bfd_alloc (abfd, length + 2), length + 2, "%c%.*s",
                bfd_get_symbol_leading_char (abfd), (int) length, name);

  sym->flags = BSF_DEBUGGING | BSF_FUNCTION;
  sym->name = symname;
  sym->section = (asection*) section;
  sym->value = (symvalue) address;
return (g_ptr_array_add (symbols, sym), (JGdbSymbol*) sym);
}

JGdbSymbol* j_gdb_builder_decl_object (JGdbBuilder* builder, const gchar* name, gpointer address, gsize size, JGdbSection* section)
{
  asymbol* sym = (gpointer) j_gdb_builder_decl_function (builder, name, address, section);
            sym->flags = BSF_DEBUGGING | BSF_OBJECT;
return (JGdbSymbol*) sym;
}

JGdbSection* j_gdb_builder_decl_section (JGdbBuilder* builder, const gchar* name, gpointer address, gsize size)
{
  bfd* abfd = builder_abfd (builder);
  flagword section_flags = SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE | SEC_HAS_CONTENTS;
  flagword applicable_flags = section_flags & bfd_applicable_section_flags (abfd);
  asection* sec = NULL;

  if ((sec = bfd_make_section (abfd, name)) == NULL)
    g_error ("(" G_STRLOC "): bfd_make_section ()!: %s", bfd_lasterr ());
  if (!bfd_set_section_alignment (sec, GLIB_SIZEOF_VOID_P))
    g_error ("(" G_STRLOC "): bfd_set_section_alignment ()!: %s", bfd_lasterr ());
  if (!bfd_set_section_flags (sec, applicable_flags))
    g_error ("(" G_STRLOC "): bfd_set_section_flags ()!: %s", bfd_lasterr ());
  if (!bfd_set_section_size (sec, (bfd_size_type) size))
    g_error ("(" G_STRLOC "): bfd_set_section_size ()!: %s", bfd_lasterr ());
  if (!bfd_set_section_vma (sec, (bfd_vma) address))
    g_error ("(" G_STRLOC "): bfd_set_section_vma ()!: %s", bfd_lasterr ());
return (JGdbSection*) sec;
}

void j_gdb_builder_fill_section (JGdbBuilder* builder, gpointer address, gsize size, JGdbSection* section)
{
  if (!bfd_set_section_contents (builder_abfd (builder), (asection*) section, address, 0, size))
    g_error ("(" G_STRLOC "): bfd_set_section_contents ()!: %s", bfd_lasterr ());
}

JGdb* j_gdb_builder_end (JGdbBuilder* builder)
{
  bfd* abfd = g_steal_pointer (&builder_abfd (builder));
  GPtrArray* symbols = builder_symbols (builder);
  asymbol** syms = NULL;
  JGdb* gdb = NULL;

  syms = bfd_alloc (abfd, sizeof (asymbol*) * symbols->len);
#ifdef HAVE_MEMCPY
  memcpy (syms, symbols->pdata, sizeof (asymbol*) * symbols->len);
#else // !HAVE_MEMCPY
  guint i;
  for (i = 0; i < symbols->len; ++i)
    syms [i] = g_ptr_array_index (symbols, i);
#endif // HAVE_MEMCPY

  if (!bfd_set_symtab (abfd, syms, symbols->len))
    g_error ("(" G_STRLOC "): bfd_set_symtab ()!: %s", bfd_lasterr ());
return (gdb = _j_gdb_new (abfd), j_gdb_builder_clear (builder), gdb);
}
