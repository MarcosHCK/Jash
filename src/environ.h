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
#ifndef __JASH_ENVIRON__
#define __JASH_ENVIRON__
#include <glib-object.h>

#define J_TYPE_ENVIRON (j_environ_get_type ())
#define J_ENVIRON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), J_TYPE_ENVIRON, JEnviron))
#define J_IS_ENVIRON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), J_TYPE_ENVIRON))
typedef struct _JEnviron JEnviron;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL GType j_environ_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL JEnviron* j_environ_new (void);
  G_GNUC_INTERNAL GBytes* j_environ_get (JEnviron* environ, GBytes* key);
  G_GNUC_INTERNAL gboolean j_environ_get_interactive (JEnviron* environ);
  G_GNUC_INTERNAL void j_environ_set (JEnviron* environ, GBytes* key, GBytes* value);
  G_GNUC_INTERNAL void j_environ_set_interactive (JEnviron* environ, gboolean value);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_ENVIRON__
