#
# spec file for package minsky
#
# Copyright (c) 2013 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


# See also http://en.opensuse.org/openSUSE:Shared_library_packaging_policy

Name:           minsky-beta
%define semver 2.21.0
%define betaver beta.38
Version: 1.1.0~beta.109
Source0:        Minsky-%{semver}-%{betaver}.tar.gz
Release:        0
Summary:        Economics dynamical systems simulator
License:        GPL-3.0+
Group:          Productivity/Scientific/Math
Url:            http://minsky.sourceforge.net
BuildRequires:  gcc-c++ tcl-devel tk-devel boost-devel json_spirit-devel librsvg2-devel gsl-devel cairo-devel pango-devel readline-devel

# required to exclude libXScrnSaver-devel on ppc64
%ifarch i586 || x86_64 || aarch64
BuildRequires: libXScrnSaver-devel
%endif


%if "%{_vendor}"=="suse"
BuildRequires: libboost_system-devel libboost_regex-devel libboost_date_time-devel libboost_program_options-devel libboost_filesystem-devel libboost_thread-devel  libopenssl-devel 
%ifarch ppc64 || ppc64le || armv7l 
BuildRequires: libXss-devel
%endif
%endif

%if "%{_vendor}"=="redhat"
BuildRequires: openssl-devel
%endif

%global debug_package %{nil}

%description
The %{name} package contains libraries and header files for
developing applications that use %{name}.

%prep
%setup -n Minsky-%{semver}-%{betaver}

%build
# work around a bodgy libXss devel package on ARM
if [ -f /usr/lib/libXss.so.1 ]; then
  mkdir -p ~/usr/lib
  ln -sf /usr/lib/libXss.so.1 ~/usr/lib/libXss.so
fi
make %{?_smp_mflags} MAKEOVERRIDES=GDBM=

%install
make PREFIX=%{buildroot}/usr install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr (-,root,root)
%{_bindir}/minsky
%dir /usr/lib/minsky
/usr/lib/minsky/*


%changelog
