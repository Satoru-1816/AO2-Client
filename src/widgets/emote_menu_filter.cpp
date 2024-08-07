#include "widgets/emote_menu_filter.h"
#include "aoemotebutton.h"
#include "aoapplication.h"
#include "courtroom.h"
#include <QResizeEvent>
#include <QFocusEvent>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QDir>

EmoteMenuFilter::EmoteMenuFilter(QDialog *parent, AOApplication *p_ao_app, Courtroom *p_courtroom)
    : QDialog(parent), ao_app(p_ao_app), courtroom(p_courtroom)
{
    categoryList = new QListWidget(this);
    searchBox = new QLineEdit(this);
    scrollArea = new QScrollArea(this);
    buttonContainer = new QWidget(this);
    gridLayout = new QGridLayout(buttonContainer);
    addCategoryButton = new QPushButton("Add Category", this);
    removeCategoryButton = new QPushButton("Remove Category", this);

    scrollArea->setWidget(buttonContainer);
    scrollArea->setWidgetResizable(true);
    searchBox->setPlaceholderText("Search...");

    setupLayout();

    categoryList->addItem("Default Emotes");
    categoryList->addItem("Favorites");
    categoryList->setMaximumHeight(150);

    containerWidget = new QWidget(this);
    containerWidget->setLayout(gridLayout);
    
    scrollArea->setWidget(containerWidget);
    scrollArea->setWidgetResizable(true);

    loadButtons();

    // connect(categoryList, &QListWidget::itemSelectionChanged, this, &EmoteMenuFilter::onCategorySelected);
    connect(categoryList, &QListWidget::itemClicked, this, &EmoteMenuFilter::onCategorySelected);
    connect(addCategoryButton, &QPushButton::clicked, this, &EmoteMenuFilter::addCategory);
    connect(removeCategoryButton, &QPushButton::clicked, this, &EmoteMenuFilter::removeCategory);
    // connect(searchBox, &QLineEdit::textChanged, this, &EmoteMenuFilter::onSearchTextChanged);

    // setParent(courtroom); YEAH NO, uncomment this later and avoid the white text apocalypse
    // setWindowFlags(Qt::Tool);
}

void EmoteMenuFilter::setupLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this); // maybe change this to something else?
    mainLayout->addWidget(searchBox);
    mainLayout->addWidget(categoryList);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(addCategoryButton);
    mainLayout->addWidget(removeCategoryButton);
    setLayout(mainLayout);
}

void EmoteMenuFilter::addCategory()
{
    bool ok;
    QString category = QInputDialog::getText(this, tr("Add Category"),
                                             tr("Category name:"), QLineEdit::Normal,
                                             "", &ok);
    if (ok && !category.isEmpty()) {
        categoryList->addItem(category);
    }
}

void EmoteMenuFilter::removeCategory()
{
    QListWidgetItem *item = categoryList->currentItem();
    if (item) {
        QString itemText = item->text();
        if (itemText != "Default Emotes" && itemText != "Favorites") {
            delete item;
        } else {
            QMessageBox::warning(this, tr("Remove Category"), tr("Cannot delete this category"));
        }
    } else {
        QMessageBox::warning(this, tr("Remove Category"), tr("No category selected"));
    }
}


void EmoteMenuFilter::resizeEvent(QResizeEvent *event) {
    QDialog::resizeEvent(event);
    if (gridLayout) {
        arrangeButtons();
    } else {
        qWarning() << "GridLayout not yet initialized.";
    }
}

//void EmoteMenuFilter::focusInEvent(QFocusEvent *event) {
//    QDialog::focusInEvent(event);
//    this->setWindowOpacity(1.0); // Fully opaque when in focus
//}

//void EmoteMenuFilter::focusOutEvent(QFocusEvent *event) {
//    QDialog::focusOutEvent(event);
//    this->setWindowOpacity(0.3); // 30% transparent when out of focus
//    // To-Do: Make a Slider to control this
//}

void EmoteMenuFilter::loadButtons(const QStringList &emoteIds) {
    QString charName = courtroom->get_current_char();
    int total_emotes = ao_app->get_emote_number(charName);
    QString emotePath;
    QString emoteName;
    QString emoteId;

    // Button size (width and height)
    int buttonSize = 40;
    
    qDeleteAll(spriteButtons);
    spriteButtons.clear();

    for (int n = 0; n < total_emotes; ++n) {    	
        emoteId = QString::number(n + 1);
        emoteName = ao_app->get_emote_comment(charName, n);
        
        if (!emoteIds.isEmpty() && (!emoteIds.contains(emoteId) || !emoteIds.contains(emoteName))) {
            continue;
        }
        
        emotePath = ao_app->get_image_suffix(ao_app->get_character_path(
            charName, "emotions/button" + QString::number(n + 1) + "_off"));
            
        AOEmoteButton *spriteButton = new AOEmoteButton(this, ao_app, 0, 0, buttonSize, buttonSize);
        spriteButton->set_image(emotePath, "");
        spriteButton->set_id(n + 1);
        spriteButton->set_comment(emoteName);
        spriteButtons.append(spriteButton);
        spriteButton->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(spriteButton, &AOEmoteButton::customContextMenuRequested, [this, spriteButton](const QPoint &pos) {
            QMenu menu;
            QAction *addTagsAction = menu.addAction("Add Tags...");
            connect(addTagsAction, &QAction::triggered, [this, spriteButton]() {
                showTagDialog(spriteButton);
            });
            menu.exec(spriteButton->mapToGlobal(pos));
        });

    }

    arrangeButtons();
}

