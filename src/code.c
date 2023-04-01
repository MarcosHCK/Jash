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
#include <code.h>

G_STATIC_ASSERT (sizeof (guintptr) * 2 >= sizeof (JCode));
G_STATIC_ASSERT (MAX_CODE < (G_MAXUINT << (sizeof (guint) * 8 - 5)));

JCode* j_code_new (JCodeType type, gsize argument_size)
{
  g_return_val_if_fail (type < MAX_CODE, NULL);
  g_return_val_if_fail (argument_size < (G_MAXUINT << 5), NULL);
  gsize full = argument_size + sizeof (JCode);

  JCode* code = g_slice_alloc0 (full);
  gpointer arg = G_STRUCT_MEMBER_P (code, sizeof (JCode));

    code->type = type;
    code->size = argument_size;
    code->argument = arg;
return code;
}

JCode* j_code_new_string (JCodeType type, const gchar* value)
{
  g_return_val_if_fail (value != NULL, NULL);

  gsize length = strlen (value);
  JCode* code = j_code_new (type, length + 1);
                memcpy (code->string_argument, value, length);
                        code->string_argument [length] = 0;
  return code;
}

void j_code_free (JCode* code)
{
  g_return_if_fail (code != NULL);
  g_slice_free1 (code->size + sizeof (JCode), code);
}
