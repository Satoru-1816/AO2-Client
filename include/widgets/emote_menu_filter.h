#ifndef EMOTE_MENU_FILTER_H
#define EMOTE_MENU_FILTER_H

#include "aoemotebutton.h"
#include "courtroom.h"
#include <QResizeEvent>
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QScrollArea>
#include <QPushButton>
#include <QGridLayout>

class AOApplication;
class Courtroom;

class EmoteMenuFilter : public QDialog
{
    Q_OBJECT

public:
    EmoteMenuFilter(QDialog *parent = nullptr, AOApplication *p_ao_app = nullptr, Courtroom *p_courtroom = nullptr);
    ~EmoteMenuFilter();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // void onCategoryChanged();
    // void onSearchTextChanged(const QString &text);
    void addCategory();
    void removeCategory();

private:
    void setupLayout();
    void loadButtons();
    void arrangeButtons();

    AOApplication *ao_app;
    Courtroom *courtroom;
    QListWidget *categoryList;
    QLineEdit *searchBox;
    QScrollArea *scrollArea;
    QWidget *buttonContainer;
    QWidget *containerWidget;
    QGridLayout *gridLayout;
    QPushButton *addCategoryButton;
    QPushButton *removeCategoryButton;
    
    QVector<AOEmoteButton*> spriteButtons;
};

#endif // EMOTE_MENU_FILTER_H
