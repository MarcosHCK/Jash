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
#ifndef __JASH_INTANCE__
#define __JASH_INTANCE__ 1
#include <glib.h>

typedef int (*JInstance) (int argc, char* argv []);
typedef struct _JInstanceIndex JInstanceIndex;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JInstanceIndex
  {
    int name;
    JInstance callback;
  };

  G_GNUC_INTERNAL const JInstanceIndex* j_instance_index_lookup (const char *str, size_t len);
  G_GNUC_INTERNAL int j_instance_help (int argc, char* argv []);
  G_GNUC_INTERNAL int j_instance_history (int argc, char* argv []);
  G_GNUC_INTERNAL int j_instance_shell (int argc, char* argv []);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_INTANCE__
