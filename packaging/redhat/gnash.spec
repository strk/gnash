Name:           gnash
Version:        0.7
Release:        1%{?dist}
Summary:        Flash movie player

Group:          Applications/Multimedia
License:        GPL
URL:            http://www.gnu.org/software/gnash/
Source0:        ftp://ftp.gnu.org/pub/gnu/gnash/gnash-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  libxml2-devel libpng-devel libjpeg-devel libogg-devel
BuildRequires:  xorg-x11-devel
BuildRequires:  SDL-devel SDL_mixer-devel
BuildRequires:  webclient
BuildRequires:  scrollkeeper

Requires(post): scrollkeeper
Requires(postun): scrollkeeper
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
Gnash is originally based on the code of GameSWF, by Thatcher
Ulrich. GameSWF is the most advanced of the free flash movie player
projects, and implements a fairly broad set of Flash Format v7
compliance. GameSWF was unsupported public domain software though,
and not really setup to be an industrial strength project that could
be used by everyone that uses Firefox.

%package devel
Summary:   Header files for gnash libraries
Group:     Development/Libraries
Requires:  %{name} = %{version}-%{release}
Requires:  libxml2-devel libpng-devel libjpeg-devel libogg-devel
Requires:  xorg-x11-devel
Requires:  SDL-devel SDL_mixer-devel

%description devel
This package contains all the files needed to develop applications that
will use the gnash libraries.

%package plugin
Summary:   Web-client flash movie player plugin 
Requires:  %{name} = %{version}-%{release}
Requires:  webclient
Group:     Applications/Multimedia

%description plugin
The gnash flash movie player plugin for firefox or mozilla.

%prep
%setup -q


%build
%configure --disable-static --with-plugin-dir=%{_libdir}/mozilla/plugins
#make %{?_smp_mflags}
make


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm -f $RPM_BUILD_ROOT/%{_libdir}/*.la

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
%doc doc/C/gnash.html doc/C/images
%{_bindir}/gnash
%{_bindir}/gparser
%{_bindir}/gprocessor
%{_libdir}/libgnash*.so.*
%{_libdir}/libmozsdk.so.*
%{_mandir}/man1/gnash*
%{_datadir}/gnash/
%{_datadir}/omf/gnash/

%files devel
%defattr(-,root,root,-)
%{_libdir}/libgnash*.so
%{_libdir}/libmozsdk.so

%files plugin
%defattr(-,root,root,-)
%{_libdir}/mozilla/plugins/libgnashplugin.so

%changelog
* Fri Feb  3 2006 Patrice Dumas <dumas@centre-cired.fr> - 0.7-1
- initial packaging
