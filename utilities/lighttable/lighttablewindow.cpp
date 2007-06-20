/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <q3dockarea.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3PtrList>
#include <Q3Frame>
#include <Q3VBoxLayout>
#include <QCloseEvent>

// KDE includes.

#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kwindowsystem.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kstatusbar.h>
#include <kmenubar.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawbinary.h>
#include <kglobal.h>

// Local includes.

#include "ddebug.h"
#include "themeengine.h"
#include "dimg.h"
#include "dmetadata.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "deletedialog.h"
#include "imagewindow.h"
#include "slideshow.h"
#include "setup.h"
#include "syncjob.h"
#include "thumbnailsize.h"
#include "lighttablepreview.h"
#include "lighttablewindowprivate.h"
#include "lighttablewindow.h"
#include "lighttablewindow.moc"

namespace Digikam
{

LightTableWindow* LightTableWindow::m_componentData = 0;

LightTableWindow* LightTableWindow::lightTableWindow()
{
    if (!m_componentData)
        new LightTableWindow();

    return m_componentData;
}

bool LightTableWindow::lightTableWindowCreated()
{
    return m_componentData;
}

LightTableWindow::LightTableWindow()
                : KMainWindow(0, "lighttable", Qt::WType_TopLevel)
{
    d = new LightTableWindowPriv;
    m_componentData = this;

    setCaption(i18n("Light Table"));

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();
    setupAccelerators();

    // Make signals/slots connections

    setupConnections();

    //-------------------------------------------------------------

    d->leftSidebar->loadViewState();
    d->rightSidebar->loadViewState();
    d->leftSidebar->populateTags();
    d->rightSidebar->populateTags();

    readSettings();
    applySettings();
    setAutoSaveSettings("LightTable Settings");
}

LightTableWindow::~LightTableWindow()
{
    m_componentData = 0;

    delete d->barView;
    delete d->rightSidebar;
    delete d->leftSidebar;
    delete d;
}

void LightTableWindow::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    config->setGroup("LightTable Settings");

    if(config->hasKey("Vertical Splitter Sizes"))
        d->vSplitter->setSizes(config->readIntListEntry("Vertical Splitter Sizes"));

    if(config->hasKey("Horizontal Splitter Sizes"))
        d->hSplitter->setSizes(config->readIntListEntry("Horizontal Splitter Sizes"));

    d->navigateByPairAction->setChecked(config->readBoolEntry("Navigate By Pair", false));
    slotToggleNavigateByPair();
}

void LightTableWindow::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    config->setGroup("LightTable Settings");
    config->writeEntry("Vertical Splitter Sizes", d->vSplitter->sizes());
    config->writeEntry("Horizontal Splitter Sizes", d->hSplitter->sizes());
    config->writeEntry("Navigate By Pair", d->navigateByPairAction->isChecked());
    config->sync();
}

void LightTableWindow::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    config->setGroup("LightTable Settings");

    d->autoLoadOnRightPanel  = config->readBoolEntry("Auto Load Right Panel",   true);
    d->autoSyncPreview       = config->readBoolEntry("Auto Sync Preview",       true);
    d->fullScreenHideToolBar = config->readBoolEntry("FullScreen Hide ToolBar", false);
    d->previewView->setLoadFullImageSize(config->readBoolEntry("Load Full Image size", false));
}

void LightTableWindow::closeEvent(QCloseEvent* e)
{
    if (!e) return;

    writeSettings();

    e->accept();
}

void LightTableWindow::setupUserArea()
{
    QWidget* mainW    = new QWidget(this);
    d->hSplitter      = new QSplitter(Qt::Horizontal, mainW);
    Q3HBoxLayout *hlay = new Q3HBoxLayout(mainW);
    d->leftSidebar    = new ImagePropertiesSideBarDB(mainW, 
                            "LightTable Left Sidebar", d->hSplitter,
                            Sidebar::Left, true);

    QWidget* centralW = new QWidget(d->hSplitter);
    Q3VBoxLayout *vlay = new Q3VBoxLayout(centralW);
    d->vSplitter      = new QSplitter(Qt::Vertical, centralW);
    d->barView        = new LightTableBar(d->vSplitter, ThumbBarView::Horizontal, 
                                          AlbumSettings::componentData().getExifRotate());
    d->previewView    = new LightTableView(d->vSplitter);
    vlay->addWidget(d->vSplitter);

    d->rightSidebar   = new ImagePropertiesSideBarDB(mainW, 
                            "LightTable Right Sidebar", d->hSplitter,
                            Sidebar::Right, true);

    hlay->addWidget(d->leftSidebar);
    hlay->addWidget(d->hSplitter);
    hlay->addWidget(d->rightSidebar);

    d->hSplitter->setFrameStyle( Q3Frame::NoFrame );
    d->hSplitter->setFrameShadow( Q3Frame::Plain );
    d->hSplitter->setFrameShape( Q3Frame::NoFrame );
    d->hSplitter->setOpaqueResize(false);
    d->vSplitter->setFrameStyle( Q3Frame::NoFrame );
    d->vSplitter->setFrameShadow( Q3Frame::Plain );
    d->vSplitter->setFrameShape( Q3Frame::NoFrame );
    d->vSplitter->setOpaqueResize(false);

    setCentralWidget(mainW);
}

