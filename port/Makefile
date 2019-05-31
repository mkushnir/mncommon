# $FreeBSD$
PORTNAME=	mrkcommon
PORTVERSION=	0.1
CATEGORIES=	devel
MASTER_SITES=	file:///data0/mkushnir/distfiles

PKGORIGIN=	${PKGCATEGORY}/${PORTNAME}

MAINTAINER=	markiyan.kushnir@gmail.com
COMMENT=	Markiyan\'s library of common C functions
LICENSE=	BSD2CLAUSE

DISTDIR=/data0/mkushnir/distfiles

GNU_CONFIGURE=	yes
CONFIGURE_ENV=	LDFLAGS="-L${LOCALBASE}/lib -L${HOME}/lib" CFLAGS=-I${HOME}/include


#OPTIONS_DEFINE=	DEBUG
#DEBUG_DESC=	Build debugging version
#
#.include <bsd.port.options.mk>
#
#.if ${PORT_OPTIONS:MDEBUG}
#CONFIGURE_ARGS+= --enable-debug
#.endif

CONFIGURE_ARGS+= --enable-debug

PLIST_FILES = include/mrkcommon/array.h \
	      include/mrkcommon/assert.h \
	      include/mrkcommon/base64.h \
	      include/mrkcommon/btrie.h \
	      include/mrkcommon/bytes.h \
	      include/mrkcommon/bytestream.h \
	      include/mrkcommon/bytestream_aux.h \
	      include/mrkcommon/cm.h \
	      include/mrkcommon/dtqueue.h \
	      include/mrkcommon/dumpm.h \
	      include/mrkcommon/endian.h \
	      include/mrkcommon/fasthash.h \
	      include/mrkcommon/float.h \
	      include/mrkcommon/hash.h \
	      include/mrkcommon/heap.h \
	      include/mrkcommon/jparse.h \
	      include/mrkcommon/json.h \
	      include/mrkcommon/malloc.h \
	      include/mrkcommon/memdebug.h \
	      include/mrkcommon/mpool.h \
	      include/mrkcommon/pbtrie.h \
	      include/mrkcommon/profile.h \
	      include/mrkcommon/rbt.h \
	      include/mrkcommon/stqueue.h \
	      include/mrkcommon/traversedir.h \
	      include/mrkcommon/unittest.h \
	      include/mrkcommon/util.h \
	      include/mrkcommon/vbytestream.h \
	      lib/libmrkcommon.so.0.0.0 \
	      lib/libmrkcommon.so.0 \
	      lib/libmrkcommon.so \
	      lib/libmrkcommon.la \
	      lib/libmrkcommon.a \

PLIST_DIRS = lib include/mrkcommon include







.include <bsd.port.mk>
