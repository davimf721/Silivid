// Verifies the macOS toolchain end to end: the portable Veyra objects, Qt/QML
// and FFmpeg all link, and the libraries loaded at run time match the headers.
// It renders nothing and is not a player.
#include <QCoreApplication>
#include <QJSValue>
#include <QQmlEngine>
#include <QtGlobal>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <cstdio>

namespace {
bool checkMajor(const char* name, unsigned runtime, unsigned headerMajor) {
    const unsigned runtimeMajor = runtime >> 16;
    const bool ok = runtimeMajor == headerMajor;
    std::printf("%s: runtime %u.%u.%u, headers major %u (%s)\n", name, runtimeMajor,
                (runtime >> 8) & 0xFFu, runtime & 0xFFu, headerMajor, ok ? "ok" : "MISMATCH");
    return ok;
}
} // namespace

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    bool ok = checkMajor("libavcodec", avcodec_version(), LIBAVCODEC_VERSION_MAJOR);
    ok &= checkMajor("libavformat", avformat_version(), LIBAVFORMAT_VERSION_MAJOR);
    ok &= checkMajor("libavutil", avutil_version(), LIBAVUTIL_VERSION_MAJOR);
    ok &= checkMajor("libswscale", swscale_version(), LIBSWSCALE_VERSION_MAJOR);
    ok &= checkMajor("libswresample", swresample_version(), LIBSWRESAMPLE_VERSION_MAJOR);

    QQmlEngine engine;
    const QJSValue result = engine.evaluate(QStringLiteral("6 * 7"));
    const bool qmlOk = !result.isError() && result.toInt() == 42;
    std::printf("Qt: runtime %s, headers %s, QML engine %s\n", qVersion(), QT_VERSION_STR,
                qmlOk ? "ok" : "FAILED");
    ok &= qmlOk;

    std::puts(ok ? "PASS macos build probe" : "FAIL macos build probe");
    return ok ? 0 : 1;
}
