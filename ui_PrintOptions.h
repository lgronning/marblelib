/********************************************************************************
** Form generated from reading UI file 'PrintOptions.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRINTOPTIONS_H
#define UI_PRINTOPTIONS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PrintOptions
{
public:
    QGridLayout *gridLayout_3;
    QGroupBox *mapGroupBox;
    QGridLayout *gridLayout;
    QCheckBox *mapCheckBox;
    QSpacerItem *horizontalSpacer;
    QCheckBox *backgroundCheckBox;
    QSpacerItem *horizontalSpacer_2;
    QCheckBox *legendCheckBox;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *PrintOptions)
    {
        if (PrintOptions->objectName().isEmpty())
            PrintOptions->setObjectName(QStringLiteral("PrintOptions"));
        PrintOptions->resize(333, 136);
        gridLayout_3 = new QGridLayout(PrintOptions);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        mapGroupBox = new QGroupBox(PrintOptions);
        mapGroupBox->setObjectName(QStringLiteral("mapGroupBox"));
        gridLayout = new QGridLayout(mapGroupBox);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        mapCheckBox = new QCheckBox(mapGroupBox);
        mapCheckBox->setObjectName(QStringLiteral("mapCheckBox"));
        mapCheckBox->setChecked(true);

        gridLayout->addWidget(mapCheckBox, 0, 0, 1, 2);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 1, 0, 1, 1);

        backgroundCheckBox = new QCheckBox(mapGroupBox);
        backgroundCheckBox->setObjectName(QStringLiteral("backgroundCheckBox"));

        gridLayout->addWidget(backgroundCheckBox, 1, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 1, 2, 1, 1);

        legendCheckBox = new QCheckBox(mapGroupBox);
        legendCheckBox->setObjectName(QStringLiteral("legendCheckBox"));
        legendCheckBox->setChecked(false);

        gridLayout->addWidget(legendCheckBox, 2, 0, 1, 2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 3, 0, 1, 1);


        gridLayout_3->addWidget(mapGroupBox, 0, 0, 1, 1);

        retranslateUi(PrintOptions);
        QObject::connect(mapCheckBox, SIGNAL(toggled(bool)), backgroundCheckBox, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(PrintOptions);
    } // setupUi

    void retranslateUi(QWidget *PrintOptions)
    {
        PrintOptions->setWindowTitle(QApplication::translate("PrintOptions", "Marble", 0));
        mapGroupBox->setTitle(QApplication::translate("PrintOptions", "Map", 0));
        mapCheckBox->setText(QApplication::translate("PrintOptions", "Visible Globe Region", 0));
        backgroundCheckBox->setText(QApplication::translate("PrintOptions", "Background", 0));
        legendCheckBox->setText(QApplication::translate("PrintOptions", "Legend", 0));
    } // retranslateUi

};

namespace Ui {
    class PrintOptions: public Ui_PrintOptions {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRINTOPTIONS_H
