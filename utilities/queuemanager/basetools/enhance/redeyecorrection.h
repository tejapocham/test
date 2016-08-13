/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : TBD
 * Description : A Red-Eye tool for automatic detection and correction filter.
 *
 * Copyright (C) 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#ifndef REDEYECORRECTION_H
#define REDEYECORRECTION_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class RedEyeCorrection : public BatchTool
{
    Q_OBJECT

public:

    explicit RedEyeCorrection(QObject* const parent = 0);
    ~RedEyeCorrection();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new RedEyeCorrection(parent); };

    void registerSettingsWidget();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

};

}  // namespace Digikam

#endif /* REDEYECORRECTION_H */