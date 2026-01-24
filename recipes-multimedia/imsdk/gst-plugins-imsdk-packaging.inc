# This .inc file contains functionality for automatically splitting
# built plugins into individual packages for each plugin. A -meta
# package is also set up that has no files of its own, but contains
# the names of all plugin packages in its RDEPENDS list.

# Dynamically generate packages for all enabled plugins
PACKAGES_DYNAMIC = "^${PN}-.* ^libgst.*"

PACKAGESPLITFUNCS =+ "split_imsdk_packages"
PACKAGESPLITFUNCS += "set_imsdk_metapkg_rdepends"

python split_imsdk_packages () {
    gst_libdir = d.expand('${libdir}/gstreamer-1.0')
    postinst = d.getVar('plugin_postinst')
    glibdir = d.getVar('libdir')
    imsdk_root = d.expand('${libdir}/imsdk')

    # Qualcomm GStreamer libraries
    do_split_packages(d, glibdir, r'^lib(.*)\.so\.*', 'lib%s', 'Qualcomm GStreamer 1.0 %s library', extra_depends='', allow_links=True)
    # Qualcomm GStreamer plugin shared objects
    do_split_packages(d, gst_libdir, r'libgstqti(.*)\.so$', d.expand('${PN}-%s'), 'Qualcomm GStreamer 1.0 plugin for qti%s', postinst=postinst, extra_depends='')
    # Qualcomm runtime modules for plugin
    do_split_packages(d, imsdk_root, r'([^/]+)/modules/[^/]+\.so', d.expand('${PN}-%s-modules'), 'IMSDK runtime modules for qti%s plugin', recursive=True, extra_depends='', match_path=True)
}

python set_imsdk_metapkg_rdepends () {
    import os
    import oe.utils

    # Go through all generated packages (excluding the main package and
    # the -meta package itself) and add them to the -meta package as RDEPENDS.

    pn = d.getVar('PN')
    metapkg =  pn + '-meta'
    d.setVar('ALLOW_EMPTY:' + metapkg, "1")
    d.setVar('FILES:' + metapkg, "")
    exclude = [ pn, pn + '-meta' ]
    metapkg_rdepends = []
    pkgdest = d.getVar('PKGDEST')
    for pkg in oe.utils.packages_filter_out_system(d):
        if pkg not in exclude and pkg not in metapkg_rdepends:
            # See if the package is empty by looking at the contents of its PKGDEST subdirectory.
            # If this subdirectory is empty, then the package is.
            # Empty packages do not get added to the meta package's RDEPENDS
            pkgdir = os.path.join(pkgdest, pkg)
            if os.path.exists(pkgdir):
                dir_contents = os.listdir(pkgdir) or []
            else:
                dir_contents = []
            is_empty = len(dir_contents) == 0
            if not is_empty:
                metapkg_rdepends.append(pkg)
    d.setVar('RDEPENDS:' + metapkg, ' '.join(metapkg_rdepends))
    d.setVar('DESCRIPTION:' + metapkg, pn + ' meta package')
}

ALLOW_EMPTY:${PN} = "1"

PACKAGES += "${PN}-apps ${PN}-meta"

FILES:${PN} = ""
FILES:${PN}-apps = "${bindir}"

RRECOMMENDS:${PN} += "${PN}-meta"
