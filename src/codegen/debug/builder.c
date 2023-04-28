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
#include <bfd.h>
#include <codegen/debug/gdb.h>
#include <glib.h>

# define OUTPUT_ON_FILE (0)
#if OUTPUT_ON_FILE
# define OUTPUT_FILE "closure_gdb"
#endif // OUTPUT_ON_FILE

G_GNUC_INTERNAL JGdb* _j_gdb_new (GBytes* bytes);
#define builder_abfd(builder) (G_STRUCT_MEMBER (bfd*, ((builder)), G_STRUCT_OFFSET (JGdbBuilder, _private1_)))
#define builder_symbols(builder) (G_STRUCT_MEMBER (GPtrArray*, ((builder)), G_STRUCT_OFFSET (JGdbBuilder, _private2_)))
#define _bfd_close0(var) ((var == NULL) ? NULL : (var = (bfd_close (var), NULL)))
#define _g_ptr_array_unref0(var) ((var == NULL) ? NULL : (var = (g_ptr_array_unref (var), NULL)))
#define __g_close0(var) ((var == -1) ? -1 : (var = (_g_close (var), -1)))
#define bfd_lasterr() (bfd_errmsg (bfd_get_error ()))

static inline bfd* bfd_get_template (void) G_GNUC_CONST;
static inline bfd* bfd_get_template (void)
{
  static bfd* __template__ = NULL;
  if (g_once_init_enter (&__template__))
    {
      static bfd templ = {0};
      if (bfd_init () != BFD_INIT_MAGIC)
        g_error ("(" G_STRLOC"): bfs_init()!: %s", bfd_lasterr ());
      if (bfd_find_target ("default", &templ) == 0)
        g_error ("(" G_STRLOC "): bfd_find_target()!: %s", bfd_lasterr ());
      g_once_init_leave (&__template__, &templ);
    }
return (__template__);
}

static inline gchar* bfd_strdup (bfd* abfd, const gchar* src) G_GNUC_MALLOC;
static inline gchar* bfd_strdup (bfd* abfd, const gchar* src)
{
  gsize length = strlen (src);
  gchar* dst = bfd_alloc (abfd, length +1);
#if HAVE_MEMCPY
  memcpy (dst, src, length + 1);
#else // !HAVE_MEMCPY
  g_strlcpy (dst, src, length + 1);
#endif // HAVE_MEMCPY
return dst;
}

static inline gchar* bfd_strdup_printf (bfd* abfd, const gchar* fmt, ...) G_GNUC_PRINTF (2, 3);
static inline gchar* bfd_strdup_printf (bfd* abfd, const gchar* fmt, ...)
{
  va_list l, l2;
  va_start (l, fmt);

  G_VA_COPY (l2, l);

  gsize length = g_printf_string_upper_bound (fmt, l);
  gchar* value = bfd_alloc (abfd, ++length);
  gint done = g_vsnprintf (value, length, fmt, l2);
return (va_end (l), va_end (l2), value);
}

