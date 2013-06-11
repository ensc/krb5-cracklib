CC = gcc
TAR = tar
XZ = xz
PKG_CONFIG = pkg-config

INSTALL = install -p
INSTALL_DATA = $(INSTALL) -m 0644
INSTALL_PROG = $(INSTALL) -m 0755

_buildflags = $(foreach k,CPP $1 LD, $(AM_$kFLAGS) $($kFLAGS) $($kFLAGS_$@))

_pkg_get_cflags =	$(shell $(PKG_CONFIG) --cflags $1)
_pkg_get_libs =		$(shell $(PKG_CONFIG) --libs $1)

prefix = /usr/local
bindir = $(prefix)/bin
sbindir = $(prefix)/sbin
libdir = $(prefix)/lib
libexecdir = $(prefix)/libexec

_target_types = PROGRAMS MODULES SCRIPTS

define _installrule
_targets :=	$(foreach t,${_target_types},$($1_$t) )
_dstdir :=	$$(DESTDIR)$$(${1}dir)/
_abstargets :=	$$(addprefix $${_dstdir},$$(_targets))

install:	install-$1
install-$1:	$$(_abstargets)

$${_dstdir}:
	mkdir -p $$@

$$(addprefix $${_dstdir},$${$1_PROGRAMS}):	_install_method=$${INSTALL_PROG}
$$(addprefix $${_dstdir},$${$1_MODULES}):	_install_method=$${INSTALL_PROG}

$$(_abstargets):$$(DESTDIR)$$(${1}dir)/%:	% | $${_dstdir}
	$${_install_method} $$< $$@

endef
