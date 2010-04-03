/*
    This file is part of LibQtLua.

    LibQtLua is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LibQtLua is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with LibQtLua.  If not, see <http://www.gnu.org/licenses/>.

    Copyright (C) 2008, Alexandre Becoulet <alexandre.becoulet@free.fr>

*/

#include <QMetaObject>

#include <QAbstractItemDelegate>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QClipboard>
#include <QColorDialog>
#include <QColumnView>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDesktopWidget>
#include <QDial>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFocusFrame>
#include <QFont>
#include <QFontDialog>
#include <QFrame>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemDelegate>
#include <QLCDNumber>
#include <QLabel>
#include <QLibrary>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QLocale>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QObject>
#include <QPainter>
#include <QPalette>
#include <QPluginLoader>
#include <QProcess>
#include <QProgressBar>
#include <QProgressDialog>
#include <QPushButton>
#include <QRadioButton>
#include <QRubberBand>
#include <QScrollArea>
#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QSlider>
#include <QSound>
#include <QSpinBox>
#include <QSplashScreen>
#include <QSplitter>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStringListModel>
#include <QTabBar>
#include <QTabWidget>
#include <QTableView>
#include <QTableWidget>
#include <QTemporaryFile>
#include <QTextDocument>
#include <QTextEdit>
#include <QThread>
#include <QTimeEdit>
#include <QTimer>
#include <QToolBar>
#include <QToolBox>
#include <QToolButton>
#include <QTranslator>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QValidator>
#include <QWidget>
#include <QWidgetAction>

#if QT_VERSION >= 0x040400
#include <QFormLayout>
#include <QCommandLinkButton>
#include <QPlainTextEdit>
#include <QStyledItemDelegate>
#endif

namespace QtLua {

const QMetaObject *meta_object_table[] = {
  &QAbstractItemDelegate::staticMetaObject,
  &QAbstractItemModel::staticMetaObject,
  &QAbstractItemView::staticMetaObject,
  &QAction::staticMetaObject,
  &QApplication::staticMetaObject,
  &QButtonGroup::staticMetaObject,
  &QCalendarWidget::staticMetaObject,
  &QCheckBox::staticMetaObject,
  &QClipboard::staticMetaObject,
  &QColorDialog::staticMetaObject,
  &QColumnView::staticMetaObject,
  &QComboBox::staticMetaObject,
  &QCoreApplication::staticMetaObject,
  &QDateEdit::staticMetaObject,
  &QDateTimeEdit::staticMetaObject,
  &QDesktopWidget::staticMetaObject,
  &QDial::staticMetaObject,
  &QDialog::staticMetaObject,
  &QDialogButtonBox::staticMetaObject,
  &QDoubleSpinBox::staticMetaObject,
  &QFile::staticMetaObject,
  &QFileDialog::staticMetaObject,
  &QFocusFrame::staticMetaObject,
  &QFont::staticMetaObject,
  &QFontDialog::staticMetaObject,
  &QFrame::staticMetaObject,
  &QGraphicsScene::staticMetaObject,
  &QGraphicsView::staticMetaObject,
  &QGridLayout::staticMetaObject,
  &QGroupBox::staticMetaObject,
  &QHBoxLayout::staticMetaObject,
  &QHeaderView::staticMetaObject,
  &QInputDialog::staticMetaObject,
  &QItemDelegate::staticMetaObject,
  &QLCDNumber::staticMetaObject,
  &QLabel::staticMetaObject,
  &QLibrary::staticMetaObject,
  &QLineEdit::staticMetaObject,
  &QListView::staticMetaObject,
  &QListWidget::staticMetaObject,
  &QLocale::staticMetaObject,
  &QMainWindow::staticMetaObject,
  &QMdiArea::staticMetaObject,
  &QMdiSubWindow::staticMetaObject,
  &QMenu::staticMetaObject,
  &QMenuBar::staticMetaObject,
  &QMessageBox::staticMetaObject,
  &QObject::staticMetaObject,
  &QPainter::staticMetaObject,
  &QPalette::staticMetaObject,
  &QPluginLoader::staticMetaObject,
  &QProcess::staticMetaObject,
  &QProgressBar::staticMetaObject,
  &QProgressDialog::staticMetaObject,
  &QPushButton::staticMetaObject,
  &QRadioButton::staticMetaObject,
  &QRubberBand::staticMetaObject,
  &QScrollArea::staticMetaObject,
  &QSettings::staticMetaObject,
  &QShortcut::staticMetaObject,
  &QSignalMapper::staticMetaObject,
  &QSlider::staticMetaObject,
  &QSound::staticMetaObject,
  &QSpinBox::staticMetaObject,
  &QSplashScreen::staticMetaObject,
  &QSplitter::staticMetaObject,
  &QStackedLayout::staticMetaObject,
  &QStackedWidget::staticMetaObject,
  &QStatusBar::staticMetaObject,
  &QStringListModel::staticMetaObject,
  &QTabBar::staticMetaObject,
  &QTabWidget::staticMetaObject,
  &QTableView::staticMetaObject,
  &QTableWidget::staticMetaObject,
  &QTemporaryFile::staticMetaObject,
  &QTextDocument::staticMetaObject,
  &QTextEdit::staticMetaObject,
  &QThread::staticMetaObject,
  &QTimeEdit::staticMetaObject,
  &QTimer::staticMetaObject,
  &QToolBar::staticMetaObject,
  &QToolBox::staticMetaObject,
  &QToolButton::staticMetaObject,
  &QTranslator::staticMetaObject,
  &QTreeView::staticMetaObject,
  &QTreeWidget::staticMetaObject,
  &QVBoxLayout::staticMetaObject,
  &QValidator::staticMetaObject,
  &QWidget::staticMetaObject,
  &QWidgetAction::staticMetaObject,

#if QT_VERSION >= 0x040400
  &QCommandLinkButton::staticMetaObject,
  &QFormLayout::staticMetaObject,
  &QPlainTextEdit::staticMetaObject,
  &QStyledItemDelegate::staticMetaObject,
#endif

  0
};

}

