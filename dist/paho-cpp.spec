Summary:            MQTT CPP Client
Name:               paho-cpp
Version:            1.5.2
Release:            0%{?dist}
License:            Eclipse Eclipse Public License 2.0 and Distribution License 1.0
Group:              Development/Tools
Source:             https://github.com/eclipse/paho.mqtt.cpp/archive/v%{version}.tar.gz
URL:                https://eclipse.org/paho/clients/cpp/
BuildRequires:      cmake3
BuildRequires:      gcc
BuildRequires:      graphviz
BuildRequires:      doxygen
BuildRequires:      openssl-devel
BuildRequires:      paho-c-devel
Requires:           openssl
Requires:           paho-c


%description
The Paho MQTT CPP Client is a fully fledged MQTT client written in ANSI standard C++ 17.


%package devel
Summary:            MQTT CPP Client development kit
Group:              Development/Libraries
Requires:           paho-cpp

%description devel
Development files and samples for the the Paho MQTT CPP Client.


%package devel-docs
Summary:            MQTT CPP Client development kit documentation
Group:              Development/Libraries

%description devel-docs
Development documentation files for the the Paho MQTT CPP Client.

%prep
%autosetup -n paho-cpp-%{version}

%build
mkdir build.paho.cpp && cd build.paho.cpp
%cmake3 -DPAHO_WITH_SSL=TRUE -DPAHO_BUILD_DOCUMENTATION=TRUE -DPAHO_BUILD_EXAMPLES=TRUE ..
make %{?_smp_mflags}

%install
cd build.paho.cpp
make install DESTDIR=%{buildroot}

%files
%doc edl-v10 epl-v20
%{_libdir}/*

%files devel
%{_bindir}/*
%{_includedir}/*

%files devel-docs
%{_datadir}/*

%changelog
* Wed Oct 11 2017 Julien Courtat <julien.courtat@aqsacom.com> - 1.0.0
- Initial packaging