void LightTableWindow::setupStatusBar()
{
    d->leftZoomBar = new StatusZoomBar(statusBar());
    statusBar()->addWidget(d->leftZoomBar, 1);
    d->leftZoomBar->setEnabled(false);

    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignCenter);
    d->statusProgressBar->setMaximumHeight(fontMetrics().height()+2);    
    statusBar()->addWidget(d->statusProgressBar, 100);
 
    d->rightZoomBar = new StatusZoomBar(statusBar());
    statusBar()->addWidget(d->rightZoomBar, 1);
    d->rightZoomBar->setEnabled(false);
}

void LightTableWindow::setupConnections()
{
    connect(d->statusProgressBar, SIGNAL(signalCancelButtonPressed()),
           this, SLOT(slotProgressBarCancelButtonPressed()));

    // Thumbs bar connections ---------------------------------------

    connect(d->barView, SIGNAL(signalSetItemOnLeftPanel(ImageInfo*)),
           this, SLOT(slotSetItemOnLeftPanel(ImageInfo*)));

    connect(d->barView, SIGNAL(signalSetItemOnRightPanel(ImageInfo*)),
           this, SLOT(slotSetItemOnRightPanel(ImageInfo*)));

    connect(d->barView, SIGNAL(signalRemoveItem(ImageInfo*)),
           this, SLOT(slotRemoveItem(ImageInfo*)));

    connect(d->barView, SIGNAL(signalEditItem(ImageInfo*)),
           this, SLOT(slotEditItem(ImageInfo*)));

    connect(d->barView, SIGNAL(signalClearAll()),
           this, SLOT(slotClearItemsList()));

    connect(d->barView, SIGNAL(signalLightTableBarItemSelected(ImageInfo*)),
           this, SLOT(slotItemSelected(ImageInfo*)));

    connect(d->barView, SIGNAL(signalDroppedItems(const ImageInfoList&)),
           this, SLOT(slotThumbbarDroppedItems(const ImageInfoList&)));

    // Zoom bars connections -----------------------------------------

    connect(d->leftZoomBar, SIGNAL(signalZoomMinusClicked()),
           d->previewView, SLOT(slotDecreaseLeftZoom()));

    connect(d->leftZoomBar, SIGNAL(signalZoomPlusClicked()),
           d->previewView, SLOT(slotIncreaseLeftZoom()));

    connect(d->leftZoomBar, SIGNAL(signalZoomSliderChanged(int)),
           d->previewView, SLOT(slotLeftZoomSliderChanged(int)));

    connect(d->rightZoomBar, SIGNAL(signalZoomMinusClicked()),
           d->previewView, SLOT(slotDecreaseRightZoom()));

    connect(d->rightZoomBar, SIGNAL(signalZoomPlusClicked()),
           d->previewView, SLOT(slotIncreaseRightZoom()));

    connect(d->rightZoomBar, SIGNAL(signalZoomSliderChanged(int)),
           d->previewView, SLOT(slotRightZoomSliderChanged(int)));

    // View connections ---------------------------------------------

    connect(d->previewView, SIGNAL(signalLeftZoomFactorChanged(double)),
           this, SLOT(slotLeftZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalRightZoomFactorChanged(double)),
           this, SLOT(slotRightZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalEditItem(ImageInfo*)),
           this, SLOT(slotEditItem(ImageInfo*)));

    connect(d->previewView, SIGNAL(signalDeleteItem(ImageInfo*)),
           this, SLOT(slotDeleteItem(ImageInfo*)));

    connect(d->previewView, SIGNAL(signalSlideShow()),
           this, SLOT(slotToggleSlideShow()));

    connect(d->previewView, SIGNAL(signalLeftDroppedItems(const ImageInfoList&)),
           this, SLOT(slotLeftDroppedItems(const ImageInfoList&)));

    connect(d->previewView, SIGNAL(signalRightDroppedItems(const ImageInfoList&)),
           this, SLOT(slotRightDroppedItems(const ImageInfoList&)));
                                  
    connect(d->previewView, SIGNAL(signalToggleOnSyncPreview(bool)),
           this, SLOT(slotToggleOnSyncPreview(bool)));

    connect(d->previewView, SIGNAL(signalLeftPreviewLoaded(bool)),
            this, SLOT(slotLeftPreviewLoaded(bool)));

    connect(d->previewView, SIGNAL(signalRightPreviewLoaded(bool)),
            this, SLOT(slotRightPreviewLoaded(bool)));

    connect(d->previewView, SIGNAL(signalLeftPanelLeftButtonClicked()),
            this, SLOT(slotLeftPanelLeftButtonClicked()));

    connect(d->previewView, SIGNAL(signalRightPanelLeftButtonClicked()),
            this, SLOT(slotRightPanelLeftButtonClicked()));
}

void LightTableWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    d->backwardAction = KStandardAction::back(this, SLOT(slotBackward()),
                                    actionCollection(), "lighttable_backward");
    d->backwardAction->setEnabled(false);

    d->forwardAction = KStandardAction::forward(this, SLOT(slotForward()),
                                   actionCollection(), "lighttable_forward");
    d->forwardAction->setEnabled(false);

    d->firstAction = new KAction(i18n("&First"), "start",
                                 KStandardShortcut::shortcut( KStandardShortcut::Home),
                                 this, SLOT(slotFirst()),
                                 actionCollection(), "lighttable_first");
    d->firstAction->setEnabled(false);

    d->lastAction = new KAction(i18n("&Last"), "finish",
                                KStandardShortcut::shortcut( KStandardShortcut::End),
                                this, SLOT(slotLast()),
                                actionCollection(), "lighttable_last");
    d->lastAction->setEnabled(false);

    d->setItemLeftAction = new KAction(i18n("Show item on left panel"), "previous",
                                       CTRL+Qt::Key_L, this, SLOT(slotSetItemLeft()),
                                       actionCollection(), "lighttable_setitemleft");
    d->setItemLeftAction->setEnabled(false);

    d->setItemRightAction = new KAction(i18n("Show item on right panel"), "next",
                                       CTRL+Qt::Key_R, this, SLOT(slotSetItemRight()),
                                       actionCollection(), "lighttable_setitemright");
    d->setItemRightAction->setEnabled(false);

    d->editItemAction = new KAction(i18n("Edit"), "editimage",
                                       Qt::Key_F4, this, SLOT(slotEditItem()),
                                       actionCollection(), "lighttable_edititem");
    d->editItemAction->setEnabled(false);

    d->removeItemAction = new KAction(i18n("Remove item"), "fileclose",
                                       CTRL+Qt::Key_K, this, SLOT(slotRemoveItem()),
                                       actionCollection(), "lighttable_removeitem");
    d->removeItemAction->setEnabled(false);

    d->clearListAction = new KAction(i18n("Clear all items"), "editshred",
                                     CTRL+SHIFT+Qt::Key_K, this, SLOT(slotClearItemsList()),
                                     actionCollection(), "lighttable_clearlist");
    d->clearListAction->setEnabled(false);

    d->fileDeleteAction = new KAction(i18n("Move to Trash"), "edittrash",
                                     Qt::Key_Delete,
                                     this, SLOT(slotDeleteItem()),
                                     actionCollection(), "lighttable_filedelete");
    d->fileDeleteAction->setEnabled(false);

    KStandardAction::close(this, SLOT(close()), actionCollection(), "lighttable_close");

    // -- Standard 'View' menu actions ---------------------------------------------

    d->syncPreviewAction = new KToggleAction(i18n("Synchronize Preview"), "goto",
                                            CTRL+SHIFT+Qt::Key_Y, this,
                                            SLOT(slotToggleSyncPreview()),
                                            actionCollection(), "lighttable_syncpreview");
    d->syncPreviewAction->setEnabled(false);

    d->navigateByPairAction = new KToggleAction(i18n("Navigate by Pair"), "kcmsystem",
                                            CTRL+SHIFT+Qt::Key_P, this,
                                            SLOT(slotToggleNavigateByPair()),
                                            actionCollection(), "lighttable_navigatebypair");
    d->navigateByPairAction->setEnabled(false);

    d->zoomPlusAction = KStandardAction::zoomIn(d->previewView, SLOT(slotIncreaseZoom()),
                                          actionCollection(), "lighttable_zoomplus");
    d->zoomPlusAction->setEnabled(false);

    d->zoomMinusAction = KStandardAction::zoomOut(d->previewView, SLOT(slotDecreaseZoom()),
                                             actionCollection(), "lighttable_zoomminus");
    d->zoomMinusAction->setEnabled(false);

    d->zoomTo100percents = new KAction(i18n("Zoom to 1:1"), "viewmag1",
                                       ALT+CTRL+Qt::Key_0,      // NOTE: Photoshop 7 use ALT+CTRL+0.
                                       this, SLOT(slotZoomTo100Percents()),
                                       actionCollection(), "lighttable_zoomto100percents");

    d->zoomFitToWindowAction = new KAction(i18n("Fit to &Window"), "view_fit_window",
                                           CTRL+SHIFT+Qt::Key_E, this, SLOT(slotFitToWindow()),
                                           actionCollection(), "lighttable_zoomfit2window");

    d->fullScreenAction = KStandardAction::fullScreen(this, SLOT(slotToggleFullScreen()),
                                                 actionCollection(), this, "lighttable_fullscreen");
    d->slideShowAction = new KAction(i18n("Slide Show"), "slideshow", Qt::Key_F9,
                                     this, SLOT(slotToggleSlideShow()),
                                     actionCollection(),"lighttable_slideshow");

    // -- Standard 'Configure' menu actions ----------------------------------------

    KStandardAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStandardAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStandardAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // -- Standard 'Help' menu actions ---------------------------------------------


    d->donateMoneyAction = new KAction(i18n("Donate Money..."),
                                       0, 0, 
                                       this, SLOT(slotDonateMoney()),
                                       actionCollection(),
                                       "lighttable_donatemoney");    

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Rating actions ---------------------------------------------------------------

    d->star0 = new KAction(i18n("Assign Rating \"No Star\""), CTRL+Qt::Key_0,
                          d->barView, SLOT(slotAssignRatingNoStar()),
                          actionCollection(), "lighttable_ratenostar");
    d->star1 = new KAction(i18n("Assign Rating \"One Star\""), CTRL+Qt::Key_1,
                          d->barView, SLOT(slotAssignRatingOneStar()),
                          actionCollection(), "lighttable_rateonestar");
    d->star2 = new KAction(i18n("Assign Rating \"Two Stars\""), CTRL+Qt::Key_2,
                          d->barView, SLOT(slotAssignRatingTwoStar()),
                          actionCollection(), "lighttable_ratetwostar");
    d->star3 = new KAction(i18n("Assign Rating \"Three Stars\""), CTRL+Qt::Key_3,
                          d->barView, SLOT(slotAssignRatingThreeStar()),
                          actionCollection(), "lighttable_ratethreestar");
    d->star4 = new KAction(i18n("Assign Rating \"Four Stars\""), CTRL+Qt::Key_4,
                          d->barView, SLOT(slotAssignRatingFourStar()),
                          actionCollection(), "lighttable_ratefourstar");
    d->star5 = new KAction(i18n("Assign Rating \"Five Stars\""), CTRL+Qt::Key_5,
                          d->barView, SLOT(slotAssignRatingFiveStar()),
                          actionCollection(), "lighttable_ratefivestar");

    // ---------------------------------------------------------------------------------

    createGUI("lighttablewindowui.rc", false);
}

