%global _hardened_build	1

%{!?release_func:%global release_func() %1%{?dist}}

%global makeflags prefix=%_prefix libdir=%_libdir libexecdir=%_libexecdir CFLAGS="$RPM_OPT_FLAGS -I%_includedir/et" LDFLAGS="%{?__global_ldflags}"


Name:		krb5-cracklib
Version:	0.1.4
Release:	%release_func 1
Summary:	crack pwqual plugin

License:	GPLv3
Source0:	%name-%version.tar.xz

BuildRequires:	cracklib-devel krb5-devel
Requires:	cracklib-dicts

%description


%prep
%setup -q


%build
make %{?_smp_mflags} CFLAGS="$RPM_OPT_FLAGS" %makeflags


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT %makeflags


%files
%doc
%_libdir/krb5/plugins/pwqual/*.so
%_libexecdir/*


%changelog
