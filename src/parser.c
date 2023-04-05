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
#include <parser.h>

struct _JParser
{
  guint ref_count;

  GPtrArray* codes;
};

G_DEFINE_QUARK (j-parser-error-quark, j_parser_error);

static void consume (JParser* self, JToken* tokens, guint n_tokens, GError** error);
static void consume2 (JParser* self, GQueue* tokens, GError** error);
#define _j_parser_unref0(var) ((var == NULL) ? NULL : (var = (j_parser_unref (var), NULL)))

JParser* j_parser_new ()
{
  JParser* self;

  self = g_slice_new (JParser);
  self->ref_count = 1;
  self->codes = g_ptr_array_new_with_free_func ((GDestroyNotify) j_code_unref);
return (self);
}

JParser* j_parser_new_from_tokens (JToken* tokens, guint n_tokens, GError** error)
{
  JParser* self = j_parser_new ();
  GError* tmperr = NULL;

  if (G_UNLIKELY ((consume (self, tokens, n_tokens, &tmperr), tmperr != NULL)))
    {
      g_propagate_error (error, tmperr);
      _j_parser_unref0 (self);
    }
return self;
}

JParser* j_parser_ref (JParser* parser)
{
  g_return_val_if_fail (parser != NULL, NULL);
  JParser* self = (parser);
return (g_atomic_int_inc (&self->ref_count), self);
}

void j_parser_unref (JParser* parser)
{
  g_return_if_fail (parser != NULL);
  JParser* self = (parser);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      g_ptr_array_unref (self->codes);
      g_slice_free (JParser, self);
    }
}

JCode** j_parser_get_codes (JParser* parser, guint* n_codes)
{
  g_return_val_if_fail (parser != NULL, NULL);
  JParser* self = (parser);
  guint nothing;

  *((n_codes) ? n_codes : &nothing) = self->codes->len;
}

static void consume (JParser* self, JToken* tokens, guint n_tokens, GError** error)
{
  GQueue tokens_ = G_QUEUE_INIT;
  guint i;

  for (i = 0; i < n_tokens; ++i)
    {
      if (tokens [i].type != J_TOKEN_TYPE_COMMENT)
        g_queue_push_head (&tokens_, &tokens [i]);
    }

  consume2 (self, &tokens_, error);
  g_queue_clear (&tokens_);
}

static void consume2 (JParser* self, GQueue* tokens, GError** error)
{
}
