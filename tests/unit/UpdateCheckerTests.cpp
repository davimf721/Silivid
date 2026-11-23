#include "veyra/base/SemVer.h"
#include "veyra/ui/UpdateChecker.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtTest>
#include <cstdio>
#include <memory>

using veyra::ui::UpdateChecker;
namespace {
const auto pagePrefix = QStringLiteral("https://github.com/davimf721/Silivid/releases/tag/");

QJsonObject entry(const QString& tag, bool prerelease = false, bool draft = false, QString url = {}) {
    if (url.isEmpty()) url = pagePrefix + tag;
    return QJsonObject{{"tag_name", tag}, {"draft", draft}, {"prerelease", prerelease}, {"html_url", url}};
}
QByteArray releases(const QJsonArray& items) { return QJsonDocument(items).toJson(); }
QByteArray one(const QString& tag, bool prerelease = false, bool draft = false, const QString& url = {}) {
    return releases({entry(tag, prerelease, draft, url)});
}
class Server : public QTcpServer {
public:
    QByteArray body = one("silivid-v0.1.0"), request, extraHeader;
    int status = 200, requests = 0;
    bool hang = false;
    Server() {
        if (!listen(QHostAddress::LocalHost)) qFatal("Cannot listen on localhost");
        connect(this, &QTcpServer::newConnection, this, [this] {
            while (hasPendingConnections()) {
                auto* socket = nextPendingConnection();
                auto header = std::make_shared<QByteArray>();
                connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
                connect(socket, &QTcpSocket::readyRead, socket, [this, socket, header] {
                    *header += socket->readAll();
                    if (!header->contains("\r\n\r\n")) return;
                    socket->disconnect(socket, &QTcpSocket::readyRead, nullptr, nullptr);
                    ++requests; request = *header;
                    if (hang) return;
                    socket->write("HTTP/1.1 " + QByteArray::number(status) + " Test\r\nContent-Type: application/json\r\n"
                                  + extraHeader + "Content-Length: " + QByteArray::number(body.size())
                                  + "\r\nConnection: close\r\n\r\n" + body);
                    socket->disconnectFromHost();
                });
            }
        });
    }
    QUrl endpoint() const { return QUrl(QStringLiteral("http://127.0.0.1:%1/releases").arg(serverPort())); }
};
int order(const char* a, const char* b) {
    const auto left = veyra::semver::parse(a), right = veyra::semver::parse(b);
    if (!left || !right) qFatal("Invalid version in test: %s, %s", a, b);
    return veyra::semver::compare(*left, *right);
}
}
class UpdateCheckerTests : public QObject {
    Q_OBJECT
private slots:
    void semverPrecedence() {
        const char* ascending[] = {"0.1.0-alpha", "0.1.0-alpha.1", "0.1.0-alpha.2", "0.1.0-alpha.10", "0.1.0-alpha.beta",
                                   "0.1.0-beta.1", "0.1.0-beta.2", "0.1.0-rc.1", "0.1.0", "0.1.1-alpha.1", "0.1.1",
                                   "0.2.0", "0.10.0", "1.0.0"};
        const int count = int(std::size(ascending));
        for (int i = 0; i < count; ++i)
            for (int j = 0; j < count; ++j) {
                const int result = order(ascending[i], ascending[j]);
                QVERIFY2(i < j ? result < 0 : (i > j ? result > 0 : result == 0), ascending[i]);
            }
        QVERIFY(order("1.0.0-1", "1.0.0-a") < 0);
    }
    void semverRejects_data() {
        QTest::addColumn<QString>("text");
        for (const char* text : {"", "1", "1.2", "1.2.3.4", "v1.2.3", "01.2.3", "1.02.3", "1.2.03", "1.2.3-", "1.2.3-a..b",
                                 "1.2.3-01", "1.2.3+build", "1.2.3-a+build", "1.2.3-a_b", " 1.2.3", "1.2.3 ",
                                 "1.2.99999999999999999999", "1.2.3-99999999999999999999"})
            QTest::newRow(text) << QString::fromLatin1(text);
    }
    void semverRejects() {
        QFETCH(QString, text);
        QVERIFY(!veyra::semver::parse(text.toStdString()));
    }
    void identity() {
        // The value lives only in cmake/SilividVersion.cmake.
        QVERIFY(veyra::semver::parse(UpdateChecker::productVersion().toStdString()).has_value());
        const auto url = UpdateChecker::releasesEndpoint();
        QCOMPARE(url.scheme(), QStringLiteral("https"));
        QCOMPARE(url.host(), QStringLiteral("api.github.com"));
        QCOMPARE(url.path(), QStringLiteral("/repos/davimf721/Silivid/releases"));
    }
    void response_data() {
        QTest::addColumn<QString>("local"); QTest::addColumn<bool>("prereleaseChannel");
        QTest::addColumn<QByteArray>("body"); QTest::addColumn<int>("http");
        QTest::addColumn<QString>("result"); QTest::addColumn<QString>("version");
        const QString alpha1 = "0.1.0-alpha.1";
        const auto row = [&](const char* name, const QString& local, bool pre, const QByteArray& body,
                             const char* result, const char* version = "", int http = 200) {
            QTest::newRow(name) << local << pre << body << http << QString::fromLatin1(result) << QString::fromLatin1(version);
        };
        row("available", alpha1, false, one("silivid-v0.1.0"), "available", "0.1.0");
        row("numeric-10-over-9", "0.9.0", false, one("silivid-v0.10.0"), "available", "0.10.0");
        row("up-to-date", "0.1.0", false, one("silivid-v0.1.0"), "current");
        row("local-newer", "0.2.0", false, one("silivid-v0.1.0"), "current");
        row("no-releases", alpha1, false, releases({}), "current");
        row("no-releases-prerelease", alpha1, true, releases({}), "current");

        row("prerelease-stable-channel", alpha1, false, one("silivid-v0.1.0-alpha.2", true), "current");
        row("prerelease-opt-in", alpha1, true, one("silivid-v0.1.0-alpha.2", true), "available", "0.1.0-alpha.2");
        row("prerelease-same", alpha1, true, one("silivid-v0.1.0-alpha.1", true), "current");
        row("prerelease-older", "0.1.0-beta.1", true, one("silivid-v0.1.0-alpha.2", true), "current");
        row("final-over-own-prerelease", "0.1.0-beta.1", false, one("silivid-v0.1.0"), "available", "0.1.0");
        row("prerelease-below-installed-final", "0.1.0", true, one("silivid-v0.1.0-rc.1", true), "current");
        row("highest-any-order", alpha1, true, releases({entry("silivid-v0.1.0-alpha.2", true), entry("silivid-v0.1.0-beta.1", true),
            entry("silivid-v0.1.0-alpha.10", true)}), "available", "0.1.0-beta.1");
        row("stable-channel-skips-newer-pre", alpha1, false, releases({entry("silivid-v0.2.0-alpha.1", true), entry("silivid-v0.1.0")}),
            "available", "0.1.0");
        row("pre-channel-prefers-newer-pre", alpha1, true, releases({entry("silivid-v0.1.0"), entry("silivid-v0.2.0-alpha.1", true)}),
            "available", "0.2.0-alpha.1");
        row("pre-channel-final-beats-its-pre", alpha1, true, releases({entry("silivid-v0.1.0-rc.1", true), entry("silivid-v0.1.0")}),
            "available", "0.1.0");

        row("draft-stable", alpha1, false, one("silivid-v0.2.0", false, true), "current");
        row("draft-prerelease-channel", alpha1, true, one("silivid-v0.2.0-alpha.1", true, true), "current");
        row("draft-beside-real", alpha1, false, releases({entry("silivid-v9.0.0", false, true), entry("silivid-v0.1.0")}),
            "available", "0.1.0");

        row("legacy-v2.2.0", alpha1, false, one("v2.2.0"), "current");
        row("legacy-v2.2.0-prerelease-channel", alpha1, true, one("v2.2.0"), "current");
        row("legacy-sources", alpha1, true, one("v2.2.0-sources", true), "current");
        row("legacy-unprefixed", alpha1, false, one("0.2.0"), "current");
        row("legacy-v0-style", alpha1, false, one("v0.2.0"), "current");
        row("legacy-upstream-url", alpha1, false,
            one("v2.2.0", false, false, "https://github.com/Likely7/Veyra-NRVideo/releases/tag/v2.2.0"), "current");
        row("legacy-beside-real", alpha1, false, releases({entry("v2.2.0"), entry("silivid-v0.1.0")}), "available", "0.1.0");

        const QString tag = "silivid-v0.2.0";
        row("url-upstream", alpha1, false, one(tag, false, false, "https://github.com/Likely7/Veyra-NRVideo/releases/tag/" + tag), "invalid-release");
        row("url-other-repo", alpha1, false, one(tag, false, false, "https://github.com/other/Silivid/releases/tag/" + tag), "invalid-release");
        row("url-other-host", alpha1, false, one(tag, false, false, "https://example.com/davimf721/Silivid/releases/tag/" + tag), "invalid-release");
        row("url-lookalike-host", alpha1, false, one(tag, false, false, "https://github.com.example.org/davimf721/Silivid/releases/tag/" + tag), "invalid-release");
        row("url-http", alpha1, false, one(tag, false, false, "http://github.com/davimf721/Silivid/releases/tag/" + tag), "invalid-release");
        row("url-userinfo", alpha1, false, one(tag, false, false, "https://github.com@example.com/davimf721/Silivid/releases/tag/" + tag), "invalid-release");
        row("url-other-tag", alpha1, false, one(tag, false, false, pagePrefix + "silivid-v0.3.0"), "invalid-release");
        row("url-query", alpha1, false, one(tag, false, false, pagePrefix + tag + "?x=1"), "invalid-release");
        row("url-fragment", alpha1, false, one(tag, false, false, pagePrefix + tag + "#x"), "invalid-release");
        row("url-traversal", alpha1, false, one(tag, false, false, pagePrefix + "../../other/" + tag), "invalid-release");
        row("bad-url-beside-real", alpha1, false, releases({entry("silivid-v9.0.0", false, false, "https://example.com/x"), entry("silivid-v0.1.0")}),
            "available", "0.1.0");

        row("tag-not-semver", alpha1, false, one("silivid-vnext"), "invalid-release");
        row("tag-build-metadata", alpha1, false, one("silivid-v0.2.0+1"), "invalid-release");
        row("tag-leading-zero", alpha1, false, one("silivid-v0.02.0"), "invalid-release");
        row("flag-missing-on-alpha", alpha1, true, one("silivid-v0.2.0-alpha.1", false), "invalid-release");
        row("flag-set-on-final", alpha1, true, one("silivid-v0.2.0", true), "invalid-release");
        row("missing-fields", alpha1, false, QByteArray("[{\"tag_name\":\"silivid-v0.2.0\"}]"), "invalid-release");
        row("non-object-entry", alpha1, false, QByteArray("[1]"), "invalid-release");

        row("json-garbage", alpha1, false, QByteArray("not json"), "invalid-json");
        row("json-truncated", alpha1, false, one("silivid-v0.1.0").left(12), "invalid-json");
        row("json-object", alpha1, false, QJsonDocument(entry("silivid-v0.1.0")).toJson(), "invalid-json");
        row("json-empty", alpha1, false, QByteArray(), "invalid-json");
        row("too-large", alpha1, false, QByteArray(2 * 1024 * 1024 + 1, 'x'), "response-too-large");
        row("http-404", alpha1, false, one("silivid-v0.1.0"), "http-404-", "", 404);
        row("http-403-rate-limit", alpha1, false, one("silivid-v0.1.0"), "http-403-", "", 403);
        row("http-500", alpha1, false, one("silivid-v0.1.0"), "http-500-", "", 500);
        row("unknown-local", "dev", false, one("silivid-v0.1.0"), "invalid-local-version");
        row("inherited-local-format", "v2.2.0", false, one("silivid-v3.0.0"), "invalid-local-version");
    }
    void response() {
        QFETCH(QString, local); QFETCH(bool, prereleaseChannel); QFETCH(QByteArray, body);
        QFETCH(int, http); QFETCH(QString, result); QFETCH(QString, version);
        for (bool manual : {false, true}) {
            Server server; server.body = body; server.status = http;
            UpdateChecker checker(local, nullptr, server.endpoint());
            checker.setPrereleaseChannel(prereleaseChannel);
            QSignalSpy done(&checker, &UpdateChecker::completed), available(&checker, &UpdateChecker::updateAvailable),
                       notices(&checker, &UpdateChecker::notice);
            checker.check(manual);
            QVERIFY(checker.checking());
            QTRY_COMPARE_WITH_TIMEOUT(done.size(), 1, 3000);
            const auto actual = done[0][0].toString();
            if (result.endsWith(u'-')) QVERIFY2(actual.startsWith(result), qPrintable(actual));
            else QCOMPARE(actual, result);
            QCOMPARE(checker.checking(), false);
            QCOMPARE(server.requests, 1);
            QCOMPARE(available.size(), result == "available" ? 1 : 0);
            QCOMPARE(notices.size(), manual && result != "available" ? 1 : 0);
            QVERIFY(server.request.toLower().contains("user-agent: silivid-update-check"));
            QVERIFY(!server.request.toLower().contains("authorization:"));
            QCOMPARE(checker.latestVersion(), version);
            if (result == "available") QCOMPARE(checker.releaseUrl(), QUrl(pagePrefix + "silivid-v" + version));
            else QVERIFY(checker.releaseUrl().isEmpty());
        }
    }
    void stableIsDefaultAndChannelClearsResult() {
        Server server; server.body = one("silivid-v0.1.0-alpha.2", true);
        UpdateChecker checker("0.1.0-alpha.1", nullptr, server.endpoint());
        QVERIFY(!checker.prereleaseChannel());
        QSignalSpy done(&checker, &UpdateChecker::completed);
        checker.setPrereleaseChannel(true); checker.check(false);
        QTRY_COMPARE_WITH_TIMEOUT(done.size(), 1, 3000);
        QCOMPARE(checker.latestVersion(), QStringLiteral("0.1.0-alpha.2"));
        checker.setPrereleaseChannel(false);
        QVERIFY(checker.latestVersion().isEmpty()); QVERIFY(checker.releaseUrl().isEmpty());
    }
    void redirectIsNotFollowed() {
        Server target; Server server; server.status = 302;
        server.extraHeader = "Location: " + target.endpoint().toEncoded() + "\r\n";
        UpdateChecker checker("0.1.0-alpha.1", nullptr, server.endpoint());
        QSignalSpy done(&checker, &UpdateChecker::completed);
        checker.check(false);
        QTRY_COMPARE_WITH_TIMEOUT(done.size(), 1, 3000);
        QVERIFY(done[0][0].toString().startsWith("http-302-"));
        QTest::qWait(100);
        QCOMPARE(target.requests, 0);
        QVERIFY(checker.latestVersion().isEmpty());
    }
    void portableFlowIsClosed() {
        Server server;
        UpdateChecker checker("0.1.0-alpha.1", nullptr, server.endpoint());
        QSignalSpy done(&checker, &UpdateChecker::completed);
        auto* portable = checker.portableUpdate();
        checker.check(false);
        QTRY_COMPARE_WITH_TIMEOUT(done.size(), 1, 3000);
        QCOMPARE(done[0][0].toString(), QStringLiteral("available"));
        portable->selectRelease("0.1.0");
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression("automatic update is disabled"));
        portable->downloadAndSchedule();
        portable->cancel();
        QTest::qWait(100);
        QCOMPARE(server.requests, 1);
        QVERIFY(!portable->available()); QVERIFY(!portable->downloading()); QVERIFY(!portable->scheduled());
        QCOMPARE(portable->progress(), 0); QVERIFY(portable->sizeText().isEmpty()); QVERIFY(portable->releaseNotes().isEmpty());
    }
    void timeoutAndCoalescing() {
        Server server; server.hang = true;
        UpdateChecker checker("0.1.0-alpha.1", nullptr, server.endpoint(), 200);
        QSignalSpy done(&checker, &UpdateChecker::completed), notices(&checker, &UpdateChecker::notice);
        checker.check(false); checker.check(true); checker.check(true);
        QTRY_COMPARE_WITH_TIMEOUT(done.size(), 1, 2000);
        QCOMPARE(server.requests, 1); QCOMPARE(notices.size(), 1);
        QCOMPARE(done[0][0].toString(), QString("timeout"));
        checker.check(false);
        QTRY_COMPARE_WITH_TIMEOUT(done.size(), 2, 2000);
        QCOMPARE(server.requests, 2); QCOMPARE(notices.size(), 1);
    }
    void startupOnce() {
        Server server; server.body = one("silivid-v0.1.0-alpha.1", true);
        UpdateChecker checker("0.1.0-alpha.1", nullptr, server.endpoint());
        QSignalSpy done(&checker, &UpdateChecker::completed), notices(&checker, &UpdateChecker::notice);
        checker.startAutomaticCheck(); checker.startAutomaticCheck();
        QCOMPARE(server.requests, 0);
        QTRY_COMPARE_WITH_TIMEOUT(done.size(), 1, 4000);
        checker.startAutomaticCheck(); QTest::qWait(2700);
        QCOMPARE(server.requests, 1); QCOMPARE(notices.size(), 0);
    }
    void manualBeforeStartup() {
        Server server;
        UpdateChecker checker("0.1.0-alpha.1", nullptr, server.endpoint());
        QSignalSpy done(&checker, &UpdateChecker::completed);
        checker.startAutomaticCheck(); checker.check(true);
        QTRY_COMPARE_WITH_TIMEOUT(done.size(), 1, 3000);
        QTest::qWait(2700); QCOMPARE(server.requests, 1);
    }
    void destructionInFlight() {
        Server server; server.hang = true;
        auto checker = std::make_unique<UpdateChecker>("0.1.0-alpha.1", nullptr, server.endpoint());
        checker->startAutomaticCheck(); checker->check(false);
        QTRY_COMPARE(server.requests, 1);
        checker.reset(); QTest::qWait(2700); QCOMPARE(server.requests, 1);
    }
};

