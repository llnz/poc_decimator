#! /bin/sh

echo aclocal...
(aclocal --version) < /dev/null > /dev/null 2>&1 || {
    echo aclocal not found
    exit 1
}

aclocal $ACLOCAL_FLAGS

echo autoheader...
(autoheader --version) < /dev/null > /dev/null 2>&1 || {
    echo autoheader not found
    exit 1
}    

autoheader

echo libtoolize...
(libtoolize --version || glibtoolize --version) < /dev/null > /dev/null 2>&1 || {
    echo libtoolize not found
    exit 1
}

#try with recursive first, then without
libtoolize --automake --copy --force --ltdl --recursive || libtoolize --automake --copy --force --ltdl || glibtoolize --automake --copy --force --ltdl --recursive || glibtoolize --automake --copy --force --ltdl

echo automake...
(automake --version) < /dev/null > /dev/null 2>&1 || {
    echo automake not found
    exit 1
}

automake --add-missing --copy --gnu

echo autoconf...
(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo autoconf not found
    exit 1
}

autoconf

echo removing config.cache...

rm -f config.cache

echo

echo ready to configure

exit 0
