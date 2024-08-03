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

class EmoteMenuFilter : public QDialog
{
    Q_OBJECT

public:
    EmoteMenuFilter(QDialog *parent = nullptr, AOApplication *p_ao_app = nullptr);
    ~EmoteMenuFilter();

private slots:
    // void onCategoryChanged();
    // void onSearchTextChanged(const QString &text);
    void addCategory();
    void removeCategory();

private:
    void setupLayout();

    AOApplication *ao_app;
    QListWidget *categoryList;
    QLineEdit *searchBox;
    QScrollArea *scrollArea;
    QWidget *buttonContainer;
    QGridLayout *gridLayout;
    QPushButton *addCategoryButton;
    QPushButton *removeCategoryButton;
};

#endif // EMOTE_MENU_FILTER_H
