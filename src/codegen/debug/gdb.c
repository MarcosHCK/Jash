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
#include <gmodule.h>

typedef struct _JGdbGlobalModule JGdbGlobalModule;
typedef struct _JGdbGlobalDescriptor JGdbGlobalDescriptor;
G_MODULE_EXPORT JGdbGlobalDescriptor __jit_debug_descriptor;
G_MODULE_EXPORT void __jit_debug_register_code ();
G_GNUC_INTERNAL JGdb* _j_gdb_new (bfd* abfd);
static G_LOCK_DEFINE (global_lock);

typedef struct
{
  bfd_size_type size;
  bfd_byte* buffer;

  /* This definition is taken from      */
  /* libbfd.h which is a bfd's privite  */
  /* header, luckly for us bfd hasn't   */
  /* changed in a while, but ...        */
} bfd_in_memory;

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
  JGdbGlobalModule link;
};

JGdbGlobalDescriptor __jit_debug_descriptor = { PACKAGE_VERSION_MAJOR, 0, NULL, NULL, };
void G_GNUC_NO_INLINE __jit_debug_register_code (void) { __asm__ __volatile__ (""); }

JGdb* _j_gdb_new (bfd* abfd)
{
  JGdb* self;

  self = g_slice_new0 (JGdb);
  self->link.symfile_addr = G_STRUCT_MEMBER (bfd_byte*, abfd->iostream, G_STRUCT_OFFSET (bfd_in_memory, buffer));
  self->link.symfile_size = G_STRUCT_MEMBER (bfd_size_type, abfd->iostream, G_STRUCT_OFFSET (bfd_in_memory, size));
return (self->abfd = abfd, self);
}

void j_gdb_free (JGdb* self)
{
  bfd_close (self->abfd);
  g_slice_free (JGdb, self);
}

void j_gdb_register (JGdb* gdb)
{
  JGdb* self = (gdb);
  JGdbGlobalModule* link = & self->link;

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
