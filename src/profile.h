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
#ifndef __JASH_PROFILE__
#define __JASH_PROFILE__ 1
#include <code.h>
#include <jobqueue.h>

typedef enum _JCodeMark JCodeMark;
typedef gint (*JParserProfiler) (GPtrArray* codes, GError** error);

#define J_PARSER_PROFILER_FINISH (0)
#define J_PARSER_PROFILER_CONTINUE (1)

#if __cplusplus
extern "C" {
#endif // __cplusplus

  enum _JCodeMark
  {
    J_CODE_MARK_FIRST_MARK = J_CODE_META_MAX_META,
  };

  G_GNUC_INTERNAL gint j_parser_profile_null_branches (GPtrArray* codes, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_PROFILE__
