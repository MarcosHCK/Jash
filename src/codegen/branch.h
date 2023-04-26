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
#ifndef __JASH_CODEGEN_BRANCH__
#define __JASH_CODEGEN_BRANCH__ 1
#include <glib.h>

typedef enum
{
  J_BRANCH_TYPE_CHAIN = 1,
  J_BRANCH_TYPE_IF = 2,
} JBranchType;

typedef struct _JBranch JBranch;
typedef struct _JBranch JBranchChain;
typedef struct _JBranchIf JBranchIf;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JBranch
  {
    union
    {
      JBranchType type;
      guint count;
    };

    guint directpc;
  };

  struct _JBranchIf
  {
    JBranch parent;
    guint reversepc;
  };

  G_GNUC_INTERNAL void j_branch_copy (const JBranch* src, JBranch* dst);
  G_GNUC_INTERNAL void j_branch_if_get_direct (const JBranchIf* branch_if, JBranchChain* dst);
  G_GNUC_INTERNAL void j_branch_if_get_reverse (const JBranchIf* branch_if, JBranchChain* dst);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_BRANCH__
