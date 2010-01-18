DESCRIPTION = "Gnash is a GNU Flash movie player that supports many SWF v7 features"
HOMEPAGE = "http://www.gnu.org/software/gnash"
LICENSE = "GPLv3"
DEPENDS = "libtool gtk+ pango atk glib agg libsdl-mixer zlib boost \
	libpng jpeg giflib curl freetype fontconfig gst-plugins-base \
        ${@base_conditional('ENTERPRISE_DISTRO', '1', '', 'ffmpeg', d)}"
          
PR = "r0"

SRC_URI = "http://www.getgnash.org/packages/snapshots/gnash-${PV}.tar.bz2"
#SRC_URI[tarball.md5sum] = "2e9f7464bc2b9246aa0a24facf2b88b1"

acpaths = " -Imacros"
inherit autotools pkgconfig

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_angstrom = "1"

# Boost lacks defines for lots of archs
TARGET_CC_ARCH_append = "-DHAVE_POLL_H ${@[' -D_BIG_ENDIAN', ' -D_LITTLE_ENDIAN'][bb.data.getVar('SITEINFO_ENDIANESS', d, 1) == 'le']}"

do_install_append() {
	oe_runmake DESTDIR=${D} install-plugin
}

# CXXFLAGS += " -I${STAGING_INCDIR} "
# LDFLAGS  += " -L${STAGING_LIBDIR} "

EXTRA_OECONF = " --with-plugins-install=system \
	          --prefix=/usr \
		  --enable-gui=gtk \
                  --enable-media=gst \
		  --disable-dependency-tracking \
               "

PACKAGES =+ "gnash-common \
	 gnash-browser-plugin \
	 gnash-gui \
	 cygnal \
	 "

FILES_gnash-browser-plugin= "${libdir}/mozilla/plugins/*"
FILES_gnash-common = "${libdir}/gnash/libgnash*.so"
FILES_gnash-gui = "${bindir}/*gnash"
FILES_cygnal = "${bindir}/gnash ${bindir}/cygnal"

PARALLEL_MAKE = "-j4"
