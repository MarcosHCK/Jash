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
#ifndef __JASH_DOSSIER__
#define __JASH_DOSSIER__ 1
#include <glib-object.h>

typedef struct _JDossierEntry JDossierEntry;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JDossierEntry
  {
    int name;
    gdouble cost;
    const gchar* long_name;
    const gchar* resource_path;
  };

  G_GNUC_INTERNAL void j_dossier_help (const gchar* parameter);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_DOSSIER__
