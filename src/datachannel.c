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
#include <fdchannel.h>

struct _JIOChannel
{
  GIOChannel parent;
  GBytes* bytes;
  gsize offset;
  guint closed : 1;
};

typedef struct _JIOChannel JIOChannel;
static GIOStatus channel_read (GIOChannel* channel, gchar* buf, gsize bufsz, gsize* bytes_read, GError** error);
static GIOStatus channel_write (GIOChannel* channel, const gchar* buf, gsize bufsz, gsize* bytes_written, GError** error);
static GIOStatus channel_seek (GIOChannel* channel, gint64 offset, GSeekType type, GError** error);
static GIOStatus channel_close (GIOChannel* channel, GError** error);
static void channel_free (GIOChannel* channel);
static GIOStatus channel_set_flags (GIOChannel* channel, GIOFlags flags, GError** error);
static GIOFlags channel_get_flags (GIOChannel* channel);

static const GIOFuncs c_funcs =
{
  channel_read,
  channel_write,
  channel_seek,
  channel_close,
  NULL,
  channel_free,
  channel_set_flags,
  channel_get_flags,
};

GIOChannel* j_data_channel_new_bytes (GBytes* bytes)
{
  g_return_val_if_fail (bytes != NULL, NULL);
  JIOChannel* self = NULL;

  self = g_slice_new0 (JIOChannel);
  self->bytes = g_bytes_ref (bytes);
  self->offset = 0;
  self->closed = 0;

  g_io_channel_init (&self->parent);
  self->parent.funcs = (gpointer) &c_funcs;
return (GIOChannel*) self;
}

GIOChannel* j_data_channel_new_data (gpointer data, gsize size)
{
  g_return_val_if_fail (data != NULL, NULL);
  GIOChannel* self = NULL;
  GBytes* bytes = NULL;

  bytes = g_bytes_new_take (data, size);
  self = j_data_channel_new_bytes (bytes);
return (g_bytes_unref (bytes), self);
}

#define UNDEFINED { g_assert_not_reached (); }
static GIOStatus channel_read (GIOChannel* channel, gchar* buf, gsize bufsz, gsize* bytes_read, GError** error) UNDEFINED
static GIOStatus channel_write (GIOChannel* channel, const gchar* buf, gsize bufsz, gsize* bytes_written, GError** error) UNDEFINED
static GIOStatus channel_seek (GIOChannel* channel, gint64 offset, GSeekType type, GError** error) UNDEFINED
static GIOStatus channel_close (GIOChannel* channel, GError** error) UNDEFINED
static void channel_free (GIOChannel* channel) UNDEFINED
static GIOStatus channel_set_flags (GIOChannel* channel, GIOFlags flags, GError** error) UNDEFINED
static GIOFlags channel_get_flags (GIOChannel* channel) UNDEFINED