void j_gdb_builder_init (JGdbBuilder* builder)
{
#if OUTPUT_ON_FILE
  builder_abfd (builder) = bfd_openw (OUTPUT_FILE, "default");
#else // OUTPUT_ON_FILE
  builder_abfd (builder) = bfd_create ("(stdin)", bfd_get_template ());
#endif // OUTPUT_ON_FILE
  builder_symbols (builder) = g_ptr_array_new ();

  if (G_UNLIKELY (builder_abfd (builder) == NULL))
    g_error ("(" G_STRLOC "): bfd_create ()!: %s", bfd_lasterr ());

  bfd* abfd = builder_abfd (builder);
  const flagword file_flags = D_PAGED | HAS_DEBUG | HAS_LOCALS | HAS_SYMS | WP_TEXT;
  const flagword applicable_flags = file_flags & bfd_applicable_file_flags (abfd);

#if OUTPUT_ON_FILE
  if (!bfd_set_format (abfd, bfd_object))
    g_error ("(" G_STRLOC "): bfd_set_format ()!: %s", bfd_lasterr ());
#else // OUTPUT_ON_FILE
  if (!bfd_make_writable (abfd))
    g_error ("(" G_STRLOC "): bfd_make_writable ()!: %s", bfd_lasterr ());
#endif // OUTPUT_ON_FILE
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

void j_gdb_builder_decl_entry (JGdbBuilder* builder, gpointer address)
{
  bfd* abfd = builder_abfd (builder);
  bfd_set_start_address (abfd, (bfd_vma) address);
}

JGdbSymbol* j_gdb_builder_decl_function (JGdbBuilder* builder, const gchar* name, gpointer address, JGdbSection* section)
{
  bfd* abfd = builder_abfd (builder);
  GPtrArray* symbols = builder_symbols (builder);
  gsize length = strlen (name);
  gsize alloc = length + 2 + 9;
  gchar* symname = NULL;
  asymbol* sym = NULL;
  gchar led = 0;

  if ((sym = bfd_make_empty_symbol (abfd)) == NULL)
    g_error ("(" G_STRLOC "): bfd_make_empty_symbol ()!: %s", bfd_lasterr ());

  sym->flags = BSF_FUNCTION | BSF_LOCAL;
  sym->name = ((led = bfd_get_symbol_leading_char (abfd)) == 0) ? bfd_strdup (abfd, name) : bfd_strdup_printf (abfd, "%c%s", led, name);
  sym->section = & G_STRUCT_MEMBER (asection, section, 0);
  sym->value = GPOINTER_TO_UINT (address - G_STRUCT_MEMBER (bfd_vma, section, G_STRUCT_OFFSET (asection, vma)));
return (g_ptr_array_add (symbols, sym), (JGdbSymbol*) sym);
}

JGdbSymbol* j_gdb_builder_decl_object (JGdbBuilder* builder, const gchar* name, gpointer address, gsize size, JGdbSection* section)
{
  asymbol* sym = (gpointer) j_gdb_builder_decl_function (builder, name, address, section);
            sym->flags = BSF_LOCAL | BSF_OBJECT;
return (JGdbSymbol*) sym;
}

static JGdbSection* j_gdb_builder_decl_section (JGdbBuilder* builder, const gchar* name, gpointer address, gsize size, flagword section_flags)
{
  bfd* abfd = builder_abfd (builder);
  flagword applicable_flags = section_flags & bfd_applicable_section_flags (abfd);
  asection* sec = NULL;

  if ((sec = bfd_make_section (abfd, bfd_strdup (abfd, name))) == NULL)
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

JGdbSection* j_gdb_builder_decl_section_as_code (JGdbBuilder* builder, const gchar* name, gpointer address, gsize size)
{
  return j_gdb_builder_decl_section (builder, name, address, size, SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE | SEC_HAS_CONTENTS);
}

JGdbSection* j_gdb_builder_decl_section_as_data (JGdbBuilder* builder, const gchar* name, gpointer address, gsize size)
{
  return j_gdb_builder_decl_section (builder, name, address, size, SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_DATA | SEC_HAS_CONTENTS);
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
  gchar* name = abfd->usrdata;
  GBytes* bytes = NULL;
  JGdb* gdb = NULL;

  if (!bfd_set_symtab (abfd, (asymbol**) symbols->pdata, symbols->len))
    g_error ("(" G_STRLOC "): bfd_set_symtab ()!: %s", bfd_lasterr ());
  if (!bfd_close (abfd))
    g_error ("(" G_STRLOC "): bfd_close ()!: %s", bfd_lasterr ());

  static const gchar __null__ [64] = {0};
  bytes = g_bytes_new_static (&__null__, sizeof (__null__));
return (gdb = _j_gdb_new (bytes), g_bytes_unref (bytes), gdb);
}
