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
#include <codegen/branch.h>
#include <codegen/codegen.h>
#include <codegen/context.h>

void j_branch_init (Dst_DECL, JBranch* branch, guint count)
{
  guint* pcs = & branch->directpc;
    branch->count = count;
  guint i;

  for (i = 0; i < count; ++i)
  {
    pcs [i] = j_context_allocpc (Dst);
  }
}

void j_branch_copy (const JBranch* src, JBranch* dst)
{
#if HAVE_MEMCPY
  memcpy (dst, src, sizeof (JBranch) + (G_SIZEOF_MEMBER (JBranch, directpc) * (src->count - 1)));
#else // !HAVE_MEMCPY
  guint* src_pcs = & src->directpc;
  guint* dst_pcs = & dst->directpc;
  guint i;

  for (i = 0; i < src->count; ++i)
    dst_pcs [i] = src_pcs [i];
    dst->count = src->count;
#endif // HAVE_MEMCPY
}

void j_branch_if_get_direct (const JBranchIf* branch_if, JBranchChain* dst)
{
  dst->count = 1;
  dst->directpc = branch_if->parent.directpc;
}

void j_branch_if_get_reverse (const JBranchIf* branch_if, JBranchChain* dst)
{
  dst->count = 1;
  dst->directpc = branch_if->reversepc;
}
