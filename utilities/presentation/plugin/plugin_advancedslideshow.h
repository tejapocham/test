/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2003-01-31
 * Description : a presentation tool.
 *
 * Copyright (C) 2006-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2012-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PLUGIN_ADVANCEDSLIDESHOW_H
#define PLUGIN_ADVANCEDSLIDESHOW_H

// KDE includes

#include <QUrl>



#include <KIPI/Plugin>

class QAction;

namespace KIPI
{
    class Interface;
}

namespace Digikam
{

class PresentationContainer;

class Plugin_AdvancedSlideshow : public KIPI::Plugin
{
    Q_OBJECT

public:

    Plugin_AdvancedSlideshow(QObject* const parent, const QVariantList& args);
    ~Plugin_AdvancedSlideshow();

    void setup(QWidget* const);

public Q_SLOTS:

    void slotActivate();

private Q_SLOTS:

    void slotAlbumChanged(bool anyAlbum);
    void slotSlideShow();

private:

    void setupActions();

private:

    QAction *         m_actionSlideShow;
    KIPI::Interface* m_interface;
    QList<QUrl>       m_urlList;
    PresentationContainer* m_sharedData;
};

}  // namespace Digikam

#endif  // PLUGIN_ADVANCEDSLIDESHOW_H
