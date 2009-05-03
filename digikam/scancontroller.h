/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-28
 * Description : scan pictures interface.
 *
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

// Qt includes

#include <QThread>
#include <QString>

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"
#include "collectionscannerobserver.h"

namespace Digikam
{

class CollectionScanner;
class SplashScreen;
class PAlbum;
class ScanControllerPriv;

class ScanController : public QThread, public InitializationObserver
{
    Q_OBJECT

public:

    static ScanController *instance();
    /** Wait for the thread to finish. Returns after all tasks are done. */
    void shutDown();

    enum Advice
    {
        Success,
        ContinueWithoutDatabase,
        AbortImmediately
    };

    /**
     * Calls DatabaseAccess::checkReadyForUse(), providing progress
     * feedback if schema updating occurs.
     * Synchronous, returns when ready.
     */
    Advice databaseInitialization();

    /**
     * Carries out a complete collection scan, providing progress feedback.
     * Synchronous, returns when ready.
     * The database will be locked while the scan is running.
     */
    void completeCollectionScan(SplashScreen *splash=0);

    /**
     * Schedules a scan of the specified part of the collection.
     * Asynchronous, returns immediately.
     */
    void scheduleCollectionScan(const QString &path);

    /**
     * Schedules a scan of the specified part of the collection.
     * Asynchronous, returns immediately.
     * A small delay may be introduced before the actual scanning starts,
     * so that you can call this often without checking for duplicates.
     * This method must only be used from the main thread.
     */
    void scheduleCollectionScanRelaxed(const QString &path);

    /**
     * The file pointed to by file path will be scanned.
     * The scan is finished when returning from the method.
     */
    void scanFileDirectly(const QString &filePath);

    /** If the controller is currently processing a database update
     *  (typically after first run),
     *  cancel this hard and as soon as possible. Any progress may be lost. */
    void abortInitialization();

    /** If the controller is currently doing a complete scan
     *  (typically at startup), stop this operation.
     *  It can be resumed later. */
    void cancelCompleteScan();

    /** Temporarily suspend collection scanning.
     *  All scheduled scanning tasks are queued
     *  and will be done when resumeCollectionScan()
     *  has been called.
     *  Calling these methods is recursive, you must resume
     *  as often as you called suspend.
     */
    void suspendCollectionScan();
    void resumeCollectionScan();

    /** Hint at the imminent copy, move or rename of an album, so that the
     *  collection scanner is informed about this.
     *  If the album is renamed, give the new name in newAlbumName.
     *  DstAlbum is the new parent album /
     *  dstPath is the new parent directory of the album, so
     *  do not include the album name to dstPath.
     */
    void hintAtMoveOrCopyOfAlbum(const PAlbum *album, const PAlbum *dstAlbum, const QString &newAlbumName = QString());
    void hintAtMoveOrCopyOfAlbum(const PAlbum *album, const QString &dstPath, const QString &newAlbumName = QString());

    /** Hint at the imminent copy, move or rename of items, so that the
     *  collection scanner is informed about this.
     *  Give the list of existing items, specify the destination with dstAlbum,
     *  and give the names at destination in itemNames (Unless for rename, names wont usually change.
     *  Give them nevertheless.)
     */
    void hintAtMoveOrCopyOfItems(const QList<qlonglong> ids, const PAlbum *dstAlbum, QStringList itemNames);
    void hintAtMoveOrCopyOfItem(qlonglong id, const PAlbum *dstAlbum, QString itemName);

Q_SIGNALS:

    void databaseInitialized(bool success);
    void completeScanDone();
    void triggerShowProgressDialog();
    void incrementProgressDialog(int);
    void errorFromInitialization(const QString &);
    void progressFromInitialization(const QString &, int);

private Q_SLOTS:

    void slotTotalFilesToScan(int count);
    void slotStartScanningAlbum(const QString &albumRoot, const QString &album);
    void slotFinishedScanningAlbum(const QString &, const QString &, int filesScanned);
    void slotStartScanningAlbumRoot(const QString &albumRoot);
    void slotStartScanningForStaleAlbums();
    void slotStartScanningAlbumRoots();

    void slotShowProgressDialog();
    void slotTriggerShowProgressDialog();
    void slotCancelPressed();

    void slotProgressFromInitialization(const QString &message, int numberOfSteps);
    void slotErrorFromInitialization(const QString &errorMessage);

    void slotRelaxedScanning();

protected:

    virtual void run();

private:

    virtual void moreSchemaUpdateSteps(int numberOfSteps);
    virtual void schemaUpdateProgress(const QString &message, int numberOfSteps);
    virtual void finishedSchemaUpdate(UpdateResult result);
    virtual void connectCollectionScanner(CollectionScanner *scanner);
    virtual void error(const QString &errorMessage);
    virtual bool continueQuery();

    void createProgressDialog();

private:

    friend class ScanControllerCreator;
    ScanController();
    ~ScanController();

    ScanControllerPriv* const d;
};

}  // namespace Digikam

#endif /* SCANCONTROLLER_H */
