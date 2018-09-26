/********************************************************************************
** Form generated from reading UI file 'navigation.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NAVIGATION_H
#define UI_NAVIGATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>
#include "ArrowDiscWidget.h"
#include "NavigationButton.h"
#include "NavigationSlider.h"

QT_BEGIN_NAMESPACE

class Ui_Navigation
{
public:
    Marble::ArrowDiscWidget *arrowDisc;
    Marble::NavigationButton *zoomInButton;
    Marble::NavigationButton *zoomOutButton;
    Marble::NavigationSlider *zoomSlider;

    void setupUi(QWidget *Navigation)
    {
        if (Navigation->objectName().isEmpty())
            Navigation->setObjectName(QStringLiteral("Navigation"));
        Navigation->resize(70, 360);
        arrowDisc = new Marble::ArrowDiscWidget(Navigation);
        arrowDisc->setObjectName(QStringLiteral("arrowDisc"));
        arrowDisc->setGeometry(QRect(0, 0, 70, 70));

        zoomInButton = new Marble::NavigationButton(Navigation);
        zoomInButton->setObjectName(QStringLiteral("zoomInButton"));
        zoomInButton->setGeometry(QRect(18, 109, 34, 36));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/marble/navigation/navigational_zoom_plus.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon1.addFile(QStringLiteral(":/marble/navigation/navigational_zoom_press_plus.png"), QSize(), QIcon::Disabled, QIcon::Off);
        icon1.addFile(QStringLiteral(":/marble/navigation/navigational_zoom_hover_plus.png"), QSize(), QIcon::Active, QIcon::Off);
        icon1.addFile(QStringLiteral(":/marble/navigation/navigational_zoom_press_plus.png"), QSize(), QIcon::Selected, QIcon::Off);
        zoomInButton->setProperty("icon", QVariant(icon1));
        zoomInButton->setProperty("iconSize", QVariant(QSize(34, 36)));
        zoomOutButton = new Marble::NavigationButton(Navigation);
        zoomOutButton->setObjectName(QStringLiteral("zoomOutButton"));
        zoomOutButton->setGeometry(QRect(18, 305, 34, 36));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/marble/navigation/navigational_zoom_minus.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon2.addFile(QStringLiteral(":/marble/navigation/navigational_zoom_press_minus.png"), QSize(), QIcon::Disabled, QIcon::Off);
        icon2.addFile(QStringLiteral(":/marble/navigation/navigational_zoom_hover_minus.png"), QSize(), QIcon::Active, QIcon::Off);
        icon2.addFile(QStringLiteral(":/marble/navigation/navigational_zoom_press_minus.png"), QSize(), QIcon::Selected, QIcon::Off);
        zoomOutButton->setProperty("icon", QVariant(icon2));
        zoomOutButton->setProperty("iconSize", QVariant(QSize(34, 36)));
        zoomSlider = new Marble::NavigationSlider(Navigation);
        zoomSlider->setObjectName(QStringLiteral("zoomSlider"));
        zoomSlider->setGeometry(QRect(18, 145, 34, 160));

        retranslateUi(Navigation);

        QMetaObject::connectSlotsByName(Navigation);
    } // setupUi

    void retranslateUi(QWidget *Navigation)
    {
        Navigation->setWindowTitle(QApplication::translate("Navigation", "Navigation", 0));
        zoomInButton->setProperty("text", QVariant(QString()));
        zoomOutButton->setProperty("text", QVariant(QString()));
    } // retranslateUi

};

namespace Ui {
    class Navigation: public Ui_Navigation {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NAVIGATION_H
