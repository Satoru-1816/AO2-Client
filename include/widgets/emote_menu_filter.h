#ifndef EMOTE_MENU_FILTER_H
#define EMOTE_MENU_FILTER_H

#include "aoemotebutton.h"
#include "courtroom.h"
#include <QResizeEvent>
#include <QFocusEvent>
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QScrollArea>
#include <QPushButton>
#include <QGridLayout>

#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>

class AOApplication;
class Courtroom;

class EmoteMenuFilter : public QDialog
{
    Q_OBJECT

public:
    EmoteMenuFilter(QDialog *parent = nullptr, AOApplication *p_ao_app = nullptr, Courtroom *p_courtroom = nullptr);
    ~EmoteMenuFilter();
    void showTagDialog(AOEmoteButton *button);
    QStringList getCategoryList() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

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

class TagDialog : public QDialog {
    Q_OBJECT
public:
    explicit TagDialog(const QStringList &categories, QWidget *parent = nullptr);

private:
    QVBoxLayout *mainLayout;
    QGroupBox *groupBox;
    QVBoxLayout *groupBoxLayout;
    QList<QCheckBox *> checkboxes;
};

#endif // EMOTE_MENU_FILTER_H