// Diagnostic against the real GitHub API; never part of CTest:
//   veyra_update_checker_tests --live [--prerelease] [--current <version>]
static int runLive(QCoreApplication& app) {
    const auto args = app.arguments();
    const int currentAt = args.indexOf("--current");
    if (currentAt >= 0 && currentAt + 1 >= args.size()) { std::fputs("--current needs a version\n", stderr); return 2; }
    const auto current = currentAt >= 0 ? args.at(currentAt + 1) : UpdateChecker::productVersion();
    UpdateChecker checker(current);
    checker.setPrereleaseChannel(args.contains("--prerelease"));
    QObject::connect(&checker, &UpdateChecker::completed, &app, [&](const QString& result) {
        std::printf("endpoint=%s\ncurrent=%s\nchannel=%s\nresult=%s\nlatest=%s\nurl=%s\n",
                    qPrintable(UpdateChecker::releasesEndpoint().toString()), qPrintable(current),
                    checker.prereleaseChannel() ? "prerelease" : "stable", qPrintable(result),
                    qPrintable(checker.latestVersion()), qPrintable(checker.releaseUrl().toString()));
        app.exit(result == "current" || result == "available" ? 0 : 1);
    });
    checker.check(false);
    return app.exec();
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    if (app.arguments().contains("--live")) return runLive(app);
    UpdateCheckerTests tests; return QTest::qExec(&tests, argc, argv);
}
#include "UpdateCheckerTests.moc"