void EmoteMenuFilter::arrangeButtons() {
    int buttonSize = 40;
    int containerWidth = scrollArea->viewport()->width();
    int spacing = gridLayout->horizontalSpacing();
    int columns = (containerWidth + spacing) / (buttonSize + spacing);


    if (columns == 0) columns = 1;

    int row = 0, col = 0;

    // Clear the layout
    // while (QLayoutItem* item = gridLayout->takeAt(0)) {
    //     delete item->widget();
    //     delete item;
    // }

    for (AOEmoteButton *spriteButton : qAsConst(spriteButtons)) {
        gridLayout->addWidget(spriteButton, row, col);

        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
    
    int totalWidth = columns * (buttonSize + spacing) - spacing; 
    int totalHeight = (row + 1) * (buttonSize + spacing) - spacing; 

    containerWidget->setMinimumSize(totalWidth, totalHeight);
    containerWidget->adjustSize();

    gridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    
    scrollArea->setWidget(containerWidget);
    scrollArea->setWidgetResizable(true);

}

QStringList EmoteMenuFilter::getCategoryList() const {
    QStringList categories;
    for (int i = 0; i < categoryList->count(); ++i) {
        categories << categoryList->item(i)->text();
    }
    return categories;
}

void EmoteMenuFilter::onCategorySelected(QListWidgetItem *item) {
    QString selectedCategory = item->text();
    if (selectedCategory != "Default Emotes") {
        QHash<QString, QStringList> categories = ao_app->read_emote_categories(courtroom->get_current_char());
        QStringList emoteIds = categories.value(selectedCategory);
        loadButtons(emoteIds);
    } else {
        // Load all buttons if no category is selected
        loadButtons();
    }
}

void EmoteMenuFilter::showTagDialog(AOEmoteButton *button) {
    QStringList categories = getCategoryList();

    TagDialog dialog(categories, this);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList selectedTags = dialog.selectedTags();
        QHash<QString, QStringList> tagsToSave;

        for (const QString &tag : selectedTags) {
            tagsToSave[tag].append(button->get_comment());
        }

        saveTagsToFile(tagsToSave);
    }
}

void EmoteMenuFilter::saveTagsToFile(const QHash<QString, QStringList> &tags) {
    QString filePath = ao_app->get_real_path(VPath("characters/" + courtroom->get_current_char() + "/"));
    
    qDebug() << "File path:" << filePath;

    QFile file(filePath + "emote_tags.ini");
    QHash<QString, QStringList> existingTags;
    
    // Read existing tags from the file if it exists
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString currentCategory;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty()) {
                continue;
            }
            if (line.startsWith('[') && line.endsWith(']')) {
                currentCategory = line.mid(1, line.length() - 2);
                existingTags.insert(currentCategory, QStringList());
            } else if (existingTags.contains(currentCategory)) {
                existingTags[currentCategory] << line;
            }
        }
        file.close();
    }
    
    // Append new tags to existing tags
    for (auto it = tags.begin(); it != tags.end(); ++it) {
        existingTags[it.key()] += it.value();
    }

    // Write all tags (existing and new) to the file
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (auto it = existingTags.begin(); it != existingTags.end(); ++it) {
            out << "[" << it.key() << "]\n";
            for (const QString &value : it.value()) {
                out << value << "\n";
            }
            out << "\n";
        }
        file.close();
    }
}

EmoteMenuFilter::~EmoteMenuFilter()
{
    delete categoryList;
    delete searchBox;
    delete scrollArea;
    delete buttonContainer;
    delete gridLayout;
    delete addCategoryButton;
    delete removeCategoryButton;
}

TagDialog::TagDialog(const QStringList &categories, QWidget *parent)
    : QDialog(parent), mainLayout(new QVBoxLayout(this)), groupBox(new QGroupBox("Emote Tags", this)), groupBoxLayout(new QVBoxLayout(groupBox))
{

    for (const QString &category : categories) {
        if (category == "Default Emotes") {
            continue; // Unneeded checkbox
        }
        QCheckBox *checkBox = new QCheckBox(category, groupBox);
        groupBoxLayout->addWidget(checkBox);
        checkboxes.append(checkBox);
    }

    groupBox->setLayout(groupBoxLayout);
    mainLayout->addWidget(groupBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *okButton = new QPushButton("Save", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    connect(okButton, &QPushButton::clicked, this, &TagDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &TagDialog::reject);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

QStringList TagDialog::selectedTags() const
{
    QStringList selected;
    for (QCheckBox *checkBox : checkboxes) {
        if (checkBox->isChecked()) {
            selected.append(checkBox->text());
        }
    }
    return selected;
}
