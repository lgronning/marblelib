/********************************************************************************
** Form generated from reading UI file 'MarbleNavigationSettingsWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MARBLENAVIGATIONSETTINGSWIDGET_H
#define UI_MARBLENAVIGATIONSETTINGSWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MarbleNavigationSettingsWidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_dragLocation;
    QComboBox *kcfg_dragLocation;
    QCheckBox *kcfg_inertialEarthRotation;
    QCheckBox *kcfg_animateTargetVoyage;
    QSpacerItem *spacer;
    QFormLayout *formLayout;
    QLabel *label_onStartup;
    QComboBox *kcfg_onStartup;
    QLabel *label_externalEditor;
    QComboBox *kcfg_externalMapEditor;
    QSpacerItem *spacerItem;

    void setupUi(QWidget *MarbleNavigationSettingsWidget)
    {
        if (MarbleNavigationSettingsWidget->objectName().isEmpty())
            MarbleNavigationSettingsWidget->setObjectName(QStringLiteral("MarbleNavigationSettingsWidget"));
        MarbleNavigationSettingsWidget->resize(333, 257);
        verticalLayout_2 = new QVBoxLayout(MarbleNavigationSettingsWidget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        groupBox = new QGroupBox(MarbleNavigationSettingsWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label_dragLocation = new QLabel(groupBox);
        label_dragLocation->setObjectName(QStringLiteral("label_dragLocation"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_dragLocation->sizePolicy().hasHeightForWidth());
        label_dragLocation->setSizePolicy(sizePolicy);
        label_dragLocation->setMinimumSize(QSize(118, 0));

        horizontalLayout->addWidget(label_dragLocation);

        kcfg_dragLocation = new QComboBox(groupBox);
        kcfg_dragLocation->setObjectName(QStringLiteral("kcfg_dragLocation"));
        kcfg_dragLocation->setEnabled(false);

        horizontalLayout->addWidget(kcfg_dragLocation);


        verticalLayout->addLayout(horizontalLayout);

        kcfg_inertialEarthRotation = new QCheckBox(groupBox);
        kcfg_inertialEarthRotation->setObjectName(QStringLiteral("kcfg_inertialEarthRotation"));

        verticalLayout->addWidget(kcfg_inertialEarthRotation);

        kcfg_animateTargetVoyage = new QCheckBox(groupBox);
        kcfg_animateTargetVoyage->setObjectName(QStringLiteral("kcfg_animateTargetVoyage"));

        verticalLayout->addWidget(kcfg_animateTargetVoyage);


        verticalLayout_2->addWidget(groupBox);

        spacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_2->addItem(spacer);

        formLayout = new QFormLayout();
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
        label_onStartup = new QLabel(MarbleNavigationSettingsWidget);
        label_onStartup->setObjectName(QStringLiteral("label_onStartup"));
        sizePolicy.setHeightForWidth(label_onStartup->sizePolicy().hasHeightForWidth());
        label_onStartup->setSizePolicy(sizePolicy);
        label_onStartup->setMinimumSize(QSize(118, 0));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_onStartup);

        kcfg_onStartup = new QComboBox(MarbleNavigationSettingsWidget);
        kcfg_onStartup->setObjectName(QStringLiteral("kcfg_onStartup"));

        formLayout->setWidget(0, QFormLayout::FieldRole, kcfg_onStartup);

        label_externalEditor = new QLabel(MarbleNavigationSettingsWidget);
        label_externalEditor->setObjectName(QStringLiteral("label_externalEditor"));
        sizePolicy.setHeightForWidth(label_externalEditor->sizePolicy().hasHeightForWidth());
        label_externalEditor->setSizePolicy(sizePolicy);
        label_externalEditor->setMinimumSize(QSize(118, 0));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_externalEditor);

        kcfg_externalMapEditor = new QComboBox(MarbleNavigationSettingsWidget);
        kcfg_externalMapEditor->setObjectName(QStringLiteral("kcfg_externalMapEditor"));

        formLayout->setWidget(1, QFormLayout::FieldRole, kcfg_externalMapEditor);


        verticalLayout_2->addLayout(formLayout);

        spacerItem = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(spacerItem);

#ifndef QT_NO_SHORTCUT
        label_dragLocation->setBuddy(kcfg_dragLocation);
        label_onStartup->setBuddy(kcfg_onStartup);
        label_externalEditor->setBuddy(kcfg_onStartup);
#endif // QT_NO_SHORTCUT

        retranslateUi(MarbleNavigationSettingsWidget);

        QMetaObject::connectSlotsByName(MarbleNavigationSettingsWidget);
    } // setupUi

    void retranslateUi(QWidget *MarbleNavigationSettingsWidget)
    {
        MarbleNavigationSettingsWidget->setWindowTitle(QApplication::translate("MarbleNavigationSettingsWidget", "Marble Cache Settings", 0));
        groupBox->setTitle(QApplication::translate("MarbleNavigationSettingsWidget", "Dragging and Animation", 0));
        label_dragLocation->setText(QApplication::translate("MarbleNavigationSettingsWidget", "&Drag location:", 0));
        kcfg_dragLocation->clear();
        kcfg_dragLocation->insertItems(0, QStringList()
         << QApplication::translate("MarbleNavigationSettingsWidget", "Keep Planet Axis Vertically", 0)
         << QApplication::translate("MarbleNavigationSettingsWidget", "Follow Mouse Pointer", 0)
        );
#ifndef QT_NO_TOOLTIP
        kcfg_dragLocation->setToolTip(QApplication::translate("MarbleNavigationSettingsWidget", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">While dragging the mouse there are two standard behaviours when dealing with a virtual globe:</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">The location below mouse pointer will follow the cursor exactly: As a result e.g. the north pole will not stay at the top which can lead to confusion. By default Marble makes sure that north is always up which results in a dragging behaviour where the location below the mouse pointer slightly \"detaches\" from the cursor. </p></body></html>", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        kcfg_inertialEarthRotation->setToolTip(QApplication::translate("MarbleNavigationSettingsWidget", "Use kinetic spinning when dragging the map", 0));
#endif // QT_NO_TOOLTIP
        kcfg_inertialEarthRotation->setText(QApplication::translate("MarbleNavigationSettingsWidget", "&Inertial Globe Rotation", 0));
#ifndef QT_NO_TOOLTIP
        kcfg_animateTargetVoyage->setToolTip(QApplication::translate("MarbleNavigationSettingsWidget", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">When searching for a location Marble can either move instantly to the new location or it can show a travel animation from the previous place to the new place.</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        kcfg_animateTargetVoyage->setText(QApplication::translate("MarbleNavigationSettingsWidget", "&Animate voyage to the target", 0));
        label_onStartup->setText(QApplication::translate("MarbleNavigationSettingsWidget", "&On startup:", 0));
        kcfg_onStartup->clear();
        kcfg_onStartup->insertItems(0, QStringList()
         << QApplication::translate("MarbleNavigationSettingsWidget", "Show Home Location", 0)
         << QApplication::translate("MarbleNavigationSettingsWidget", "Return to Last Location Visited", 0)
        );
#ifndef QT_NO_TOOLTIP
        kcfg_onStartup->setToolTip(QApplication::translate("MarbleNavigationSettingsWidget", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">By default Marble will display the home location immediately after the application has started. As an alternative it can also show the last position that was active when the user left the application. </p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        label_externalEditor->setText(QApplication::translate("MarbleNavigationSettingsWidget", "&External Editor:", 0));
        kcfg_externalMapEditor->clear();
        kcfg_externalMapEditor->insertItems(0, QStringList()
         << QApplication::translate("MarbleNavigationSettingsWidget", "Always ask", 0)
         << QApplication::translate("MarbleNavigationSettingsWidget", "Potlatch (Web browser)", 0)
         << QApplication::translate("MarbleNavigationSettingsWidget", "JOSM", 0)
         << QApplication::translate("MarbleNavigationSettingsWidget", "Merkaartor", 0)
        );
#ifndef QT_NO_TOOLTIP
        kcfg_externalMapEditor->setToolTip(QApplication::translate("MarbleNavigationSettingsWidget", "<p>The application that is launched to edit maps. Potlatch (default) requires a web browser with flash support to be installed. When selecting josm or merkaartor, make sure that the respective application is installed.</p>", 0));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class MarbleNavigationSettingsWidget: public Ui_MarbleNavigationSettingsWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MARBLENAVIGATIONSETTINGSWIDGET_H
