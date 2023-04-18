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
#ifndef __JASH_HISTCONTROL__
#define __JASH_HISTCONTROL__ 1
#include <glib.h>

typedef enum _JHistControl JHistControl;
typedef struct _JHistControlIndex JHistControlIndex;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  enum _JHistControl
  {
    J_HIST_CONTROL_NONE = 0,
    J_HIST_CONTROL_IGNORE_DUPLICATED = (1 << 0),
    J_HIST_CONTROL_IGNORE_FIRST_SPACE = (1 << 1),
    J_HIST_CONTROL_ERASE_DUPLICATED = (1 << 2),
  };

  struct _JHistControlIndex
  {
    gint name;
    glong value;
  };

  G_GNUC_INTERNAL const JHistControlIndex* j_hist_control_index_lookup (const char *str, size_t len);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_HISTCONTROL__
