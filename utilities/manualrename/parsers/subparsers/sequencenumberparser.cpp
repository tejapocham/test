/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a sequence number parser class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "sequencenumberparser.h"
#include "sequencenumberparser.moc"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

namespace Digikam
{

class SequenceNumberDialogPriv
{
public:

    SequenceNumberDialogPriv()
    {
        digits = 0;
        start  = 0;
        step   = 0;
    }

    KIntNumInput* digits;
    KIntNumInput* start;
    KIntNumInput* step;
};

// --------------------------------------------------------

SequenceNumberDialog::SequenceNumberDialog()
                    : KDialog(0), d(new SequenceNumberDialogPriv)
{
    setCaption(i18n("Add sequence number"));

    d->digits = new KIntNumInput;
    d->start  = new KIntNumInput;
    d->step   = new KIntNumInput;

    QLabel* digitsLabel = new QLabel(i18nc("number of digits", "Digits:"));
    QLabel* startLabel  = new QLabel(i18nc("start of sequence number range", "Start:"));
    QLabel* stepLabel   = new QLabel(i18nc("stepping used for sequence number range", "Step:"));

    d->digits->setRange(1, 999999, 1);
    d->digits->setSliderEnabled(false);

    d->start->setRange(1, 999999, 1);
    d->start->setSliderEnabled(false);

    d->step->setRange(1, 999999, 1);
    d->step->setSliderEnabled(false);

    QGroupBox* gbox         = new QGroupBox(i18n("Custom Range"));
    QGridLayout* gboxLayout = new QGridLayout;
    gboxLayout->addWidget(startLabel, 0, 0);
    gboxLayout->addWidget(d->start,   0, 1);
    gboxLayout->addWidget(stepLabel,  1, 0);
    gboxLayout->addWidget(d->step,    1, 1);
    gboxLayout->setRowStretch(2, 10);
    gbox->setLayout(gboxLayout);

    QWidget* w              = new QWidget;
    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(digitsLabel, 0, 0, 1, 1);
    mainLayout->addWidget(d->digits,   0, 1, 1, 1);
    mainLayout->addWidget(gbox,        1, 0, 1,-1);
    w->setLayout(mainLayout);

    setMainWidget(w);
}

SequenceNumberDialog::~SequenceNumberDialog()
{
}

int SequenceNumberDialog::digits() const
{
    return d->digits->value();
}

int SequenceNumberDialog::start()  const
{
    return d->start->value();
}

int SequenceNumberDialog::step()   const
{
    return d->step->value();
}

// --------------------------------------------------------

SequenceNumberParser::SequenceNumberParser()
                    : SubParser(i18n("Sequence Number..."), SmallIcon("accessories-calculator"))
{
    setUseTokenMenu(false);
    setUseModifiers(false);

    addToken("#", i18n("Sequence Number"),
             i18n("sequence number"));

    addToken("#{start}", i18n("Sequence Number (start)"),
             i18n("sequence number (custom start)"));

    addToken("#{start,step}", i18n("Sequence Number (start, step)"), i18n(
            "sequence number (custom start + step)"));

}

void SequenceNumberParser::slotTokenTriggered(const QString& token)
{
    Q_UNUSED(token)

    SequenceNumberDialog* dlg = new SequenceNumberDialog;

    QString tmp;
    if (dlg->exec() == KDialog::Accepted)
    {
        int _digits = dlg->digits();
        int _start  = dlg->start();
        int _step   = dlg->step();

        tmp = QString("%1").arg("#", _digits, QChar('#'));
        if (_start > 1)
        {
            tmp.append(QString("{%1").arg(QString::number(_start)));

            if (_step > 1)
            {
                tmp.append(QString(",%1").arg(QString::number(_step)));
            }

            tmp.append(QChar('}'));
        }
    }

    delete dlg;

    emit signalTokenTriggered(tmp);
}

void SequenceNumberParser::parseOperation(const QString& parseString, const ParseInformation& info, ParseResults& results)
{
    QRegExp regExp("(#+)(\\{\\s*(\\d+)\\s*,?\\s*(\\d+)*\\s*\\})?");
    int slength = 0;
    int start   = 0;
    int step    = 0;
    int number  = 0;
    int index   = info.index;

    // --------------------------------------------------------

    QString tmp;
    PARSE_LOOP_START(parseString, regExp)
    {
        slength = regExp.cap(1).length();
        start   = regExp.cap(3).isEmpty() ? 1 : regExp.cap(3).toInt();
        step    = regExp.cap(4).isEmpty() ? 1 : regExp.cap(4).toInt();

        number = start + ((index - 1) * step);
        tmp    = QString("%1").arg(number, slength, 10, QChar('0'));
    }
    PARSE_LOOP_END(parseString, regExp, tmp, results)
}

} // namespace Digikam
