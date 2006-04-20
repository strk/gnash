Name:           gnash
Version:        0.7
Release:        0%{?dist}
Summary:        GNU flash movie player

Group:          Applications/Internet
License:        GPL
URL:            http://www.gnu.org/software/gnash/
Source0:        http://www.gnu.org/software/gnash/releases/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  libxml2-devel libpng-devel libjpeg-devel libogg-devel
# the opengl devel packages are required by gtkglext-devel
# monolithic Xorg
#BuildRequires:  xorg-x11-devel
# modular Xorg 
#BuildRequires:  libGLU-devel libGL-devel
# SDL-devel is required by SDL_mixer-devel
#BuildRequires:  SDL-devel 
BuildRequires:  SDL_mixer-devel
BuildRequires:  kdelibs-devel
BuildRequires:  gtkglext-devel
# GStreamer isn't used yet, but will be in the near future.
BuildRequires:  gstreamer-devel
#BuildRequires:  gstreamer-devel >= 0.10
#BuildRequires:  webclient
BuildRequires:  scrollkeeper

Requires(post): scrollkeeper
Requires(postun): scrollkeeper
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Gnash is a GNU Flash movie player based on GameSWF,
and supports many SWF v7 features.

%package devel
Summary:   Header files for gnash libraries
Group:     Development/Libraries
Requires:  %{name} = %{version}-%{release}

%description devel
# actually there are no headers, as Gnash isn't really an API
# for other applications, so this package is rather pointless
This package contains all the files needed to develop applications that
will use the gnash libraries.

%package plugin
Summary:   Web-client flash movie player plugin 
Requires:  %{name} = %{version}-%{release}
Requires:  webclient
Group:     Applications/Multimedia

%description plugin
The gnash flash movie player plugin for firefox or mozilla.

%package -n klash
Summary:   Konqueror flash movie player plugin
Requires:  %{name} = %{version}-%{release}
Group:     Applications/Multimedia

%description -n klash
The gnash flash movie player plugin for Konqueror.

%prep
%setup -q -n %{name}-%{version}
#%patch1 -p1 -b .1
#%%patch2 -p1 -b .2
#./autogen.sh

%build
%configure --disable-static --with-plugindir=%{_libdir}/mozilla/plugins \
  --enable-ghelp --enable-docbook --enable-klash --disable-dependency-tracking
#make %{?_smp_mflags}
make


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm -f $RPM_BUILD_ROOT/%{_libdir}/*.la
rm -rf $RPM_BUILD_ROOT/%{_localstatedir}/scrollkeeper

%clean
rm -rf $RPM_BUILD_ROOT


%post 
/sbin/ldconfig
scrollkeeper-update -q -o %{_datadir}/omf/%{name} || :

%postun
/sbin/ldconfig
scrollkeeper-update -q || :


%files
%defattr(-,root,root,-)
%doc README AUTHORS COPYING NEWS 
%doc doc/C/gnash.html 
%doc doc/C/images
%{_bindir}/gnash
%{_bindir}/gparser
%{_bindir}/gprocessor
%{_libdir}/libgnash*.so.*
%{_mandir}/man1/gnash*
%{_datadir}/gnash/
%{_datadir}/omf/gnash/

%files devel
%defattr(-,root,root,-)
%{_libdir}/libgnash*.so

%files plugin
%defattr(-,root,root,-)
%{_libdir}/mozilla/plugins/libgnashplugin.so

%files -n klash
%{_libdir}/kde3/libklashpart.*
%{_datadir}/apps/klash/
%{_datadir}/config/klashrc
%{_datadir}/services/klash_part.desktop

%changelog
* Fri Feb  3 2006 Patrice Dumas <dumas@centre-cired.fr> - 0.7-1
- initial packaging
