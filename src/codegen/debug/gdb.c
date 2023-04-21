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
#include <codegen/codegen.h>
#include <codegen/context.h>
#include <codegen/debug/gdb.h>
#include <gmodule.h>

typedef struct _JGdbGlobalModule JGdbGlobalModule;
typedef struct _JGdbGlobalDescriptor JGdbGlobalDescriptor;
G_MODULE_EXPORT JGdbGlobalDescriptor __jit_debug_descriptor;
G_MODULE_EXPORT void __jit_debug_register_code ();
static G_LOCK_DEFINE (global_lock);

typedef enum
{
  J_GDB_GLOBAL_ACTION_NONE = 0,
  J_GDB_GLOBAL_ACTION_REGISTER,
  J_GDB_GLOBAL_ACTION_UNREGISTER
} JGdbGlobalAction;

struct _JGdbGlobalModule
{
  JGdbGlobalModule* next;
  JGdbGlobalModule* prev;
  gpointer symfile_addr;
  guint64 symfile_size;
};

/* GDB JIT descriptor. */
struct _JGdbGlobalDescriptor
{
  guint32 version;
  guint32 action_flag /*<JGdbGlobalAction>*/;
  JGdbGlobalModule* relevant_entry;
  JGdbGlobalModule* first_entry;
};

struct _JGdb
{
  bfd* abfd;
  GPtrArray* symbols;
  JGdbGlobalModule link;
};

JGdbGlobalDescriptor __jit_debug_descriptor = { PACKAGE_VERSION_MAJOR, 0, NULL, NULL, };
void G_GNUC_NO_INLINE __jit_debug_register_code (void) { __asm__ __volatile__ (""); }
#define bfd_lasterr() (bfd_errmsg (bfd_get_error ()))

static bfd* _bfd_create (const gchar* name)
{
  static gsize core = 0;
  static bfd templ = {0};
  bfd* abfd = NULL;

  if (g_once_init_enter (&core))
    {
      bfd_init ();
      bfd_find_target ("default", &templ);
      g_once_init_leave (&core, 1);
    }

  if ((abfd = bfd_create (name, &templ)) == NULL)
    g_error ("(" G_STRLOC "): bfd_create ()!: %s", bfd_lasterr ());
  if (!bfd_make_writable (abfd))
    g_error ("(" G_STRLOC "): bfd_make_writable ()!: %s", bfd_lasterr ());

  flagword file_flags = HAS_LINENO | HAS_DEBUG | HAS_SYMS;
  flagword applicable_flags = file_flags & bfd_applicable_file_flags (abfd);

  if (!bfd_set_arch_mach (abfd, j_gdb_default_arch, j_gdb_default_mach))
    g_error ("(" G_STRLOC "): bfd_set_arch_mach ()!: %s", bfd_lasterr ());
  if (!bfd_set_file_flags (abfd, applicable_flags))
    g_error ("(" G_STRLOC "): bfd_set_file_flags ()!: %s", bfd_lasterr ());
  if (!bfd_set_format (abfd, bfd_object))
    g_error ("(" G_STRLOC "): bfd_set_format ()!: %s", bfd_lasterr ());
return (abfd);
}

JGdb* j_gdb_new ()
{
  JGdb* self;

  self = g_slice_new0 (JGdb);
  self->abfd = _bfd_create ("(stdin)");
  self->symbols = g_ptr_array_new ();
return (self);
}

void j_gdb_free (JGdb* gdb)
{
  JGdb* self = (gdb);

  bfd_close (self->abfd);
  g_ptr_array_unref (self->symbols);
  g_slice_free (JGdb, gdb);
}

JGdbSection* j_gdb_decl_section (JGdb* gdb, const gchar* name, gpointer address, gsize size)
{
  JGdb* self = (gdb);
  bfd* abfd = self->abfd;
  asection* sec;

  if ((sec = bfd_make_section (abfd, name)) == NULL)
    g_error ("(" G_STRLOC "): bfd_make_section ()!: %s", bfd_lasterr ());
  if (!bfd_set_section_size (sec, (bfd_size_type) size))
    g_error ("(" G_STRLOC "): bfd_set_section_size ()!: %s", bfd_lasterr ());
  if (!bfd_set_section_vma (sec, (bfd_vma) address))
    g_error ("(" G_STRLOC "): bfd_set_section_vma ()!: %s", bfd_lasterr ());
  if (!bfd_set_section_alignment (sec, GLIB_SIZEOF_VOID_P))
    g_error ("(" G_STRLOC "): bfd_set_section_alignment ()!: %s", bfd_lasterr ());
return (JGdbSection*) sec;
}

JGdbSymbol* j_gdb_decl_function (JGdb* gdb, const gchar* name, gpointer address, JGdbSection* section)
{
  JGdb* self = (gdb);
  bfd* abfd = self->abfd;
  gchar* symname = NULL;
  gsize length = strlen (name);
  asymbol* sym;

  if ((sym = bfd_make_empty_symbol (abfd)) == NULL)
    g_error ("(" G_STRLOC "): bfd_make_empty_symbol ()!: %s", bfd_lasterr ());

  g_snprintf (symname = bfd_alloc (abfd, length + 2), length + 2, "%c%.*s",
                bfd_get_symbol_leading_char (abfd), (int) length, name);

  sym->flags = BSF_DEBUGGING | BSF_FUNCTION | BSF_GLOBAL;
  sym->name = symname;
  sym->section = (asection*) section;
  sym->value = (symvalue) address;
return (g_ptr_array_add (self->symbols, sym), (JGdbSymbol*) sym);
}

void j_gdb_finish (JGdb* gdb)
{
  JGdb* self = (gdb);
  bfd* abfd = self->abfd;

  if (!bfd_set_symtab (abfd, (asymbol**) self->symbols->pdata, (unsigned) self->symbols->len))
    g_error ("(" G_STRLOC "): bfd_set_symtab ()!: %s", bfd_lasterr ());
}

void j_gdb_register (JGdb* gdb)
{
  JGdb* self = (gdb);
  JGdbGlobalModule* link = & self->link;
  bfd* abfd = self->abfd;

  link->symfile_addr = (gpointer) abfd->origin;
  link->symfile_size = (gsize) bfd_get_size (abfd);

  G_LOCK (global_lock);
    {
      if ((link->next = __jit_debug_descriptor.first_entry) != NULL)
        link->next->prev = link;
      __jit_debug_descriptor.action_flag = J_GDB_GLOBAL_ACTION_REGISTER;
      __jit_debug_descriptor.first_entry = link;
      __jit_debug_descriptor.relevant_entry = link;
      __jit_debug_register_code ();
    }
  G_UNLOCK (global_lock);
}

void j_gdb_unregister (JGdb* gdb)
{
  JGdbGlobalModule* link = & gdb->link;

  G_LOCK (global_lock);
    {
      if (link->prev != NULL)
        link->prev->next = link->next;
      else
        __jit_debug_descriptor.first_entry = link->next;
      if (link->next != NULL)
        link->next->prev = link->prev;

      __jit_debug_descriptor.action_flag = J_GDB_GLOBAL_ACTION_UNREGISTER;
      __jit_debug_descriptor.relevant_entry = link;
      __jit_debug_register_code ();
    }
  G_UNLOCK (global_lock);
}
