# Copyright 2023 MarcosHCK
# Copyright 2023 DavierSB
# This file is part of JASH.
#
# JASH is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# JASH is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with JASH. If not, see <http://www.gnu.org/licenses/>.
#

VOID=

#
# Binaries and libraries
# - declare
#

bin_PROGRAMS=\
	jash
	$(VOID)

noinst_HEADERS=\
	ast.h \
	datachannel.h \
	fdchannel.h \
	histcontrol.h \
	lexer.h \
	parser.h \
	readline.h \
	token.h \
	walker.h \
	$(VOID)

#
# Binaries and libraries
# - sources
#

jash_SOURCES=\
	datachannel.c \
	fdchannel.c \
	histcontrol.c \
	jash.c \
	lexer.c \
	parser.c \
	readline.c \
	token.c \
	$(VOID)
jash_CFLAGS=\
	-DG_LOG_DOMAIN=\"Jash\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-flto \
	$(GOBJECT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(LLVM_CFLAGS) \
	$(READLINE_CFLAGS) \
	$(VOID)
jash_LDFLAGS=\
	-flto \
	$(GOBJECT_LIBS) \
	$(GLIB_LIBS) \
	$(LLVM_LIBS) \
	$(READLINE_LIBS) \
	$(VOID)

#
# Extensions
# - .stringlist -> .c
#

SUFFIXES=.stringlist

.stringlist.c:
	$(GPERF) --output-file $@ -PCI $<
