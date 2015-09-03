Summary: Ctwm is a window manager for the X Window System.
Name: ctwm
Version: 3.8.1
Release: 1
Source: http://www.ctwm.org/dist/%{name}-%{version}.tar.gz
Copyright: MIT
Group: User Interface/X
BuildRoot: /var/tmp/%{name}-%{version}-buildroot
URL: http://www.ctwm.org/

BuildRequires: xorg-x11-devel
Requires: m4

%description
Ctwm is a window manager for the X Window System.  It provides
titlebars, shaped windows, virtual screens (workspaces), several forms
of icon management, user-defined macro functions, click-to-type and
pointer-driven keyboard focus, and user-specified key and pointer
button bindings.  It is actually twm (Tab Window Manager) from the MIT
X11 distribution slightly modified to accommodate the use of several
virtual screens (workspaces). It is heavily inspired from the
Hewlett-Packard vuewm window manager.  In addition, ctwm can use
coloured, shaped icons and background root pixmaps in XPM and JPG format,
as well as any format understood by the imconv package [from the
San Diego Supercomputer Center] and xwd files.  Ctwm can be compiled
to use both, either or none of the above icon/pixmap formats.

%prep

%setup -q -n %{name}-%{version}

%build
cp Imakefile.local-template Imakefile.local
xmkmf
make

%install
rm -fr $RPM_BUILD_ROOT

# Call the man page something a little more Unixly.
mkdir -p $RPM_BUILD_ROOT/usr/X11R6/man/man1

# RedHat-ish OSes have the configuration file in /etc/X11/ctwm,
# possibly with a symlink /usr/X11R6/lib/X11/ctwm pointing at it.
make install \
    DESTDIR=$RPM_BUILD_ROOT \
    PIXMAPDIR=/usr/X11R6/include/X11/pixmaps \
    CONFDIR=/etc/X11/ctwm
%{__mkdir_p} $RPM_BUILD_ROOT/usr/X11R6/lib/X11
%{__ln_s} ../../../../etc/X11/ctwm $RPM_BUILD_ROOT/usr/X11R6/lib/X11/ctwm

# Install the manual page separately.
install -c -m 0644 ctwm.man $RPM_BUILD_ROOT/usr/X11R6/man/man1/ctwm.1x

%clean
rm -fr $RPM_BUILD_ROOT

%files
%defattr(0644,root,root,0755)
%doc README CHANGES PROBLEMS README.gnome TODO.gnome
%doc *.ctwmrc
%doc sound.doc vms.txt vms2.txt

%attr(0755,root,root) /usr/X11R6/bin/ctwm
%attr(0644,root,root) /usr/X11R6/man/man1/ctwm.1x.gz

/usr/X11R6/lib/X11/ctwm
%config %attr(0644,root,root) /etc/X11/ctwm/system.ctwmrc
%attr(0644,root,root) /usr/X11R6/include/X11/pixmaps/*

%changelog
* Thu Jan  5 2012 Richard Levitte <richard@levitte.org>
- Release ctwm 3.8.1.
* Fri Feb 16 2006 Richard Levitte <richard@levitte.org>
- Release ctwm 3.8a.
* Wed May  4 2005 Rudolph T Maceyko <rm55@pobox.com>
- Tweaks.  Added all .ctwmrc files as well as sound and VMS docs.
* Wed May  4 2005 Richard Levitte <richard@levitte.org>
- Changed some directory specifications to RedHat-ish standards.
* Tue May  3 2005 Richard Levitte <richard@levitte.org>
- Received the original from Johan Vromans.  Adjusted it to become
  an official .spec file.
