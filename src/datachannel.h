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
#ifndef __JASH_DATA_CHANNEL__
#define __JASH_DATA_CHANNEL__ 1
#include <glib.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL GIOChannel* j_data_channel_new_bytes (GBytes* bytes);
  G_GNUC_INTERNAL GIOChannel* j_data_channel_new_take (gpointer data, gsize size);
  G_GNUC_INTERNAL gpointer j_data_channel_get_data (GIOChannel* channel, gsize size);
  G_GNUC_INTERNAL gsize j_data_channel_get_size (GIOChannel* channel);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_DATA_CHANNEL__