void LightTableWindow::setupAccelerators()
{
    d->accelerators = new KAccel(this);

    d->accelerators->insert("Exit fullscreen", i18n("Exit Fullscreen mode"),
                    i18n("Exit from fullscreen viewing mode"),
                    Qt::Key_Escape, this, SLOT(slotEscapePressed()),
                    false, true);

    d->accelerators->insert("Next Image Qt::Key_Space", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Qt::Key_Space, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Previous Image Qt::Key_Backspace", i18n("Previous Image"),
                    i18n("Load Previous Image"),
                    Qt::Key_Backspace, this, SLOT(slotBackward()),
                    false, true);

    d->accelerators->insert("Next Image Qt::Key_Next", i18n("Next Image"),
                    i18n("Load Next Image"),
                    Qt::Key_Next, this, SLOT(slotForward()),
                    false, true);

    d->accelerators->insert("Previous Image Qt::Key_Prior", i18n("Previous Image"),
                    i18n("Load Previous Image"),
                    Qt::Key_Prior, this, SLOT(slotBackward()),
                    false, true);

    d->accelerators->insert("Zoom Plus Qt::Key_Plus", i18n("Zoom in"),
                    i18n("Zoom in on image"),
                    Qt::Key_Plus, d->previewView, SLOT(slotIncreaseZoom()),
                    false, true);
    
    d->accelerators->insert("Zoom Plus Qt::Key_Minus", i18n("Zoom out"),
                    i18n("Zoom out of image"),
                    Qt::Key_Minus, d->previewView, SLOT(slotDecreaseZoom()),
                    false, true);
}

void LightTableWindow::slotThumbbarDroppedItems(const ImageInfoList& list)
{
    loadImageInfos(list, 0);
}

void LightTableWindow::loadImageInfos(const ImageInfoList &list, ImageInfo *imageInfoCurrent)
{
    ImageInfoList l = list;

    if (!imageInfoCurrent) 
        imageInfoCurrent = l.first();

    AlbumSettings *settings = AlbumSettings::componentData();

    if (!settings) return;

    QString imagefilter = settings->getImageFileFilter().toLower() +
                          settings->getImageFileFilter().toUpper();

    if (KDcrawIface::DcrawBinary::componentData().versionIsRight())
    {
        // add raw files only if dcraw is available
        imagefilter += settings->getRawFileFilter().toLower() +
                       settings->getRawFileFilter().toUpper();
    }

    d->barView->blockSignals(true);
    for (Q3PtrList<ImageInfo>::const_iterator it = l.begin(); it != l.end(); ++it)
    {
        QString fileExtension = (*it)->kurl().fileName().section( '.', -1 );

        if ( imagefilter.find(fileExtension) != -1 && 
             !d->barView->findItemByInfo(*it) )
        {
            new LightTableBarItem(d->barView, *it);
        }
    }   
    d->barView->blockSignals(false);

    LightTableBarItem *ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(imageInfoCurrent));
    if (ltItem) 
        d->barView->setSelectedItem(ltItem);

    // if window is iconified, show it
    if (isMinimized())
    {
        KWindowSystem::deIconifyWindow(winId());
    }

    refreshStatusBar();
}   

