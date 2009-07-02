# This script was contributed by Marcus Alanen.
#
# Henning Makholm, the xcftools author, have not had occasion to test
# it, so use at your own peril. In particular, note that there is a
# hard-coded version number just a few lines in. I make no promises
# that I'll remember to update it for versions 1.0.7 - caveat emptor!

#############################################################################
# Variables
#############################################################################
%define name    xcftools
%define version 1.0.7
%define release 1

#############################################################################
# Preamble. This contains general information about the package.
#############################################################################
Summary   : xcftools
Name      : %{name} 
Version   : %{version} 
Release   : %{release} 
License   : Public Domain
Group     : Development/Tools
Source    : xcftools-%version.tar.gz
Packager  : Marcus Alanen <marcus.alanen@gmail.com>

BuildRoot : %{_tmppath}/%{name}-buildroot
Prefix    : /usr
Requires  : glibc

#############################################################################
# A description of the project
#############################################################################
%description
xcftools is a set of tools for extracting information from GIMP's xcf
files.

#############################################################################
# Preparations for the install. We don't need to do anything special here,
# so we let the %setup-macro take care of creating a directory and unpacking
# the sources.
#############################################################################
%prep
rm -rf $RPM_BUILD_ROOT 
%setup
%patch0 -p1

#############################################################################
# Build the project
#############################################################################
%build
./configure --prefix=/usr && make DESTDIR=$RPM_BUILD_ROOT

#############################################################################
# Install the software
#############################################################################
%install
mkdir $RPM_BUILD_ROOT/usr || true
mkdir $RPM_BUILD_ROOT/usr/share || true
mkdir $RPM_BUILD_ROOT/usr/share/man || true
mkdir $RPM_BUILD_ROOT/usr/share/man/man1/ || true

make install DESTDIR=$RPM_BUILD_ROOT

cd $RPM_BUILD_ROOT

function getfiles() {
	# Change /usr/bin to ./usr/bin
	dir=$(echo $1 | sed -e 's|.|\.\/|;')
	find $dir -type d -print | sed 's,^\.,\%attr(-\,root\,root) \%dir ,'

	find $dir -type f -print | sed -e 's,^\.,\%attr(-\,root\,root) ,' \
	         -e '/\/etc\//s|^|%config|' \
	         -e '/\/config\//s|^|%config|'

	find $dir -type l -print | sed 's,^\.,\%attr(-\,root\,root) ,' 
}

all=$RPM_BUILD_DIR/filelist.%{name}

getfiles %{prefix} | grep -v /man/ > $all

#############################################################################
# Files to be included in the package.
#############################################################################
%files -f ../filelist.%{name}
%defattr(-,root,root,0755)
%{_mandir}/*/*

#############################################################################
# Cleaning up stuff.
#############################################################################
%clean
make clean
rm -rf $RPM_BUILD_ROOT 

