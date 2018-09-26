/********************************************************************************
** Form generated from reading UI file 'FileViewWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILEVIEWWIDGET_H
#define UI_FILEVIEWWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FileViewWidget
{
public:
    QVBoxLayout *verticalLayout;
    QTreeView *m_treeView;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *m_saveButton;
    QPushButton *m_closeButton;

    void setupUi(QWidget *FileViewWidget)
    {
        if (FileViewWidget->objectName().isEmpty())
            FileViewWidget->setObjectName(QStringLiteral("FileViewWidget"));
        FileViewWidget->resize(229, 282);
        verticalLayout = new QVBoxLayout(FileViewWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        m_treeView = new QTreeView(FileViewWidget);
        m_treeView->setObjectName(QStringLiteral("m_treeView"));
        m_treeView->setIconSize(QSize(0, 0));
        m_treeView->setTextElideMode(Qt::ElideLeft);

        verticalLayout->addWidget(m_treeView);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        m_saveButton = new QPushButton(FileViewWidget);
        m_saveButton->setObjectName(QStringLiteral("m_saveButton"));
        m_saveButton->setEnabled(false);

        hboxLayout->addWidget(m_saveButton);

        m_closeButton = new QPushButton(FileViewWidget);
        m_closeButton->setObjectName(QStringLiteral("m_closeButton"));
        m_closeButton->setEnabled(false);

        hboxLayout->addWidget(m_closeButton);


        verticalLayout->addLayout(hboxLayout);


        retranslateUi(FileViewWidget);

        QMetaObject::connectSlotsByName(FileViewWidget);
    } // setupUi

    void retranslateUi(QWidget *FileViewWidget)
    {
        FileViewWidget->setWindowTitle(QApplication::translate("FileViewWidget", "File View", 0));
        m_saveButton->setText(QApplication::translate("FileViewWidget", "&Save", 0));
        m_closeButton->setText(QApplication::translate("FileViewWidget", "&Close", 0));
    } // retranslateUi

};

namespace Ui {
    class FileViewWidget: public Ui_FileViewWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEVIEWWIDGET_H
