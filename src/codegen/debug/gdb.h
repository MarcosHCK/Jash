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
#ifndef __JASH_CODEGEN_DEBUG_GDB__
#define __JASH_CODEGEN_DEBUG_GDB__ 1
#include <glib.h>

typedef struct _JGdb JGdb;
typedef struct _JGdbBuilder JGdbBuilder;
typedef struct _JGdbSection JGdbSection;
typedef struct _JGdbSymbol JGdbSymbol;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JGdbBuilder
  {
    gpointer _private1_;
    gpointer _private2_;
  };

  G_GNUC_INTERNAL extern const guint j_gdb_default_arch;
  G_GNUC_INTERNAL extern const guint j_gdb_default_mach;

  G_GNUC_INTERNAL void j_gdb_builder_init (JGdbBuilder* builder);
  G_GNUC_INTERNAL void j_gdb_builder_clear (JGdbBuilder* builder);
  G_GNUC_INTERNAL JGdbSymbol* j_gdb_builder_decl_function (JGdbBuilder* builder, const gchar* name, gpointer address, JGdbSection* section);
  G_GNUC_INTERNAL JGdbSymbol* j_gdb_builder_decl_object (JGdbBuilder* builder, const gchar* name, gpointer address, gsize size, JGdbSection* section);
  G_GNUC_INTERNAL JGdbSymbol* j_gdb_builder_decl_section_symbol (JGdbBuilder* builder, const gchar* name, gpointer address, JGdbSection* section);
  G_GNUC_INTERNAL JGdbSymbol* j_gdb_builder_decl_symbol (JGdbBuilder* builder, const gchar* name, gpointer address, JGdbSection* section);
  G_GNUC_INTERNAL JGdbSection* j_gdb_builder_decl_section_as_code (JGdbBuilder* builder, const gchar* name, gpointer address, gsize size);
  G_GNUC_INTERNAL JGdbSection* j_gdb_builder_decl_section_as_data (JGdbBuilder* builder, const gchar* name, gpointer address, gsize size);
  G_GNUC_INTERNAL void j_gdb_builder_fill_section (JGdbBuilder* builder, gpointer address, gsize size, JGdbSection* section);
  G_GNUC_INTERNAL JGdb* j_gdb_builder_end (JGdbBuilder* builder);

  G_GNUC_INTERNAL void j_gdb_free (JGdb* gdb);
  G_GNUC_INTERNAL void j_gdb_register (JGdb* gdb);
  G_GNUC_INTERNAL void j_gdb_unregister (JGdb* gdb);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_DEBUG_GDB__
