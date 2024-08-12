#ifndef EMOTE_MENU_FILTER_H
#define EMOTE_MENU_FILTER_H

#include "aoemotebutton.h"
#include "eventfilters.h"
#include <QResizeEvent>
#include <QShowEvent>
#include <QFocusEvent>
#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include <QPushButton>
#include <QGridLayout>

#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>

#include <QFile>
#include <QHash>
#include <QTextStream>

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
    QString getEmoteMenuChat();
    QTextEdit* getEmoteMenuMsgBox() const { return messageBox; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    // void focusInEvent(QFocusEvent *event) override;
    // void focusOutEvent(QFocusEvent *event) override;

private slots:
    // void onCategoryChanged();
    // void onSearchTextChanged(const QString &text);
    void addCategory();
    void removeCategory();
    void onCategorySelected(QListWidgetItem *item);
    void onButtonClicked(AOEmoteButton *button);

private:
    void setupLayout();
    void setupCategories();
    void loadButtons(const QStringList &emoteIds = QStringList());
    void arrangeButtons();
    void saveTagsToFile(const QHash<QString, QStringList> &tags);
    
	void updateButtonSelection(AOEmoteButton *button, bool isSelected);

    AOApplication *ao_app;
    Courtroom *courtroom;
    QListWidget *categoryList;
    QTextEdit *messageBox;
    QScrollArea *scrollArea;
    QWidget *buttonContainer;
    QWidget *containerWidget;
    QGridLayout *gridLayout;
    QPushButton *addCategoryButton;
    QPushButton *removeCategoryButton;
    QTextEditFilter *emote_menu_ic_chat_filter;
    
    QVector<AOEmoteButton*> spriteButtons;
    QList<AOEmoteButton*> selectedButtons;
};

class TagDialog : public QDialog {
    Q_OBJECT
public:
    explicit TagDialog(const QStringList &categories, QWidget *parent = nullptr);
    QStringList selectedTags() const;
    
private:
    QVBoxLayout *mainLayout;
    QGroupBox *groupBox;
    QVBoxLayout *groupBoxLayout;
    QList<QCheckBox *> checkboxes;
    QWidget *scrollWidget;
    QVBoxLayout *scrollLayout;
    QScrollArea *scrollArea;
};

#endif // EMOTE_MENU_FILTER_H
