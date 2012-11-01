Name: siilihai-client
Version: 2.0.0
Release: 1
Summary: Siilihai web forum reader
License: GPL
Group: Productivity/Networking/Web/Utilities
Url: http://siilihai.com/
Source: %{name}-%{version}.tar.gz
Requires: libsiilihai2
Requires: qtwebkit
BuildRequires: qt-devel
BuildRequires: libsiilihai2-devel
BuildRequires: pkgconfig(QtCore) >= 4.6.0
BuildRequires: pkgconfig(QtWebKit) >= 4.6.0

%description
Siilihai web forum reader

%prep
%setup -q

%build
%qmake
make %{?jobs:-j%jobs}

%install
%{__rm} -rf %{buildroot}
%qmake_install

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
