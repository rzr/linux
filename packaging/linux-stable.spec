#
# Spec written for Tizen Mobile, some bits and pieces originate
# from MeeGo/Moblin/Fedora
#

%define extraversion ""
%define upstream_version 4.2.0

%if !%{defined platform}
%define platform default
%endif

%define variant %{profile}-%{_arch}-%{platform}
%define kernel_version %{version}-%{release}
%define kernel_full_version %{version}-%{release}-%{variant}
%define arch_32bits i386 i586 i686 %{ix86}

# Default arch config for tizen per arch (unless overiden after)
%define kernel_image bzImage

%define dtbs_supported 0
%define modules_supported 1
%define trace_supported 1
%define uboot_supported 0
%define vdso_supported 1


# Overide per configuration

%ifarch %{arch_32bits}
%define kernel_arch i386
%define kernel_arch_subdir arch/x86
%define defconfig i386_defconfig
%endif

%ifarch x86_64
%define kernel_arch x86_64
%define kernel_arch_subdir arch/x86
%define defconfig x86_64_defconfig
%endif

%ifarch %arm
%define kernel_arch arm
%define kernel_arch_subdir arch/%{kernel_arch}
%define defconfig vexpress_defconfig
%define kernel_image zImage
%define vdso_supported 0
%define modules_supported 0
%endif


Name: linux-stable
Summary: Tizen kernel
Group: System/Kernel
License: GPL-2.0
URL: http://www.kernel.org/
Version: %{upstream_version}

# The below is used when we are on an -rc version
#%#define rc_num 6
#%#define release_ver 0
#%#define rc_str %{?rc_num:0.rc%{rc_num}}%{!?rc_num:1}
#%if ! 0%{?opensuse_bs}
#Release: 0
#%else
#Release: 0
#%endif
Release: 0

BuildRequires: module-init-tools
BuildRequires: findutils
BuildRequires: libelf-devel
BuildRequires: binutils-devel
BuildRequires: which
BuildRequires: bc
# net-tools provides the 'hostname' utility which kernel build wants
BuildRequires: net-tools
# The below is required for building perf
BuildRequires: flex
BuildRequires: bison
BuildRequires: libdw-devel
BuildRequires: python-devel
%if %{uboot_supported}
BuildRequires: u-boot-tools
%endif

ExclusiveArch: %{arch_32bits} x86_64 armv7l

Source0: %{name}-%{version}.tar.bz2

%description
This package contains the Linux kernel for Tizen.


%package -n kernel-%{variant}
Summary: Tizen kernel
Group: System/Kernel
Provides: kernel-profile-%{profile} = %{version}-%{release}
Provides: kernel-uname-r = %{kernel_full_version}
Requires(post): /usr/bin/ln
Requires(post): /usr/bin/sort
Requires(post): rpm

# We use 'setup-scripts-bootloader-conf' in post and postun, so ideally we need to
# have the below here, but this causes gbs/obs build failures like this:
#    "have choice for virtual-setup-scripts-bootloader needed by kernel-x86-scripts: setup-extlinux setup-gummiboot"
# The reason is that it will try to install the kernel to the build root, and
# fail with the above error. To fix it one would need to add 'setup-extlinux'
# or 'setup-gummiboot' to 'review.tizen.org/gerrit/scm/meta/build-config'. But
# it is probably not worth the trouble, so I commented out the below two lines.
# -- Artem
#Requires(post): virtual-setup-scripts-bootloader
#Requires(postun): virtual-setup-scripts-bootloader

#Requires(post): setup-gummiboot
#Requires(postun): setup-gummiboot

Requires(post): /usr/sbin/depmod
Requires(post): /usr/bin/dracut
Requires(post): /usr/bin/kmod

Requires(postun): /usr/bin/ln
Requires(postun): /usr/bin/sed
Requires(postun): rpm

# We can't let RPM do the dependencies automatic because it'll then pick up
# a correct but undesirable perl dependency from the module headers which
# isn't required for the kernel proper to function
AutoReq: no
AutoProv: yes

