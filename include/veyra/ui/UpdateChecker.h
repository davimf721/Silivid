#pragma once
#include <QObject>
#include <QByteArray>
#include <QUrl>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "veyra/base/SemVer.h"
#include "veyra/ui/PortableUpdate.h"
#include <optional>

namespace veyra::ui {
// UI-only, bounded asynchronous metadata request; never touches the engine.
// Silivid: reads davimf721/Silivid releases, notifies and opens the release page.
class UpdateChecker final : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool checking READ checking NOTIFY checkingChanged)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY releaseChanged)
    Q_PROPERTY(QUrl releaseUrl READ releaseUrl NOTIFY releaseChanged)
    Q_PROPERTY(bool prereleaseChannel READ prereleaseChannel WRITE setPrereleaseChannel NOTIFY channelChanged)
    Q_PROPERTY(veyra::ui::PortableUpdate* portable READ portableUpdate CONSTANT)
public:
    static QString productVersion();
    static QUrl releasesEndpoint();

    explicit UpdateChecker(QString currentVersion, QObject* parent = nullptr,
        QUrl endpoint = releasesEndpoint(),
        int timeoutMs = 12000);
    ~UpdateChecker() override;
    bool checking() const { return reply_ != nullptr; }
    QString latestVersion() const { return latestVersion_; }
    QUrl releaseUrl() const { return releaseUrl_; }
    // Off by default: the stable channel ignores every pre-release.
    bool prereleaseChannel() const { return prereleaseChannel_; }
    void setPrereleaseChannel(bool enabled);
    PortableUpdate* portableUpdate() { return &portable_; }
    // Kept for the preference code; GitHub is the only route.
    void setDownloadRoute(veyra::portable::DownloadRoute) {}
    Q_INVOKABLE void startAutomaticCheck();
    Q_INVOKABLE void check(bool manual = true);
    Q_INVOKABLE void openRelease();
signals:
    void checkingChanged();
    void releaseChanged();
    void channelChanged();
    void updateAvailable(const QString& version);
    void notice(const QString& message, bool isError);
    // Terminal diagnostic result (also used by deterministic/real HTTP tests).
    void completed(const QString& result);
private:
    void finish();
    QString evaluate();
    QNetworkAccessManager network_;
    QTimer deadline_;
    QNetworkReply* reply_ = nullptr;
    QString currentVersion_, latestVersion_;
    std::optional<veyra::semver::Version> current_;
    QUrl endpoint_, releaseUrl_;
    QByteArray body_;
    QString failure_;
    int timeoutMs_;
    bool automaticScheduled_ = false, manual_ = false, prereleaseChannel_ = false;
    PortableUpdate portable_;
};
}
