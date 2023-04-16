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
#include <codegen_common.h>
#ifdef G_OS_WIN32
# include <windows.h>
#else // !G_OS_WIN32
# include <sys/mman.h>
# if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#   define MAP_ANONYMOUS MAP_ANON
# endif // !MAP_ANONYMOUS && MAP_ANON
#endif // G_OS_WIN32

static void closure_notify (gpointer notify_data, JCodegenClosure* jc)
{
#ifdef G_OS_WIN32
  VirtualFree (jc->block, jc->blocksz, 0);
#else // !G_OS_WIN32
  munmap (jc->block, jc->blocksz);
#endif // G_OS_WIN32
}

G_STATIC_ASSERT (G_STRUCT_OFFSET (JCodegenClosure, entry) == G_STRUCT_OFFSET (GCClosure, callback));

GClosure* j_codegen_emit (Dst_DECL)
{
  JCodegenClosure* jc = NULL;
  GClosure* gc = NULL;
  int result = 0;
  size_t sz;

  if ((result = dasm_link (Dst, &sz)), G_UNLIKELY (result != 0))
    g_error ("(" G_STRLOC "): dasm_link()!");

  gc = g_closure_new_simple (sizeof (*jc), NULL);
  jc = (JCodegenClosure*) gc;

  g_closure_add_finalize_notifier (gc, NULL, (GClosureNotify) closure_notify);
  g_closure_set_marshal (gc, g_cclosure_marshal_VOID__POINTER);

  g_closure_ref (gc);
  g_closure_sink (gc);

  jc->blocksz = sz;
#if G_OS_WIN32
  jc->block = VirtualAlloc (0, sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else // !G_OS_WIN32
  jc->block = mmap (0, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif // G_OS_WIN32

  if ((result = dasm_encode (Dst, jc->block)), G_UNLIKELY (result != 0))
    g_error("(" G_STRLOC "): dasm_encode()!");

#ifdef G_OS_WIN32
  G_STMT_START
    {
      DWORD dwOld;
      VirtualProtect (jc->block, sz, PAGE_EXECUTE_READ, &dwOld);
    }
  G_STMT_END;
#else // !G_OS_WIN32
  mprotect (jc->block, sz, PROT_READ | PROT_EXEC);
#endif // G_OS_WIN32
return (jc->entry = G_CALLBACK (Dst->labels [J_CODEGEN_LABEL_MAIN]), gc);
}
