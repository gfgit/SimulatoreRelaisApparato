/**
 * src/utils/colorselectionwidget.cpp
 *
 * This file is part of the Simulatore Relais Apparato source code.
 *
 * Copyright (C) 2024 Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "colorselectionwidget.h"

#include <QColorDialog>
#include <QPointer>

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

#include <QPainter>

class ColorWidget : public QWidget
{
public:
    ColorWidget() = default;

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.fillRect(rect(), mColor);
    }

    inline QColor color() const
    {
        return mColor;
    }

    inline void setColor(const QColor &newColor)
    {
        mColor = newColor;
        update();
    }

private:
    QColor mColor;
};

ColorSelectionWidget::ColorSelectionWidget(QWidget *parent)
    : QWidget{parent}
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    mColorNameEdit = new QLineEdit;
    mColorNameEdit->setPlaceholderText(tr("#RRGGBB"));
    lay->addWidget(mColorNameEdit);

    mColorWidget = new ColorWidget;
    mColorWidget->resize(mColorNameEdit->height(),
                         mColorNameEdit->height());
    mColorWidget->setMinimumSize(20, 20);
    lay->addWidget(mColorWidget);

    mChooseBut = new QPushButton(tr("Choose"));
    lay->addWidget(mChooseBut);

    connect(mChooseBut, &QPushButton::clicked,
            this, &ColorSelectionWidget::showColorDialog);
    connect(mColorNameEdit, &QLineEdit::editingFinished,
            this, &ColorSelectionWidget::parseColorName);
}

QColor ColorSelectionWidget::color() const
{
    return mColorWidget->color();
}

void ColorSelectionWidget::setColor(const QColor &newColor)
{
    if (mColorWidget->color() == newColor)
        return;

    mColorWidget->setColor(newColor);
    mColorNameEdit->setText(newColor.name(QColor::HexRgb));
    emit colorChanged(newColor);
}

void ColorSelectionWidget::showColorDialog()
{
    QPointer<QColorDialog> dlg = new QColorDialog(this);

    dlg->setCustomColor(0, Qt::red);
    dlg->setCustomColor(1, Qt::blue);
    dlg->setCustomColor(2, Qt::cyan);
    dlg->setCustomColor(3, Qt::darkGreen);
    dlg->setCustomColor(4, Qt::yellow);
    dlg->setCustomColor(5, Qt::darkYellow);

    dlg->setCurrentColor(color());

    if(dlg->exec() != QDialog::Accepted || !dlg)
        return;

    setColor(dlg->selectedColor());
}

void ColorSelectionWidget::parseColorName()
{
    if(QColor::isValidColorName(mColorNameEdit->text()))
        setColor(QColor::fromString(mColorNameEdit->text()));
    else
        mColorNameEdit->setText(color().name(QColor::HexRgb));
}
