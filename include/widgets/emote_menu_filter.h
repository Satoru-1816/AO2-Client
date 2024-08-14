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
    QString getEmoteMenuChat(bool clear);
    QString emoteMenu_charName;

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
    void onButtonLoaded(const QString &emotePath, const QString &emoteId, const QString &emoteName, const QString &charName, int buttonSize);

private:
    void setupLayout();
    void setupCategories();
    void loadButtons(const QStringList &emoteIds = QStringList(), bool isIniswap = false, const QString &subfolderPath = "");
    void arrangeButtons();
    void saveTagsToFile(const QHash<QString, QStringList> &tags);
    void removeCategoryFromFile(const QString &category);
    void removeFromCurrentTag(AOEmoteButton *button);
    void removeButtonsFromCategory(const QString &category, const QHash<QString, QStringList> &buttonsToRemove);
    
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

class ButtonLoader : public QObject {
    Q_OBJECT

public:
    ButtonLoader(AOApplication *p_ao_app, EmoteMenuFilter *parent = nullptr);

    void setParams(const QStringList &emoteIds, bool isIniswap, const QString &subfolderPath, QString charName, int buttonSize) {
        this->emoteIds = emoteIds;
        this->isIniswap = isIniswap;
        this->subfolderPath = subfolderPath;
        this->charName = charName;
        this->buttonSize = buttonSize;
    }

public slots:
    void process();

signals:
    void buttonLoaded(const QString &emotePath, const QString &emoteId, const QString &emoteName, const QString &charName, int buttonSize);
    void finished();

private:
	AOApplication *ao_app;
    EmoteMenuFilter *emoteMenuFilter;
    QStringList emoteIds;
    bool isIniswap;
    QString subfolderPath;
    QString charName;
    int buttonSize;
};

#endif // EMOTE_MENU_FILTER_H