%description -n kernel-%{variant}
This package contains the Linux kernel for Tizen (%{profile} profile, architecure %{_arch})

%package -n kernel-%{variant}-devel
Summary: Development package for building kernel modules
Group: Development/System
Provides: kernel-devel = %{kernel_full_version}
Provides: kernel-devel-uname-r = %{kernel_full_version}
Requires(post): /usr/bin/find
Requires: kernel-%{variant} = %{version}-%{release}
AutoReqProv: no

%description -n kernel-%{variant}-devel
This package provides kernel headers and makefiles sufficient to build modules
against the %{variant} kernel package.


%package -n perf
Summary: The 'perf' performance counter tool
Group: System/Kernel
Provides: perf = %{kernel_full_version}
Requires: kernel-%{variant} = %{version}-%{release}

%description -n perf
This package provides the "perf" tool that can be used to monitor performance
counter events as well as various kernel internal events.



###
### PREP
###
%prep
# Unpack the kernel tarball
%setup -q -n %{name}-%{version}



###
### BUILD
###
%build
# Make sure EXTRAVERSION says what we want it to say
sed -i "s/^EXTRAVERSION.*/EXTRAVERSION = %{extraversion}-%{release}-%{variant}/" Makefile

# Build perf
make -s -C tools/lib/traceevent ARCH=%{kernel_arch} %{?_smp_mflags}
make -s -C tools/perf WERROR=0 ARCH=%{kernel_arch}

%if %{defined loadaddr}
export LOADADDR=%{loadaddr}
%endif

# Build kernel and modules
make -s ARCH=%{kernel_arch} %{defconfig}
make %{?_smp_mflags} %{kernel_image} ARCH=%{kernel_arch}

%if %modules_supported
make -s ARCH=%{kernel_arch} %{?_smp_mflags} modules
%endif

%if %dtbs_supported
make -s ARCH=%{kernel_arch} %{?_smp_mflags} dtbs
%endif



###
### INSTALL
###
%install
install -d %{buildroot}/boot

install -m 644 .config %{buildroot}/boot/config-%{kernel_full_version}
install -m 644 System.map %{buildroot}/boot/System.map-%{kernel_full_version}
install -m 755 %{kernel_arch_subdir}/boot/%{kernel_image} %{buildroot}/boot/vmlinuz-%{kernel_full_version}
# Dummy initrd, will not be included in the actual package but needed for files
touch %{buildroot}/boot/initrd-%{kernel_full_version}.img

%if %modules_supported
make -s ARCH=%{kernel_arch} INSTALL_MOD_PATH=%{buildroot} modules_install KERNELRELEASE=%{kernel_full_version}
%endif

%if %vdso_supported
make -s ARCH=%{kernel_arch} INSTALL_MOD_PATH=%{buildroot} vdso_install KERNELRELEASE=%{kernel_full_version}
%endif

%if %dtbs_supported
install -d "%{buildroot}/boot/"
find "arch/%{kernel_arch}/" -iname "*.dtb" -exec install "{}" "%{buildroot}/boot/" \;
%endif

rm -rf %{buildroot}/lib/firmware

# And save the headers/makefiles etc for building modules against
#
# This all looks scary, but the end result is supposed to be:
# * all arch relevant include/ files
# * all Makefile/Kconfig files
# * all script/ files

# Remove existing build/source links and create pristine dirs
rm -f %{buildroot}/lib/modules/%{kernel_full_version}/build
rm -f %{buildroot}/lib/modules/%{kernel_full_version}/source
install -d %{buildroot}/lib/modules/%{kernel_full_version}/build
ln -fs build %{buildroot}/lib/modules/%{kernel_full_version}/source


# First, copy all dirs containing Makefile of Kconfig files
cp --parents `find  -type f -name "Makefile*" -o -name "Kconfig*"` %{buildroot}/lib/modules/%{kernel_full_version}/build
install Module.symvers %{buildroot}/lib/modules/%{kernel_full_version}/build/
install System.map %{buildroot}/lib/modules/%{kernel_full_version}/build/

