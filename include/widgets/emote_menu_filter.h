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

#endif // EMOTE_MENU_FILTER_H

class ButtonLoader : public QObject {
    Q_OBJECT

public:
    ButtonLoader(QObject *parent = nullptr) : QObject(parent) {}

    void setParams(const QStringList &emoteIds, bool isIniswap, const QString &subfolderPath, QString charName, int buttonSize) {
        this->emoteIds = emoteIds;
        this->isIniswap = isIniswap;
        this->subfolderPath = subfolderPath;
        this->charName = charName;
        this->buttonSize = buttonSize;
    }

public slots:
    void process() {
        QString currentCharName = charName;
        if (isIniswap && !subfolderPath.isEmpty()) {
            currentCharName = subfolderPath;
        }

        int total_emotes = ao_app->get_emote_number(currentCharName);

        for (int n = 0; n < total_emotes; ++n) {
            QString emoteId = QString::number(n + 1);
            QString emoteName = ao_app->get_emote_comment(currentCharName, n);

            if (!emoteIds.isEmpty() && (!emoteIds.contains(emoteId) && !emoteIds.contains(emoteName))) {
                continue;
            }

            QString emotePath = ao_app->get_image_suffix(ao_app->get_character_path(currentCharName, "emotions/button" + QString::number(n + 1) + "_off"));

            emit buttonLoaded(emotePath, emoteId, emoteName, currentCharName, buttonSize);

            // Small sleep to simulate batch processing and prevent overloading UI
            QThread::msleep(10);
        }

        emit finished();
    }

signals:
    void buttonLoaded(const QString &emotePath, const QString &emoteId, const QString &emoteName, const QString &charName, int buttonSize);
    void finished();

private:
    QStringList emoteIds;
    bool isIniswap;
    QString subfolderPath;
    QString charName;
    int buttonSize;
};
