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
	code.h \
	datachannel.h \
	fdchannel.h \
	jobqueue.h \
	lexer.h \
	machine.h \
	parser.h \
	token.h \
	$(VOID)

#
# Binaries and libraries
# - sources
#

jash_SOURCES=\
	code.c \
	datachannel.c \
	fdchannel.c \
	jash.c \
	jobqueue.c \
	lexer.c \
	machine.c \
	parser.c \
	$(VOID)
jash_CFLAGS=\
	-DG_LOG_DOMAIN=\"Jash\" \
	-DG_LOG_USE_STRUCTURED=1 \
	$(GOBJECT_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(VOID)
jash_LDFLAGS=\
	$(GOBJECT_LIBS) \
	$(GLIB_LIBS) \
	$(VOID)