# Then, drop all but the needed Makefiles/Kconfig files
rm -rf %{buildroot}/lib/modules/%{kernel_full_version}/build/Documentation
rm -rf %{buildroot}/lib/modules/%{kernel_full_version}/build/scripts
rm -rf %{buildroot}/lib/modules/%{kernel_full_version}/build/include

# Copy config and scripts
install .config %{buildroot}/lib/modules/%{kernel_full_version}/build/
cp -a scripts %{buildroot}/lib/modules/%{kernel_full_version}/build
if [ -d %{kernel_arch_subdir}/scripts ]; then
    cp -a %{kernel_arch_subdir}/scripts %{buildroot}/lib/modules/%{kernel_full_version}/build/%{kernel_arch_subdir}/ || :
fi
if [ -f %{kernel_arch_subdir}/*lds ]; then
    cp -a %{kernel_arch_subdir}/*lds %{buildroot}/lib/modules/%{kernel_full_version}/build/%{kernel_arch_subdir}/ || :
fi
rm -f %{buildroot}/lib/modules/%{kernel_full_version}/build/scripts/*.o
rm -f %{buildroot}/lib/modules/%{kernel_full_version}/build/scripts/*/*.o
cp -a --parents %{kernel_arch_subdir}/include %{buildroot}/lib/modules/%{kernel_full_version}/build

# Copy include files
mkdir -p %{buildroot}/lib/modules/%{kernel_full_version}/build/include
find include/ -mindepth 1 -maxdepth 1 -type d | xargs -I{} cp -a {} %{buildroot}/lib/modules/%{kernel_full_version}/build/include

# Save the vmlinux file for kernel debugging into the devel package
cp vmlinux %{buildroot}/lib/modules/%{kernel_full_version}

# Mark modules executable so that strip-to-file can strip them
find %{buildroot}/lib/modules/%{kernel_full_version} -name "*.ko" -type f | xargs --no-run-if-empty chmod 755

# Move the devel headers out of the root file system
install -d %{buildroot}/usr/src/kernels
mv %{buildroot}/lib/modules/%{kernel_full_version}/build %{buildroot}/usr/src/kernels/%{kernel_full_version}

ln -sf /usr/src/kernels/%{kernel_full_version} %{buildroot}/lib/modules/%{kernel_full_version}/build

# Install perf
install -d %{buildroot}
make -s -C tools/perf DESTDIR=%{buildroot} install
install -d  %{buildroot}%{_bindir}
install -d  %{buildroot}%{_libexecdir}
mv %{buildroot}/bin/* %{buildroot}%{_bindir}
mv %{buildroot}/libexec/* %{buildroot}%{_libexecdir}
rm %{buildroot}/etc/bash_completion.d/perf

# Dont package debug files
rm -rf %{buildroot}/usr/lib/debug/.build-id
rm -rf %{buildroot}/usr/lib/debug/lib/traceevent/plugins/*.debug



###
### SCRIPTS
###

%post -n kernel-%{variant}
if [ -f "/boot/loader/loader.conf" ]; then
	# EFI boot with gummiboot
	INSTALLERFW_MOUNT_PREFIX="/" /bin/sh /usr/sbin/setup-gummiboot-conf \
	    add -f \
	    "%{kernel_full_version}" "%{kernel_full_version}" "vmlinuz-%{kernel_full_version}" \
	    "$(cat /proc/cmdline)"
    # "/etc/installerfw-environment" does not exist in MIC environment, when it
    # builds the image. MIC will add boot-loader entries later using the
    # 'setup-scripts-boot' script.
    if [ -f "/etc/installerfw-environment" ] && \
        [ -x "/usr/sbin/setup-scripts-bootloader-conf" ]; then
            /usr/sbin/setup-scripts-bootloader-conf add -f vmlinuz-%{kernel_full_version}
            /usr/sbin/setup-scripts-bootloader-conf default -f vmlinuz-%{kernel_full_version}
    fi
else
	# Legacy boot
	last_installed_ver="$(rpm -q --qf '%{INSTALLTIME}: %{VERSION}-%{RELEASE}\n' kernel-%{variant} | sort -r | sed -e 's/[^:]*: \(.*\)/\1/g' | sed -n -e "1p")"
	ln -sf vmlinuz-$last_installed_ver-%{variant} /boot/vmlinuz

	if [ -z "$last_installed_ver" ]; then
		# Something went wrong, print some diagnostics
		printf "%s\n" "Error: cannot find kernel version" 1>&2
		printf "%s\n" "The command was: rpm -q --qf '%{INSTALLTIME}: %{VERSION}-%{RELEASE}\n' kernel-%{variant} | sort -r | sed -e 's/[^:]*: \(.*\)/\1/g' | sed -n -e \"1p\"" 1>&2
		printf "%s\n" "Output of the \"rpm -q --qf '%{INSTALLTIME}: %{VERSION}-%{RELEASE}\n' kernel-%{variant}\" is:" 1>&2
		result="$(rpm -q --qf '%{INSTALLTIME}: %{VERSION}-%{RELEASE}\n' kernel-%{variant})"
		printf "%s\n" "$result" 1>&2
	fi
	if [ -x "/usr/sbin/setup-scripts-bootloader-conf" ]; then
            /usr/sbin/setup-scripts-bootloader-conf add -f vmlinuz-%{kernel_full_version}
            /usr/sbin/setup-scripts-bootloader-conf default -f vmlinuz-%{kernel_full_version}
	fi
fi

%{_bindir}/dracut /boot/initrd-%{kernel_full_version}.img %{kernel_full_version}

%post -n kernel-%{variant}-devel
if [ -x /usr/sbin/hardlink ]; then
	cd /usr/src/kernels/%{kernel_full_version}
	/usr/bin/find . -type f | while read f; do
		hardlink -c /usr/src/kernels/*/$f $f
	done
fi

%postun -n kernel-%{variant}
if [ -f "/boot/loader/loader.conf" ]; then
	# EFI boot with gummiboot
	INSTALLERFW_MOUNT_PREFIX="/" /bin/sh /usr/sbin/setup-gummiboot-conf \
	    remove "vmlinuz-%{kernel_full_version}"
    if [ -f "/etc/installerfw-environment" ] && \
        [ -x "/usr/sbin/setup-scripts-bootloader-conf" ]; then
            /usr/sbin/setup-scripts-bootloader-conf remove -f vmlinuz-%{kernel_full_version}
    fi

else
	last_installed_ver="$(rpm -q --qf '%{INSTALLTIME}: %{VERSION}-%{RELEASE}\n' kernel-%{variant} | sort -r | sed -e 's/[^:]*: \(.*\)/\1/g' | sed -n -e "1p")"
	if [ -n "$last_installed_ver" ]; then
		ln -sf vmlinuz-$last_installed_ver-%{variant} /boot/vmlinuz
	else
		rm -rf /boot/vmlinuz
	fi
fi



###
### FILES
###
%files -n kernel-%{variant}
%license COPYING
/boot/vmlinuz-%{kernel_full_version}
/boot/System.map-%{kernel_full_version}
/boot/config-%{kernel_full_version}
%if %modules_supported
%dir /lib/modules/%{kernel_full_version}
/lib/modules/%{kernel_full_version}/kernel
/lib/modules/%{kernel_full_version}/modules.*
%endif
/lib/modules/%{kernel_full_version}/build
/lib/modules/%{kernel_full_version}/source

%if %vdso_supported
/lib/modules/%{kernel_full_version}/vdso
%endif
%if %dtbs_supported
/boot/*.dtb
%endif
%ghost /boot/initrd-%{kernel_full_version}.img


%files -n kernel-%{variant}-devel
%license COPYING
%verify(not mtime) /usr/src/kernels/%{kernel_full_version}
/lib/modules/%{kernel_full_version}/vmlinux


%files -n perf
%license COPYING
%{_bindir}/perf
%{_libexecdir}/perf-core
%if %trace_supported
%{_bindir}/trace
/%{_lib}/traceevent/plugins/*.so
%endif
