#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>
#include <optional>

namespace veyra::portable {

inline constexpr qint64 maxManifestBytes = 64 * 1024;
inline constexpr qint64 maxArchiveBytes = 3ll << 30;
inline constexpr qint64 maxExpandedBytes = 8ll << 30;
inline constexpr auto manifestAsset = "Veyra-portable-update-v1.json";

struct UpdatePackage {
    QString version;
    QString flavor;
    QString asset;
    QString notes;
    QByteArray digest;
    qint64 archiveBytes = 0;
    qint64 expandedBytes = 0;
};

QByteArray updatePublicKey();
QString releaseVersion(const QString& displayVersion);
// Silivid has no third-party mirrors; a saved mirror preference is no longer valid.
enum class DownloadRoute { GitHub };
std::optional<DownloadRoute> parseDownloadRoute(const QString& name);
// Always empty in Silivid: the portable download flow is disabled.
QUrl assetUrl(const QString& version, const QString& asset, DownloadRoute route = DownloadRoute::GitHub);
bool allowedRedirect(const QUrl& url);
QString packageAssetName(const QString& version, const QString& flavor);
std::optional<UpdatePackage> readUpdateManifest(
    const QByteArray& envelope, const QString& flavor, const QString& installed,
    const QString& selectedVersion, const QByteArray& publicKey);

} // namespace veyra::portable
