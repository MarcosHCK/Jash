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
#ifndef __JASH__
#define __JASH__ 1
#include <glib-object.h>

#define J_TYPE_ASH (j_ash_get_type ())
#define J_ASH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), J_TYPE_ASH, JAsh))
#define J_IS_ASH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), J_TYPE_ASH))
typedef struct _JAsh JAsh;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL GType j_ash_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL void j_ash_run_interactive (JAsh* jash, GError** error);
  G_GNUC_INTERNAL void j_ash_run_script (JAsh* jash, const gchar* filename, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH__
