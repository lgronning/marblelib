/********************************************************************************
** Form generated from reading UI file 'MarbleCacheSettingsWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MARBLECACHESETTINGSWIDGET_H
#define UI_MARBLECACHESETTINGSWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MarbleCacheSettingsWidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox_cache;
    QGridLayout *gridLayout;
    QLabel *label_volatileCache;
    QSpinBox *kcfg_volatileTileCacheLimit;
    QPushButton *button_clearVolatileCache;
    QSpacerItem *spacerItem;
    QLabel *label_persistentCache;
    QSpinBox *kcfg_persistentTileCacheLimit;
    QPushButton *button_clearPersistentCache;
    QSpacerItem *spacerItem1;
    QGroupBox *groupBox_proxy;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_proxyUrl;
    QLineEdit *kcfg_proxyUrl;
    QLabel *label_proxyPort;
    QSpinBox *kcfg_proxyPort;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label_proxyType;
    QComboBox *kcfg_proxyType;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *verticalSpacer_3;
    QGroupBox *kcfg_proxyAuth;
    QGridLayout *gridLayout1;
    QLabel *label_username;
    QLabel *label_password;
    QLineEdit *kcfg_proxyPass;
    QLineEdit *kcfg_proxyUser;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *MarbleCacheSettingsWidget)
    {
        if (MarbleCacheSettingsWidget->objectName().isEmpty())
            MarbleCacheSettingsWidget->setObjectName(QStringLiteral("MarbleCacheSettingsWidget"));
        MarbleCacheSettingsWidget->resize(459, 380);
        verticalLayout_2 = new QVBoxLayout(MarbleCacheSettingsWidget);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        groupBox_cache = new QGroupBox(MarbleCacheSettingsWidget);
        groupBox_cache->setObjectName(QStringLiteral("groupBox_cache"));
        gridLayout = new QGridLayout(groupBox_cache);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label_volatileCache = new QLabel(groupBox_cache);
        label_volatileCache->setObjectName(QStringLiteral("label_volatileCache"));
        QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_volatileCache->sizePolicy().hasHeightForWidth());
        label_volatileCache->setSizePolicy(sizePolicy);
        label_volatileCache->setMinimumSize(QSize(118, 0));

        gridLayout->addWidget(label_volatileCache, 0, 0, 1, 1);

        kcfg_volatileTileCacheLimit = new QSpinBox(groupBox_cache);
        kcfg_volatileTileCacheLimit->setObjectName(QStringLiteral("kcfg_volatileTileCacheLimit"));
        kcfg_volatileTileCacheLimit->setAlignment(Qt::AlignRight);
        kcfg_volatileTileCacheLimit->setMaximum(999999);
        kcfg_volatileTileCacheLimit->setValue(100);

        gridLayout->addWidget(kcfg_volatileTileCacheLimit, 0, 1, 1, 1);

        button_clearVolatileCache = new QPushButton(groupBox_cache);
        button_clearVolatileCache->setObjectName(QStringLiteral("button_clearVolatileCache"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(button_clearVolatileCache->sizePolicy().hasHeightForWidth());
        button_clearVolatileCache->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(button_clearVolatileCache, 0, 2, 1, 1);

        spacerItem = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem, 0, 4, 1, 1);

        label_persistentCache = new QLabel(groupBox_cache);
        label_persistentCache->setObjectName(QStringLiteral("label_persistentCache"));
        sizePolicy.setHeightForWidth(label_persistentCache->sizePolicy().hasHeightForWidth());
        label_persistentCache->setSizePolicy(sizePolicy);
        label_persistentCache->setMinimumSize(QSize(118, 0));

        gridLayout->addWidget(label_persistentCache, 1, 0, 1, 1);

        kcfg_persistentTileCacheLimit = new QSpinBox(groupBox_cache);
        kcfg_persistentTileCacheLimit->setObjectName(QStringLiteral("kcfg_persistentTileCacheLimit"));
        kcfg_persistentTileCacheLimit->setMinimumSize(QSize(100, 0));
        kcfg_persistentTileCacheLimit->setAlignment(Qt::AlignRight);
        kcfg_persistentTileCacheLimit->setMaximum(999999);
        kcfg_persistentTileCacheLimit->setValue(999999);

        gridLayout->addWidget(kcfg_persistentTileCacheLimit, 1, 1, 1, 1);

        button_clearPersistentCache = new QPushButton(groupBox_cache);
        button_clearPersistentCache->setObjectName(QStringLiteral("button_clearPersistentCache"));
        sizePolicy1.setHeightForWidth(button_clearPersistentCache->sizePolicy().hasHeightForWidth());
        button_clearPersistentCache->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(button_clearPersistentCache, 1, 2, 1, 2);

        spacerItem1 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(spacerItem1, 1, 4, 1, 1);


        verticalLayout_2->addWidget(groupBox_cache);

        groupBox_proxy = new QGroupBox(MarbleCacheSettingsWidget);
        groupBox_proxy->setObjectName(QStringLiteral("groupBox_proxy"));
        verticalLayout = new QVBoxLayout(groupBox_proxy);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label_proxyUrl = new QLabel(groupBox_proxy);
        label_proxyUrl->setObjectName(QStringLiteral("label_proxyUrl"));

        horizontalLayout->addWidget(label_proxyUrl);

        kcfg_proxyUrl = new QLineEdit(groupBox_proxy);
        kcfg_proxyUrl->setObjectName(QStringLiteral("kcfg_proxyUrl"));

        horizontalLayout->addWidget(kcfg_proxyUrl);

        label_proxyPort = new QLabel(groupBox_proxy);
        label_proxyPort->setObjectName(QStringLiteral("label_proxyPort"));

        horizontalLayout->addWidget(label_proxyPort);

        kcfg_proxyPort = new QSpinBox(groupBox_proxy);
        kcfg_proxyPort->setObjectName(QStringLiteral("kcfg_proxyPort"));
        kcfg_proxyPort->setMaximum(65535);
        kcfg_proxyPort->setValue(8081);

        horizontalLayout->addWidget(kcfg_proxyPort);


        verticalLayout->addLayout(horizontalLayout);

        verticalSpacer_2 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalSpacer_4 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);

        label_proxyType = new QLabel(groupBox_proxy);
        label_proxyType->setObjectName(QStringLiteral("label_proxyType"));

        horizontalLayout_2->addWidget(label_proxyType);

        kcfg_proxyType = new QComboBox(groupBox_proxy);
        kcfg_proxyType->setObjectName(QStringLiteral("kcfg_proxyType"));

        horizontalLayout_2->addWidget(kcfg_proxyType);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalSpacer_3 = new QSpacerItem(20, 13, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_3);

        kcfg_proxyAuth = new QGroupBox(groupBox_proxy);
        kcfg_proxyAuth->setObjectName(QStringLiteral("kcfg_proxyAuth"));
        kcfg_proxyAuth->setFlat(true);
        kcfg_proxyAuth->setCheckable(true);
        kcfg_proxyAuth->setChecked(false);
        gridLayout1 = new QGridLayout(kcfg_proxyAuth);
        gridLayout1->setObjectName(QStringLiteral("gridLayout1"));
        label_username = new QLabel(kcfg_proxyAuth);
        label_username->setObjectName(QStringLiteral("label_username"));

        gridLayout1->addWidget(label_username, 0, 0, 1, 1);

        label_password = new QLabel(kcfg_proxyAuth);
        label_password->setObjectName(QStringLiteral("label_password"));

        gridLayout1->addWidget(label_password, 1, 0, 1, 1);

        kcfg_proxyPass = new QLineEdit(kcfg_proxyAuth);
        kcfg_proxyPass->setObjectName(QStringLiteral("kcfg_proxyPass"));
        kcfg_proxyPass->setEnabled(false);
        kcfg_proxyPass->setEchoMode(QLineEdit::Password);

        gridLayout1->addWidget(kcfg_proxyPass, 1, 1, 1, 1);

        kcfg_proxyUser = new QLineEdit(kcfg_proxyAuth);
        kcfg_proxyUser->setObjectName(QStringLiteral("kcfg_proxyUser"));
        kcfg_proxyUser->setEnabled(false);

        gridLayout1->addWidget(kcfg_proxyUser, 0, 1, 1, 1);


        verticalLayout->addWidget(kcfg_proxyAuth);


        verticalLayout_2->addWidget(groupBox_proxy);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

#ifndef QT_NO_SHORTCUT
        label_volatileCache->setBuddy(kcfg_volatileTileCacheLimit);
        label_persistentCache->setBuddy(kcfg_persistentTileCacheLimit);
        label_proxyUrl->setBuddy(kcfg_proxyUrl);
        label_proxyPort->setBuddy(kcfg_proxyPort);
        label_username->setBuddy(kcfg_proxyUser);
        label_password->setBuddy(kcfg_proxyPass);
#endif // QT_NO_SHORTCUT

        retranslateUi(MarbleCacheSettingsWidget);

        QMetaObject::connectSlotsByName(MarbleCacheSettingsWidget);
    } // setupUi

    void retranslateUi(QWidget *MarbleCacheSettingsWidget)
    {
#ifndef QT_NO_TOOLTIP
        groupBox_cache->setToolTip(QApplication::translate("MarbleCacheSettingsWidget", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">There are two caches being used for Marble: The physical memory which is needed to keep map data in the computer's memory. Increasing the value will make the application more responsive. The hard disc memory cache is used by download contents from the Internet (e.g. Wikipedia data or map data). Decrease this value if you want to save space on the hard disc and if high usage of the Internet is not an issue. </p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        groupBox_cache->setTitle(QApplication::translate("MarbleCacheSettingsWidget", "C&ache", 0));
        label_volatileCache->setText(QApplication::translate("MarbleCacheSettingsWidget", "&Physical memory:", 0));
        kcfg_volatileTileCacheLimit->setSuffix(QApplication::translate("MarbleCacheSettingsWidget", " MB", 0));
        button_clearVolatileCache->setText(QApplication::translate("MarbleCacheSettingsWidget", "C&lear", 0));
        label_persistentCache->setText(QApplication::translate("MarbleCacheSettingsWidget", "&Hard disc:", 0));
        kcfg_persistentTileCacheLimit->setSpecialValueText(QApplication::translate("MarbleCacheSettingsWidget", "Unlimited", 0));
        kcfg_persistentTileCacheLimit->setSuffix(QApplication::translate("MarbleCacheSettingsWidget", " MB", 0));
        button_clearPersistentCache->setText(QApplication::translate("MarbleCacheSettingsWidget", "Cl&ear", 0));
#ifndef QT_NO_TOOLTIP
        groupBox_proxy->setToolTip(QApplication::translate("MarbleCacheSettingsWidget", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Proxy settings for your local intranet. Please leave empty if there is no proxy.</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        groupBox_proxy->setTitle(QApplication::translate("MarbleCacheSettingsWidget", "&Proxy", 0));
        label_proxyUrl->setText(QApplication::translate("MarbleCacheSettingsWidget", "&Proxy:", 0));
        kcfg_proxyUrl->setText(QString());
        label_proxyPort->setText(QApplication::translate("MarbleCacheSettingsWidget", "P&ort:", 0));
        label_proxyType->setText(QApplication::translate("MarbleCacheSettingsWidget", "Proxy type:", 0));
        kcfg_proxyType->clear();
        kcfg_proxyType->insertItems(0, QStringList()
         << QApplication::translate("MarbleCacheSettingsWidget", "Http", 0)
         << QApplication::translate("MarbleCacheSettingsWidget", "Socks5", 0)
        );
        kcfg_proxyAuth->setTitle(QApplication::translate("MarbleCacheSettingsWidget", "Requires authentication", 0));
        kcfg_proxyAuth->setProperty("text", QVariant(QApplication::translate("MarbleCacheSettingsWidget", "Requires authentication", 0)));
        label_username->setText(QApplication::translate("MarbleCacheSettingsWidget", "U&sername:", 0));
        label_password->setText(QApplication::translate("MarbleCacheSettingsWidget", "&Password:", 0));
        Q_UNUSED(MarbleCacheSettingsWidget);
    } // retranslateUi

};

namespace Ui {
    class MarbleCacheSettingsWidget: public Ui_MarbleCacheSettingsWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MARBLECACHESETTINGSWIDGET_H
