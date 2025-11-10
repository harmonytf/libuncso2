pkgname=libuncso2
pkgver=1.2.0
pkgrel=1
pkgdesc="Backend for UnCSO2"
arch=(any)
url="https://github.com/harmonytf/libuncso2"
license=(GPL-3.0-only)
makedepends=(cmake)
source=()
_sourcedir="$pkgname"

prepare() {
	ln -snf "$startdir" "$srcdir"
}

build() {
	cmake -B build -S "$_sourcedir" \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -Wno-dev
    cmake --build build
}

check() {
	ctest --test-dir "$_sourcedir/build/tests" --build-config RelWithDebInfo --output-on-failure
}

package() {
	DESTDIR="$pkgdir" cmake --install build
    install -Dm644 "$_sourcedir/COPYING" -t "$pkgdir/usr/share/licenses/$pkgname"

    rm -rf "$pkgdir/usr/bin/cryptest"
    rm -rf "$pkgdir/usr/include/cryptopp"
    rm -rf "$pkgdir/usr/lib/libcryptopp.a"
    rm -rf "$pkgdir/usr/share/cmake"
    rm -rf "$pkgdir/usr/share/cryptopp"
    rm -rf "$pkgdir/usr/share/pkgconfig/cryptopp.pc"

    rm -rf "$pkgdir/usr/bin"
    rm -rf "$pkgdir/usr/share/pkgconfig"
}
