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

void j_extern_builtin_again (const gchar* const* arguments, guint n_arguments, GError** error)
{
  exit (23);
}

void j_extern_builtin_help (const gchar* const* arguments, guint n_arguments, GError** error)
{
  exit (23);
}

void j_extern_builtin_history (const gchar* const* arguments, guint n_arguments, GError** error)
{
  exit (23);
}

void j_extern_builtin_jobs (const gchar* const* arguments, guint n_arguments, GError** error)
{
  exit (23);
}

void j_set_closure_error_exit (GError** error, int value)
{
  GValue* error_value;
  GError* error_pointer;

#if DEVELOPER == 1
  error_pointer = g_error_new (J_CLOSURE_ERROR, J_CLOSURE_ERROR_EXIT, "exit %i", value);
#else // !DEVELOPER
  error_pointer = g_error_new_literal (J_CLOSURE_ERROR, J_CLOSURE_ERROR_EXIT, "exit");
#endif // DEVELOPER
  error_value = j_closure_error_value (error_pointer);

  g_value_set_int (g_value_init (error_value, G_TYPE_INT), value);
  g_propagate_error (error, error_pointer);
}
