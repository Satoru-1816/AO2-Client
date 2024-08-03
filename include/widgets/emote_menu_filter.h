#ifndef EMOTE_MENU_FILTER_H
#define EMOTE_MENU_FILTER_H

#include "aoemotebutton.h"

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QScrollArea>
#include <QPushButton>
#include <QGridLayout>

class AOApplication;

QT_BEGIN_NAMESPACE
namespace Ui { class EmoteMenuFilter; }
QT_END_NAMESPACE

class EmoteMenuFilter : public QDialog
{
    Q_OBJECT

public:
    EmoteMenuFilter(QDialog *parent = nullptr, AOApplication *p_ao_app = nullptr);
    ~EmoteMenuFilter();

private slots:
    void onCategoryChanged();
    void onSearchTextChanged(const QString &text);

private:
	AOApplication *ao_app;
    Ui::EmoteMenuFilter *emote_filter_ui;
    QListWidget *categoryList;
    QLineEdit *searchBox;
    QScrollArea *scrollArea;
    QWidget *buttonContainer;
    QGridLayout *gridLayout;
    QPushButton *addCategory;
    QPushButton *removeCategory;

};

#endif // EMOTE_MENU_FILTER_H