void LightTableWindow::refreshStatusBar()
{
    switch (d->barView->countItems())
    {
        case 0:
            d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, 
                                                  i18n("No item on Light Table"));   
            break;
        case 1:
            d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, 
                                                  i18n("1 item on Light Table"));   
            break;
        default:
            d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, 
                                                  i18n("%1 items on Light Table")
                                                  .arg(d->barView->countItems()));   
            break;
    }  
}

void LightTableWindow::slotItemsUpdated(const KUrl::List& urls)
{
    d->barView->refreshThumbs(urls);

    for (KUrl::List::const_iterator it = urls.begin() ; it != urls.end() ; ++it)
    {
        if (d->previewView->leftImageInfo())
        {
            if (d->previewView->leftImageInfo()->kurl() == *it)
            {
                d->previewView->leftReload();
                d->leftSidebar->itemChanged(d->previewView->leftImageInfo());
            }
        }
    
        if (d->previewView->rightImageInfo())
        {
            if (d->previewView->rightImageInfo()->kurl() == *it)
            {
                d->previewView->rightReload();
                d->rightSidebar->itemChanged(d->previewView->rightImageInfo());
            }
        }
    }
}

void LightTableWindow::slotLeftPanelLeftButtonClicked()
{
    if (d->navigateByPairAction->isChecked()) return;

    d->barView->setSelectedItem(d->barView->findItemByInfo(d->previewView->leftImageInfo()));
}

void LightTableWindow::slotRightPanelLeftButtonClicked()
{
    // With navigate by pair option, only the Feft panel can be selected.
    if (d->navigateByPairAction->isChecked()) return;

    d->barView->setSelectedItem(d->barView->findItemByInfo(d->previewView->rightImageInfo()));
}

void LightTableWindow::slotLeftPreviewLoaded(bool b)
{
    d->leftZoomBar->setEnabled(b);

    if (b)
    {
        d->previewView->checkForSelection(d->barView->currentItemImageInfo());
        d->barView->setOnLeftPanel(d->previewView->leftImageInfo());
    
        LightTableBarItem *item = d->barView->findItemByInfo(d->previewView->leftImageInfo());
        if (item) item->setOnLeftPanel(true);
    
        if (d->navigateByPairAction->isChecked() && item)
        {
            LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(item->next());
            if (next)
            {
                d->barView->setOnRightPanel(next->info());
                slotSetItemOnRightPanel(next->info());
            }
            else
            {
                LightTableBarItem* first = dynamic_cast<LightTableBarItem*>(d->barView->firstItem());
                slotSetItemOnRightPanel(first ? first->info() : 0);
            }
        }
    }
}

void LightTableWindow::slotRightPreviewLoaded(bool b)
{
    d->rightZoomBar->setEnabled(b);
    if (b)
    {
        d->previewView->checkForSelection(d->barView->currentItemImageInfo());
        d->barView->setOnRightPanel(d->previewView->rightImageInfo());
    
        LightTableBarItem *item = d->barView->findItemByInfo(d->previewView->rightImageInfo());
        if (item) item->setOnRightPanel(true);
    }
}

void LightTableWindow::slotItemSelected(ImageInfo* info)
{
    if (info)
    {
        d->setItemLeftAction->setEnabled(true);
        d->setItemRightAction->setEnabled(true);
        d->editItemAction->setEnabled(true);
        d->removeItemAction->setEnabled(true);
        d->clearListAction->setEnabled(true);
        d->fileDeleteAction->setEnabled(true);
        d->backwardAction->setEnabled(true);
        d->forwardAction->setEnabled(true);
        d->firstAction->setEnabled(true);
        d->lastAction->setEnabled(true);
        d->syncPreviewAction->setEnabled(true);
        d->zoomPlusAction->setEnabled(true);
        d->zoomMinusAction->setEnabled(true);
        d->navigateByPairAction->setEnabled(true);

        LightTableBarItem* curr = d->barView->findItemByInfo(info);
        if (curr)
        {
            if (!curr->prev())
            {
                d->backwardAction->setEnabled(false);
                d->firstAction->setEnabled(false);
            }
    
            if (!curr->next())
            {
                d->forwardAction->setEnabled(false);
                d->lastAction->setEnabled(false);
            }

            if (d->navigateByPairAction->isChecked())
            {
                d->setItemLeftAction->setEnabled(false);
                d->setItemRightAction->setEnabled(false);
  
                d->barView->setOnLeftPanel(info);
                slotSetItemOnLeftPanel(info);
            }
            else if (d->autoLoadOnRightPanel && !curr->isOnLeftPanel()) 
            {
                d->barView->setOnRightPanel(info);
                slotSetItemOnRightPanel(info);
            }
        }
    }
    else
    {
        d->setItemLeftAction->setEnabled(false);
        d->setItemRightAction->setEnabled(false);
        d->editItemAction->setEnabled(false);
        d->removeItemAction->setEnabled(false);
        d->clearListAction->setEnabled(false);
        d->fileDeleteAction->setEnabled(false);
        d->backwardAction->setEnabled(false);
        d->forwardAction->setEnabled(false);
        d->firstAction->setEnabled(false);
        d->lastAction->setEnabled(false);
        d->zoomPlusAction->setEnabled(false);
        d->zoomMinusAction->setEnabled(false);
        d->syncPreviewAction->setEnabled(false);
        d->navigateByPairAction->setEnabled(false);
    }

    d->previewView->checkForSelection(info);
}    

