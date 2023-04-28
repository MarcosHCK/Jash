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
#include <config.h>
#include <lexer/datachannel.h>

struct _JIOChannel
{
  GIOChannel parent;
  GBytes* bytes;
  gsize offset;
  guint closed : 1;
};

typedef struct _JIOChannel JIOChannel;
static GIOStatus channel_read (GIOChannel* channel, gchar* buf, gsize bufsz, gsize* bytes_read, GError** error);
static GIOStatus channel_seek (GIOChannel* channel, gint64 offset, GSeekType type, GError** error);
static GIOStatus channel_close (GIOChannel* channel, GError** error);
static void channel_free (GIOChannel* channel);
static GIOStatus channel_set_flags (GIOChannel* channel, GIOFlags flags, GError** error);
static GIOFlags channel_get_flags (GIOChannel* channel);

static const GIOFuncs c_funcs =
{
  channel_read,
  NULL,
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
  self->parent.is_seekable = TRUE;
  self->parent.is_readable = TRUE;
  self->parent.is_writeable = FALSE;
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

static GIOStatus channel_read (GIOChannel* channel, gchar* buf, gsize bufsz, gsize* bytes_read, GError** error)
{
  JIOChannel* self = (gpointer) channel;
  gsize left = g_bytes_get_size (self->bytes) - self->offset;
  gsize chunksz = (bufsz > left) ? left : bufsz;

  if (self->closed)
    {
      g_set_error (error, G_IO_CHANNEL_ERROR, G_IO_CHANNEL_ERROR_FAILED, "Already closed");
      return G_IO_STATUS_ERROR;
    }

  if ((*bytes_read = chunksz) == 0)
    return G_IO_STATUS_EOF;
  else
    {
      memcpy (buf, g_bytes_get_region (self->bytes, 1, self->offset, chunksz), chunksz);
      return (self->offset += chunksz, G_IO_STATUS_NORMAL);
    }
}

static GIOStatus channel_seek (GIOChannel* channel, gint64 offset, GSeekType type, GError** error)
{
  JIOChannel* self = (gpointer) channel;

  if (self->closed)
    {
      g_set_error (error, G_IO_CHANNEL_ERROR, G_IO_CHANNEL_ERROR_FAILED, "Already closed");
      return G_IO_STATUS_ERROR;
    }

  switch (type)
    {
      case G_SEEK_CUR:
        {
          if (offset < 0)
            {
              if (self->offset >= -offset)
                self->offset -= (-offset);
              else
                {
                  g_set_error_literal (error, G_IO_CHANNEL_ERROR, G_IO_CHANNEL_ERROR_INVAL, "Buffer underflow");
                  return G_IO_STATUS_ERROR;
                }
            }
          else
            {
              gsize tmp;

              if ((tmp = self->offset + (gsize) offset) > self->offset)
                {
                  if (g_bytes_get_size (self->bytes) >= tmp)
                    self->offset = tmp;
                  else
                    {
                      g_set_error_literal (error, G_IO_CHANNEL_ERROR, G_IO_CHANNEL_ERROR_INVAL, "Buffer overflow");
                      return G_IO_STATUS_ERROR;
                    }
                }
              else
                {
                  /* Overflow!!! */
                  g_set_error_literal (error, G_IO_CHANNEL_ERROR, G_IO_CHANNEL_ERROR_INVAL, "Integer overflow");
                  return G_IO_STATUS_ERROR;
                }
            }
          break;
        }

      case G_SEEK_SET:
        {
          if (offset < 0)
            {
              g_set_error_literal (error, G_IO_CHANNEL_ERROR, G_IO_CHANNEL_ERROR_INVAL, "Invalid offset");
              return G_IO_STATUS_ERROR;
            }
          else
            {
              if (g_bytes_get_size (self->bytes) >= offset)
                self->offset = offset;
              else
                {
                  g_set_error_literal (error, G_IO_CHANNEL_ERROR, G_IO_CHANNEL_ERROR_INVAL, "Invalid offset");
                  return G_IO_STATUS_ERROR;
                }
            }
          break;
        }

      case G_SEEK_END:
        {
          self->offset = g_bytes_get_size (self->bytes);
          return channel_seek (channel, offset, G_SEEK_CUR, error);
        }

      default: g_assert_not_reached (); break;
    }
return G_IO_STATUS_NORMAL;
}

static GIOStatus channel_close (GIOChannel* channel, GError** error)
{
  JIOChannel* self = (gpointer) channel;
  self->closed = TRUE;
return G_IO_STATUS_NORMAL;
}

static void channel_free (GIOChannel* channel)
{
  JIOChannel* self = (gpointer) channel;

  g_bytes_unref (self->bytes);
  g_slice_free (JIOChannel, self);
}

static GIOStatus channel_set_flags (GIOChannel* channel, GIOFlags flags, GError** error) UNDEFINED
static GIOFlags channel_get_flags (GIOChannel* channel) UNDEFINED
