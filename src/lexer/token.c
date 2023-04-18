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
#include <lexer/token.h>
#include <lexer/private.h>

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

#define _DEFINE_INTERN_FULL(type,name,value) \
  const gchar* j_token_ ## type ## _ ## name ## _intern_string (void) \
    { \
      static gsize __value__ = 0; \
      if (g_once_init_enter (&__value__)) \
        { \
          const gchar* static_string = G_STRINGIFY (value); \
          const gchar* intern_string = g_intern_static_string (static_string); \
          g_once_init_leave (&__value__, GPOINTER_TO_SIZE (intern_string)); \
        } \
      return (gchar*) __value__; \
    }

#define _DEFINE_INTERN(type,name) _DEFINE_INTERN_FULL (type,name,name)

_DEFINE_INTERN (builtin, again);
_DEFINE_INTERN (builtin, cd);
_DEFINE_INTERN (builtin, exit);
_DEFINE_INTERN (builtin, false);
_DEFINE_INTERN (builtin, fg);
_DEFINE_INTERN (builtin, get);
_DEFINE_INTERN (builtin, help);
_DEFINE_INTERN (builtin, history);
_DEFINE_INTERN (builtin, jobs);
_DEFINE_INTERN (builtin, set);
_DEFINE_INTERN (builtin, true);
_DEFINE_INTERN (builtin, unset);
_DEFINE_INTERN (keyword, else);
_DEFINE_INTERN_FULL (keyword, end, fi);
_DEFINE_INTERN (keyword, if);
_DEFINE_INTERN (keyword, then);
_DEFINE_INTERN_FULL (operator, detach, &);
_DEFINE_INTERN_FULL (operator, expansion, `);
_DEFINE_INTERN_FULL (operator, logical_and, &&);
_DEFINE_INTERN_FULL (operator, logical_or, ||);
_DEFINE_INTERN_FULL (operator, pipe, |);
_DEFINE_INTERN_FULL (operator, redirection_append, >>);
_DEFINE_INTERN_FULL (operator, redirection_read, <);
_DEFINE_INTERN_FULL (operator, redirection_write, >);
_DEFINE_INTERN_FULL (separator, chain, ;);
_DEFINE_INTERN_FULL (separator, newline, \n);

JTokens* _j_tokens_new ()
{
  JTokens* self;

  self = g_slice_new (JTokens);
  self->ref_count = 1;
  self->chunk = (gpointer) g_string_chunk_new (64);
  self->array = (gpointer) g_array_new (0, 0, sizeof (JToken));
return (self);
}

JTokens* j_tokens_ref (JTokens* tokens)
{
  g_return_val_if_fail (tokens != NULL, NULL);
  JTokens* self = (tokens);
return (g_atomic_int_inc (&self->ref_count), self);
}

void j_tokens_unref (JTokens* tokens)
{
  g_return_if_fail (tokens != NULL);
  JTokens* self = (tokens);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      g_string_chunk_free (self->chunk);
      g_array_unref (&self->array->g_array);
      g_slice_free (JTokens, self);
    }
}

guint j_tokens_get_count (JTokens* tokens)
{
  g_return_val_if_fail (tokens != NULL, 0);
  JTokens* self = (tokens);
return (self->array->count);
}

JToken* j_tokens_index (JTokens* tokens, guint index)
{
  g_return_val_if_fail (tokens != NULL, NULL);
  JTokens* self = (tokens);
return & g_array_index (&self->array->g_array, JToken, index);
}
