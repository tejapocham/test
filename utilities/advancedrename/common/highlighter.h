/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-19
 * Description : syntax highlighter for AdvancedRename utility
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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

// Qt includes

#include <QSyntaxHighlighter>
#include <QVector>
#include <QTextCharFormat>

class QTextEdit;

namespace Digikam
{

class Parser;

class Highlighter : public QSyntaxHighlighter
{

public:

    enum Colors
    {
        OptionColor     = Qt::red,
        ModifierColor   = Qt::darkGreen,
        QuotedTextColor = Qt::blue,
        ParameterColor  = Qt::darkYellow
    };

    enum PatternType
    {
        OptionPattern = 0,
        ModifierPattern,
        QuotedTextPattern,
        ParameterPattern
    };

public:

    Highlighter(QTextEdit* parent, Parser* parser);
    virtual ~Highlighter();

protected:

    virtual void highlightBlock(const QString& text);

private:

    void setupHighlightingGrammar(Parser* parser);

private:

    struct HighlightingRule
    {
        PatternType     type;
        QRegExp         pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;
    HighlightingRule          quotationRule;

    QTextCharFormat optionFormat;
    QTextCharFormat parameterFormat;
    QTextCharFormat modifierFormat;
    QTextCharFormat quotationFormat;
};

}  // namespace Digikam


#endif /* HIGHLIGHTER_H */
