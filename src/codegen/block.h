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
#ifndef __JASH_CODEGEN_BLOCK__
#define __JASH_CODEGEN_BLOCK__ 1
#include <glib.h>

typedef struct _JBlock JBlock;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JBlock
  {
    gpointer ptr;
    gsize sz;
  };

  #define J_BLOCK_INIT { NULL, 0, }
  #define j_block_ptr(block) (({ JBlock* __block = ((block)); __block->ptr; }))
  #define j_block_sz(block) (({ JBlock* __block = ((block)); __block->sz; }))

  G_GNUC_INTERNAL void j_block_clear (JBlock* block);
  G_GNUC_INTERNAL void j_block_init (JBlock* block, gsize sz);
  G_GNUC_INTERNAL void j_block_protect (JBlock* block);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_BLOCK__
