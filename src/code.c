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
G_STATIC_ASSERT (J_CODE_TYPE_MAX_CODE < (G_MAXUINT >> 5));

/**
 * j_code_new: (constructor)
 * @type: instruction code type.
 * @argument_size: size of reserved area for instruction argument.
 * 
 * Creates an empty instruction.
 * 
 * Returns: (transfer full): an empty #JCode instance.
 */
JCode* j_code_new (JCodeType type, gsize argument_size)
{
  g_return_val_if_fail (type < J_CODE_TYPE_MAX_CODE, NULL);
  g_return_val_if_fail (argument_size < (G_MAXUINT >> 5), NULL);
  gsize full = argument_size + sizeof (JCode);

  JCode* code = g_slice_alloc0 (full);
  gpointer arg = G_STRUCT_MEMBER_P (code, sizeof (JCode));

    code->ref_count = 1;
    code->type = type;
    code->size = argument_size;
    code->argument = arg;
return code;
}

/**
 * j_code_new: (constructor)
 * @type: instruction code type.
 * @value: instruction argument value.
 *
 * Creates a @type instruction whose argument is an string
 * which is copied from @value.
 *
 * Returns: (transfer full): a #JCode instance.
 */
JCode* j_code_new_string (JCodeType type, const gchar* value)
{
  g_return_val_if_fail (value != NULL, NULL);

  gsize length = strlen (value);
  JCode* code = j_code_new (type, length + 1);
                memcpy (code->string_argument, value, length);
                        code->string_argument [length] = 0;
  return code;
}

/**
 * j_code_ref: (method)
 * @code: a #JCode instance.
 *
 * Increments @code reference count (atomically).
 *
 * Returns: (transfer full): see description.
 */
JCode* j_code_ref (JCode* code)
{
  g_return_val_if_fail (code != NULL, NULL);
  JCode* self = (code);
return (g_atomic_int_inc (&self->ref_count), self);
}

/**
 * j_code_unref: (method)
 * @code: a #JCode instance.
 *
 * Decrements @code reference count (atomically).
 *
 */
void j_code_unref (JCode* code)
{
  g_return_if_fail (code != NULL);
  JCode* self = (code);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      g_slice_free1 (code->size + sizeof (JCode), code);
    }
}
