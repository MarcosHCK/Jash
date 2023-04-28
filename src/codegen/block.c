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
#include <config.h>
#include <codegen/block.h>
#ifdef G_OS_WIN32
# include <windows.h>
#else // !G_OS_WIN32
# include <sys/mman.h>
# if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#   define MAP_ANONYMOUS MAP_ANON
# endif // !MAP_ANONYMOUS && MAP_ANON
#endif // G_OS_WIN32

static const JBlock __null = {0};

void j_block_clear (JBlock* block)
{
#ifdef G_OS_WIN32
  VirtualFree (block->ptr, block->sz, 0);
#else // !G_OS_WIN32
  munmap (block->ptr, block->sz);
#endif // G_OS_WIN32
  *block = __null;
}

void j_block_init (JBlock* block, gsize sz)
{
#if G_OS_WIN32
  block->ptr = VirtualAlloc (0, block->sz = sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else // !G_OS_WIN32
  block->ptr = mmap (0, block->sz = sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif // G_OS_WIN32
}

void j_block_protect (JBlock* block)
{
#ifdef G_OS_WIN32
  G_STMT_START
    {
      DWORD dwOld;
      VirtualProtect (block->ptr, block->sz, PAGE_EXECUTE_READ, &dwOld);
    }
  G_STMT_END;
#else // !G_OS_WIN32
  mprotect (block->ptr, block->sz, PROT_READ | PROT_EXEC);
#endif // G_OS_WIN32
}
