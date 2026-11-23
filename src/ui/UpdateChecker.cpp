#include "veyra/ui/UpdateChecker.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QDebug>

namespace veyra::ui {
namespace {
// One page of releases with their notes can be far larger than a single release.
constexpr qsizetype maxBody = 2 * 1024 * 1024;
const auto tagPrefix = QStringLiteral("silivid-v");
const auto releasePagePrefix = QStringLiteral("https://github.com/davimf721/Silivid/releases/tag/");

struct Candidate {
    semver::Version version;
    QString text;
    QUrl url;
};
enum class Entry { Ignored, Rejected, Eligible };

// Anything without the Silivid tag prefix, such as an inherited v2.x tag, is not
// a Silivid release. A release that claims the prefix must be fully consistent.
Entry classify(const QJsonValue& value, bool prereleaseChannel, Candidate& candidate) {
    if (!value.isObject()) return Entry::Rejected;
    const auto release = value.toObject();
    const auto tag = release.value(QStringLiteral("tag_name")).toString();
    if (!tag.startsWith(tagPrefix)) return Entry::Ignored;

    const auto draft = release.value(QStringLiteral("draft"));
    const auto prerelease = release.value(QStringLiteral("prerelease"));
    if (!draft.isBool() || !prerelease.isBool()) return Entry::Rejected;
    if (draft.toBool()) return Entry::Ignored;

    const auto text = tag.sliced(tagPrefix.size());
    auto version = semver::parse(text.toStdString());
    if (!version || version->isPrerelease() != prerelease.toBool()) return Entry::Rejected;
    if (version->isPrerelease() && !prereleaseChannel) return Entry::Ignored;

    const auto page = release.value(QStringLiteral("html_url")).toString();
    if (page != releasePagePrefix + tag) return Entry::Rejected;
    candidate = {std::move(*version), text, QUrl(page, QUrl::StrictMode)};
    return candidate.url.isValid() ? Entry::Eligible : Entry::Rejected;
}
}

QString UpdateChecker::productVersion() { return QStringLiteral(VEYRA_SILIVID_VERSION); }
QUrl UpdateChecker::releasesEndpoint() {
    // /releases/latest never returns a pre-release and is 404 until a stable one exists.
    return QUrl(QStringLiteral("https://api.github.com/repos/davimf721/Silivid/releases?per_page=30"));
}

UpdateChecker::UpdateChecker(QString currentVersion, QObject* parent, QUrl endpoint, int timeoutMs)
    : QObject(parent), currentVersion_(std::move(currentVersion)),
      current_(semver::parse(currentVersion_.toStdString())), endpoint_(std::move(endpoint)), timeoutMs_(timeoutMs),
      portable_(currentVersion_) {
    deadline_.setSingleShot(true);
    connect(&deadline_, &QTimer::timeout, this, [this] {
        if (reply_) { failure_ = QStringLiteral("timeout"); reply_->abort(); }
    });
}
UpdateChecker::~UpdateChecker() {
    deadline_.stop();
    if (reply_) {
        reply_->disconnect(this);
        reply_->abort();
    }
}
void UpdateChecker::setPrereleaseChannel(bool enabled) {
    if (prereleaseChannel_ == enabled) return;
    prereleaseChannel_ = enabled;
    latestVersion_.clear(); releaseUrl_.clear();
    emit channelChanged();
    emit releaseChanged();
}
void UpdateChecker::startAutomaticCheck() {
    if (automaticScheduled_ || QCoreApplication::arguments().contains(QStringLiteral("--portable-update-probe"))) return;
    automaticScheduled_ = true;
    QTimer::singleShot(2500, this, [this] {
        // A manual check made during startup already fulfils this run's check.
        if (!checking() && !manual_) check(false);
    });
}
void UpdateChecker::check(bool manual) {
    if (checking()) { manual_ = manual_ || manual; return; }
    manual_ = manual;
    body_.clear(); failure_.clear();
    QNetworkRequest request(endpoint_);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("User-Agent", "Silivid-Update-Check");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    // A redirect could leave the repository, so it is reported as an HTTP failure.
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setTransferTimeout(timeoutMs_);
    reply_ = network_.get(request);
    qInfo().noquote() << "[update-check-start]" << (manual_ ? "manual" : "automatic") << endpoint_.host();
    reply_->setReadBufferSize(maxBody + 1);
    connect(reply_, &QIODevice::readyRead, this, [this] {
        body_ += reply_->read(maxBody + 1 - body_.size());
        if (body_.size() > maxBody) { failure_ = QStringLiteral("response-too-large"); reply_->abort(); }
    });
    connect(reply_, &QNetworkReply::finished, this, &UpdateChecker::finish);
    deadline_.start(timeoutMs_);
    emit checkingChanged();
}
QString UpdateChecker::evaluate() {
    if (!current_) return QStringLiteral("invalid-local-version");
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(body_, &error);
    if (error.error != QJsonParseError::NoError || !document.isArray()) return QStringLiteral("invalid-json");

    std::optional<Candidate> best;
    bool rejected = false;
    for (const auto& value : document.array()) {
        Candidate candidate;
        const auto entry = classify(value, prereleaseChannel_, candidate);
        rejected = rejected || entry == Entry::Rejected;
        if (entry == Entry::Eligible && (!best || semver::compare(candidate.version, best->version) > 0))
            best = std::move(candidate);
    }
    if (!best && rejected) return QStringLiteral("invalid-release");
    if (best && semver::compare(best->version, *current_) > 0) {
        latestVersion_ = best->text; releaseUrl_ = best->url;
        emit releaseChanged();
        emit updateAvailable(latestVersion_);
        return QStringLiteral("available");
    }
    latestVersion_.clear(); releaseUrl_.clear(); emit releaseChanged();
    if (manual_) emit notice(tr("当前已是最新版本"), false);
    return QStringLiteral("current");
}
void UpdateChecker::finish() {
    deadline_.stop();
    auto* reply = reply_;
    if (reply->isOpen()) body_ += reply->read(maxBody + 1 - body_.size());
    if (body_.size() > maxBody) failure_ = QStringLiteral("response-too-large");
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (failure_.isEmpty() && (reply->error() != QNetworkReply::NoError || status != 200))
        failure_ = QStringLiteral("http-%1-network-%2").arg(status).arg(int(reply->error()));
    reply_ = nullptr;
    reply->deleteLater();
    QString result = failure_;
    if (failure_.isEmpty()) {
        result = evaluate();
        if (result != QLatin1String("available") && result != QLatin1String("current")) failure_ = result;
    }
    if (!failure_.isEmpty() && manual_) emit notice(tr("检查更新失败，请检查网络后重试"), true);
    body_.clear();
    qInfo().noquote() << "[update-check]" << result;
    emit checkingChanged();
    emit completed(result);
}
void UpdateChecker::openRelease() {
    if (!releaseUrl_.isEmpty()) QDesktopServices::openUrl(releaseUrl_);
}
}