void LightTableWindow::slotLeftDroppedItems(const ImageInfoList& list)
{
    ImageInfo *info = *(list.begin());
    loadImageInfos(list, info);

    // We will check if first item from list is already stored in thumbbar
    // Note than thumbbar store all ImageInfo reference in memory for preview object.
    LightTableBarItem *item = d->barView->findItemByInfo(info);
    if (item)
        slotSetItemOnLeftPanel(item->info());
}

void LightTableWindow::slotRightDroppedItems(const ImageInfoList& list)
{
    ImageInfo *info = *(list.begin());
    loadImageInfos(list, info);

    // We will check if first item from list is already stored in thumbbar
    // Note than thumbbar store all ImageInfo reference in memory for preview object.
    LightTableBarItem *item = d->barView->findItemByInfo(info);
    if (item)
        slotSetItemOnRightPanel(item->info());
}

void LightTableWindow::slotSetItemLeft()
{
    if (d->barView->currentItemImageInfo())
    {
        slotSetItemOnLeftPanel(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotSetItemRight()
{
    if (d->barView->currentItemImageInfo())
    {
        slotSetItemOnRightPanel(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotSetItemOnLeftPanel(ImageInfo* info)
{
    d->previewView->setLeftImageInfo(info);
    if (info)
        d->leftSidebar->itemChanged(info);
    else
        d->leftSidebar->slotNoCurrentItem();
}

void LightTableWindow::slotSetItemOnRightPanel(ImageInfo* info)
{
    d->previewView->setRightImageInfo(info);
    if (info)
        d->rightSidebar->itemChanged(info);
    else
        d->rightSidebar->slotNoCurrentItem();
}

void LightTableWindow::slotClearItemsList()
{
    if (d->previewView->leftImageInfo())
    {
        d->previewView->setLeftImageInfo();
        d->leftSidebar->slotNoCurrentItem();
    }

    if (d->previewView->rightImageInfo())
    {
        d->previewView->setRightImageInfo();
        d->rightSidebar->slotNoCurrentItem();
    }

    d->barView->clear();
    refreshStatusBar();
}

void LightTableWindow::slotDeleteItem()
{
    if (d->barView->currentItemImageInfo())
        slotDeleteItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotDeleteItem(ImageInfo* info)
{
    bool ask         = true;
    bool permanently = false;

    KUrl u = info->kurl();
    PAlbum *palbum = AlbumManager::componentData().findPAlbum(u.directory());
    if (!palbum)
        return;

    // Provide a digikamalbums:// URL to KIO
    KUrl kioURL  = info->kurlForKIO();
    KUrl fileURL = u;

    bool useTrash;

    if (ask)
    {
        bool preselectDeletePermanently = permanently;

        DeleteDialog dialog(this);

        KUrl::List urlList;
        urlList.append(u);
        if (!dialog.confirmDeleteList(urlList,
             DeleteDialogMode::Files,
             preselectDeletePermanently ?
                     DeleteDialogMode::NoChoiceDeletePermanently : DeleteDialogMode::NoChoiceTrash))
            return;

        useTrash = !dialog.shouldDelete();
    }
    else
    {
        useTrash = !permanently;
    }

    // trash does not like non-local URLs, put is not implemented
    if (useTrash)
        kioURL = fileURL;

    if (!SyncJob::del(kioURL, useTrash))
    {
        QString errMsg(SyncJob::lastErrorMsg());
        KMessageBox::error(this, errMsg, errMsg);
        return;
    }

    emit signalFileDeleted(u);

    slotRemoveItem(info);
}

void LightTableWindow::slotRemoveItem()
{
    if (d->barView->currentItemImageInfo())
        slotRemoveItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotRemoveItem(ImageInfo* info)
{
    if (d->previewView->leftImageInfo())
    {
        if (d->previewView->leftImageInfo()->id() == info->id())
        {
            d->previewView->setLeftImageInfo();
            d->leftSidebar->slotNoCurrentItem();
        }
    }

    if (d->previewView->rightImageInfo())
    {
        if (d->previewView->rightImageInfo()->id() == info->id())
        {
            d->previewView->setRightImageInfo();
            d->rightSidebar->slotNoCurrentItem();
        }
    }

    d->barView->removeItem(info);
    d->barView->setSelected(d->barView->currentItem());
    refreshStatusBar();
}

void LightTableWindow::slotEditItem()
{
    if (d->barView->currentItemImageInfo())
        slotEditItem(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotEditItem(ImageInfo* info)
{
    ImageWindow *im    = ImageWindow::imagewindow();
    ImageInfoList list = d->barView->itemsImageInfoList();

    im->loadImageInfos(list, info, i18n("Light Table"), true);
    
    if (im->isHidden())
        im->show();
    else
        im->raise();
        
    im->setFocus();
}

void LightTableWindow::slotZoomTo100Percents()
{
    d->previewView->setLeftZoomFactor(1.0);
    d->previewView->setRightZoomFactor(1.0);
}

void LightTableWindow::slotFitToWindow()
{
    d->previewView->fitToWindow();
}

void LightTableWindow::slotToggleSlideShow()
{
    KSharedConfig::Ptr config = KGlobal::config();
    config->setGroup("ImageViewer Settings");
    bool startWithCurrent = config->readBoolEntry("SlideShowStartCurrent", false);

    SlideShowSettings settings;
    settings.delay                = config->readNumEntry("SlideShowDelay", 5) * 1000;
    settings.printName            = config->readBoolEntry("SlideShowPrintName", true);
    settings.printDate            = config->readBoolEntry("SlideShowPrintDate", false);
    settings.printApertureFocal   = config->readBoolEntry("SlideShowPrintApertureFocal", false);
    settings.printExpoSensitivity = config->readBoolEntry("SlideShowPrintExpoSensitivity", false);
    settings.printMakeModel       = config->readBoolEntry("SlideShowPrintMakeModel", false);
    settings.printComment         = config->readBoolEntry("SlideShowPrintComment", false);
    settings.loop                 = config->readBoolEntry("SlideShowLoop", false);
    slideShow(startWithCurrent, settings);
}

void LightTableWindow::slideShow(bool startWithCurrent, SlideShowSettings& settings)
{
    int       i = 0;
    DMetadata meta;
    d->cancelSlideShow = false;

    d->statusProgressBar->progressBarMode(StatusProgressBar::CancelProgressBarMode, 
                                  i18n("Preparing slideshow. Please wait..."));

    ImageInfoList list = d->barView->itemsImageInfoList();

    for (ImageInfo *info = list.first() ; !d->cancelSlideShow && info ; info = list.next())
    {
        SlidePictureInfo pictInfo;
        pictInfo.comment = info->comment();

        // Perform optimizations: only read pictures metadata if necessary.
        if (settings.printApertureFocal || settings.printExpoSensitivity || settings.printMakeModel)
        {
            meta.load(info->kurl().path());
            pictInfo.photoInfo = meta.getPhotographInformations();
        }

        // In case of dateTime extraction from metadata failed 
        pictInfo.photoInfo.dateTime = info->dateTime(); 
        settings.pictInfoMap.insert(info->kurl(), pictInfo);
        settings.fileList.append(info->kurl());

        d->statusProgressBar->setProgressValue((int)((i++/(float)list.count())*100.0));
        kapp->processEvents();
    }

    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, QString());   
    refreshStatusBar();

    if (!d->cancelSlideShow)
    {
        settings.exifRotate = AlbumSettings::componentData().getExifRotate();
    
        SlideShow *slide = new SlideShow(settings);
        if (startWithCurrent)
            slide->setCurrent(d->barView->currentItemImageInfo()->kurl());
    
        slide->show();
    }
}

void LightTableWindow::slotProgressBarCancelButtonPressed()
{
    d->cancelSlideShow = true;
}

void LightTableWindow::slotToggleFullScreen()
{
    if (d->fullScreen) // out of fullscreen
    {

#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~Qt::WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        statusBar()->show();
        leftDock()->show();
        rightDock()->show();
        topDock()->show();
        bottomDock()->show();
        
        QObject* obj = child("ToolBar","KToolBar");
        
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            
            if (d->fullScreenAction->isPlugged(toolBar) && d->removeFullScreenButton)
                d->fullScreenAction->unplug(toolBar);
                
            if (toolBar->isHidden())
                showToolBars();
        }

        // -- remove the gui action accels ----

        unplugActionAccel(d->zoomFitToWindowAction);

        if (d->fullScreen)
        {
            d->leftSidebar->restore();
            d->rightSidebar->restore();
        }
        else       
        {
            d->leftSidebar->backup();        
            d->rightSidebar->backup();        
        }
        
        d->fullScreen = false;
    }
    else  // go to fullscreen
    {
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();
        topDock()->hide();
        leftDock()->hide();
        rightDock()->hide();
        bottomDock()->hide();
        
        QObject* obj = child("ToolBar","KToolBar");
        
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            
            if (d->fullScreenHideToolBar)
            {
                hideToolBars();
            }
            else
            {   
                showToolBars();

                if ( !d->fullScreenAction->isPlugged(toolBar) )
                {
                    d->fullScreenAction->plug(toolBar);
                    d->removeFullScreenButton=true;
                }
                else    
                {
                    // If FullScreen button is enable in toolbar settings
                    // We don't remove it when we out of fullscreen mode.
                    d->removeFullScreenButton=false;
                }
            }
        }

        // -- Insert all the gui actions into the accel --

        plugActionAccel(d->zoomFitToWindowAction);

        if (d->fullScreen) 
        {
            d->leftSidebar->restore();
            d->rightSidebar->restore();
        }
        else
        {
            d->leftSidebar->backup();        
            d->rightSidebar->backup();        
        }

        showFullScreen();
        d->fullScreen = true;
    }
}

void LightTableWindow::slotEscapePressed()
{
    if (d->fullScreen)
        d->fullScreenAction->activate();
}

void LightTableWindow::showToolBars()
{
    Q3PtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for( ; it.current()!=0L ; ++it)
    {
        bar=it.current();
        
        if (bar->area())
            bar->area()->show();
        else
            bar->show();
    }
}

void LightTableWindow::hideToolBars()
{
    Q3PtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for( ; it.current()!=0L ; ++it)
    {
        bar=it.current();
        
        if (bar->area()) 
            bar->area()->hide();
        else 
            bar->hide();
    }
}

void LightTableWindow::plugActionAccel(KAction* action)
{
    if (!action)
        return;

    d->accelerators->insert(action->text(),
                    action->text(),
                    action->whatsThis(),
                    action->shortcut(),
                    action,
                    SLOT(activate()));
}

void LightTableWindow::unplugActionAccel(KAction* action)
{
    d->accelerators->remove(action->text());
}

void LightTableWindow::slotDonateMoney()
{
    KApplication::kApplication()->invokeBrowser("http://www.digikam.org/?q=donation");
}

void LightTableWindow::slotEditKeys()
{
    KKeyDialog dialog(true, this);
    dialog.insert( actionCollection(), i18n( "General" ) );
    dialog.configure();
}

void LightTableWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config(), "LightTable Settings");
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void LightTableWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), "LightTable Settings");
}

void LightTableWindow::slotSetup()
{
    Setup setup(this, 0);    
        
    if (setup.exec() != QDialog::Accepted)
        return;

    KGlobal::config()->sync();
    
    applySettings();
}

void LightTableWindow::slotLeftZoomFactorChanged(double zoom)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->previewView->leftZoomMin();
    double zmax = d->previewView->leftZoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    int size    = (int)((zoom - b) /a); 

    d->leftZoomBar->setZoomSliderValue(size);
    d->leftZoomBar->setZoomTrackerText(i18n("zoom: %1%").arg((int)(zoom*100.0)));

    d->leftZoomBar->setEnableZoomPlus(true);
    d->leftZoomBar->setEnableZoomMinus(true);

    if (d->previewView->leftMaxZoom())
        d->leftZoomBar->setEnableZoomPlus(false);

    if (d->previewView->leftMinZoom())
        d->leftZoomBar->setEnableZoomMinus(false);
}

void LightTableWindow::slotRightZoomFactorChanged(double zoom)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->previewView->rightZoomMin();
    double zmax = d->previewView->rightZoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    int size    = (int)((zoom - b) /a); 

    d->rightZoomBar->setZoomSliderValue(size);
    d->rightZoomBar->setZoomTrackerText(i18n("zoom: %1%").arg((int)(zoom*100.0)));

    d->rightZoomBar->setEnableZoomPlus(true);
    d->rightZoomBar->setEnableZoomMinus(true);

    if (d->previewView->rightMaxZoom())
        d->rightZoomBar->setEnableZoomPlus(false);

    if (d->previewView->rightMinZoom())
        d->rightZoomBar->setEnableZoomMinus(false);
}

void LightTableWindow::slotToggleSyncPreview()
{
    d->previewView->setSyncPreview(d->syncPreviewAction->isChecked());    
}

void LightTableWindow::slotToggleOnSyncPreview(bool t)
{
    d->syncPreviewAction->setEnabled(t);

    if (!t)
    {
        d->syncPreviewAction->setChecked(false);
    }
    else
    {
        if (d->autoSyncPreview)
            d->syncPreviewAction->setChecked(true);
    }
}

void LightTableWindow::slotBackward()
{
    ThumbBarItem* curr = d->barView->currentItem();
    if (curr && curr->prev())
        d->barView->setSelected(curr->prev());
}

void LightTableWindow::slotForward()
{
    ThumbBarItem* curr = d->barView->currentItem();
    if (curr && curr->next())
        d->barView->setSelected(curr->next());
}

void LightTableWindow::slotFirst()
{
    d->barView->setSelected( d->barView->firstItem() );
}

void LightTableWindow::slotLast()
{
    d->barView->setSelected( d->barView->lastItem() );
}

void LightTableWindow::slotToggleNavigateByPair()
{
    d->barView->setNavigateByPair(d->navigateByPairAction->isChecked());
    d->previewView->setNavigateByPair(d->navigateByPairAction->isChecked());
    slotItemSelected(d->barView->currentItemImageInfo());
}

}  // namespace Digikam

