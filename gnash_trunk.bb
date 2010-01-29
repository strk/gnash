DESCRIPTION = "Gnash is a GNU Flash movie player that supports many SWF v8 features"
HOMEPAGE = "http://www.gnu.org/software/gnash"
LICENSE = "GPLv3"
DEPENDS = "giflib cairo libtool gtk+ agg libsdl-mixer zlib boost jpeg pango curl freetype \
           ${@base_conditional('ENTERPRISE_DISTRO', '1', '', 'ffmpeg', d)}"
DEPENDS += "gst-plugins-base"
          
PR = "r6"

SRC_URI = "http://www.getgnash.org/packages/snapshots/gnash-trunk.tar.bz2"

inherit autotools pkgconfig

EXTRA_OECONF = "--enable-gui=gtk \
                --enable-renderer=agg \
                ${@base_conditional('ENTERPRISE_DISTRO', '1', '', '--enable-media=ffmpeg', d)} \
                --with-plugins-install=system \
		--disable-plugins \
		--disable-dependency-tracking \
		--disable-testsuite \
		--enable-media=ffmpeg \
		--with-cairo-incl=${STAGING_DIR_HOST}/usr/include/cairo \
                --with-cairo-lib=${STAGING_DIR_HOST}/usr/lib \
		--with-jpeg-incl=${STAGING_DIR_HOST}/usr/include/ \
                --with-jpeg-lib=${STAGING_DIR_HOST}/usr/lib \
                --with-top-level=${STAGING_DIR_HOST}/usr \
                "

PACKAGES =+ " gnash-browser-plugin libgnashamf libgnashbackend libgnashbase libgnashgeo libgnashgui libgnashplayer libgnashserver "

FILES_gnash-browser-plugin= "${libdir}/mozilla/plugins/*"
FILES_libgnashamf = "${libdir}/gnash/libgnashamf-${PV}.so"
FILES_libgnashbackend = "${libdir}/gnash/libgnashbackend-${PV}.so"
FILES_libgnashbase = "${libdir}/gnash/libgnashbase-${PV}.so"
FILES_libgnashgeo = "${libdir}/gnash/libgnashgeo-${PV}.so"
FILES_libgnashgui = "${libdir}/gnash/libgnashgui-${PV}.so"
FILES_libgnashplayer = "${libdir}/gnash/libgnashplayer-${PV}.so"
FILES_libgnashserver = "${libdir}/gnash/libgnashserver-${PV}.so"

PARALLEL_MAKE = "-j4"

acpaths = " -Imacros"

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_angstrom = "1"

# Boost lacks defines for lots of archs
TARGET_CC_ARCH_append = " -I${STAGING_INCDIR}/libxml2 -DHAVE_POLL_H ${@[' -D_BIG_ENDIAN', ' -D_LITTLE_ENDIAN'][bb.data.getVar('SITEINFO_ENDIANESS', d, 1) == 'le']}"


do_install_append() {
	oe_runmake DESTDIR=${D} install-plugin
}

