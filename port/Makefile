# $FreeBSD$
PORTNAME=	mrkcommon
PORTVERSION=	0.1
CATEGORIES=	devel
MASTER_SITES=	https://raw.github.com/mkushnir/distfiles/master/

MAINTAINER=	markiyan.kushnir@gmail.com
COMMENT=	Markiyan\'s library of common C functions

MAN3=		mrkcommon.3
MANCOMPRESSED=	no
USE_LDCONFIG=	yes


GNU_CONFIGURE=	yes

OPTIONS_DEFINE=	DEBUG
DEBUG_DESC=	Build debugging version

.include <bsd.port.options.mk>

.if ${PORT_OPTIONS:MDEBUG}
CONFIGURE_ARGS+= --enable-debug
.endif

.include <bsd.port.mk>
