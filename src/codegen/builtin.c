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