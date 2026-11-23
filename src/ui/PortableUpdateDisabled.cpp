// Silivid only notifies and opens the release page. This replaces PortableUpdate.cpp
// in the build, so no download, signature, scheduling or install code is linked.
// The object keeps its QML surface and permanently reports "nothing available".
#include "veyra/ui/PortableUpdate.h"

#include <QDebug>

namespace veyra::ui {

PortableUpdate::PortableUpdate(const QString&, QObject* parent) : QObject(parent) {}
PortableUpdate::~PortableUpdate() = default;

int PortableUpdate::progress() const { return 0; }
QString PortableUpdate::sizeText() const { return {}; }
QString PortableUpdate::releaseNotes() const { return {}; }
void PortableUpdate::selectRelease(const QString&) {}
void PortableUpdate::downloadAndSchedule() {
    qWarning().noquote() << "[portable-update] refused: automatic update is disabled in Silivid";
}
void PortableUpdate::cancel() {}

} // namespace veyra::ui
