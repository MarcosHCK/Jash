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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct _JIOChannel
{
  GIOChannel parent;
  gint fd;
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

GIOChannel* j_fd_channel_new (gint fd)
{
  JIOChannel* self = NULL;
  struct stat stat;

  self = g_slice_new0 (JIOChannel);
  self->fd = fd;

  g_io_channel_init (&self->parent);
  self->parent.funcs = (gpointer) &c_funcs;

  if (fstat (fd, &stat) != 0)
    self->parent.is_seekable = FALSE;
  else
    {
      gint reg = (S_ISREG (stat.st_mode)) ? 1 : 0;
      gint chr = (S_ISCHR (stat.st_mode)) ? 1 : 0;
      gint blk = (S_ISBLK (stat.st_mode)) ? 1 : 0;
      self->parent.is_seekable = reg | chr | blk;
    }

  g_io_channel_get_flags ((gpointer) self);
return (GIOChannel*) self;
}

gint j_fd_channel_get_fd (GIOChannel* channel)
{
  g_return_val_if_fail (channel != NULL, -1);
  JIOChannel* self = (gpointer) channel;
return (self->fd);
}

static GIOStatus channel_read (GIOChannel* channel, gchar* buf, gsize bufsz, gsize* bytes_read, GError** error)
{
  JIOChannel* self = (gpointer) channel;
  gsize _bytes_read;
  gssize got;

  if (bufsz > G_MAXSSIZE)
    bufsz = G_MAXSSIZE;
  if (bytes_read == NULL)
    bytes_read = &_bytes_read;

  do
  {
    if ((got = read (self->fd, buf, bufsz)) >= 0)
      *bytes_read = got;
    else
      {
        gint e;

        switch ((e = errno))
        {
#ifdef EINTR
          case EINTR: break;
#endif // EINTR
#ifdef EAGAIN
          case EAGAIN: return G_IO_STATUS_AGAIN;
#endif // EAGAIN
          default:
            {
              g_set_error_literal
              (error,
              G_IO_CHANNEL_ERROR,
              g_file_error_from_errno (e),
              g_strerror (e));
              return G_IO_STATUS_ERROR;
            }
        }
      }
  } while (got < 0);
return (got == 0) ? G_IO_STATUS_EOF : G_IO_STATUS_NORMAL;
}

#define UNDEFINED { g_assert_not_reached (); }
static GIOStatus channel_write (GIOChannel* channel, const gchar* buf, gsize bufsz, gsize* bytes_written, GError** error) UNDEFINED
static GIOStatus channel_seek (GIOChannel* channel, gint64 offset, GSeekType type, GError** error) UNDEFINED
static GIOStatus channel_close (GIOChannel* channel, GError** error) UNDEFINED
static void channel_free (GIOChannel* channel) UNDEFINED
static GIOStatus channel_set_flags (GIOChannel* channel, GIOFlags flags, GError** error) UNDEFINED
static GIOFlags channel_get_flags (GIOChannel* channel) UNDEFINED
