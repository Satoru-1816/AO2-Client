#include "widgets/emote_menu_filter.h"
#include "aoemotebutton.h"
#include "aoapplication.h"
#include "courtroom.h"
#include <QResizeEvent>
#include <QShowEvent>
#include <algorithm>
#include <QFocusEvent>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QDir>
#include <QMouseEvent>

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
    setupCategories();

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

void EmoteMenuFilter::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    arrangeButtons();
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
        
        if (!emoteIds.isEmpty() && (!emoteIds.contains(emoteId) && !emoteIds.contains(emoteName))) {
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
        spriteButton->setFixedSize(buttonSize, buttonSize);

        connect(spriteButton, &AOEmoteButton::clicked, this, [this, spriteButton]() {
            onButtonClicked(spriteButton);
        });

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
        spriteButton->setFixedSize(buttonSize, buttonSize); // Ensure fixed size

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

void EmoteMenuFilter::setupCategories() {
    QString currentChar = courtroom->get_current_char();
    QMap<QString, QStringList> categories = ao_app->read_emote_categories(currentChar);

    qDebug() << (categories);
    for (const QString &category : categories.keys()) {
        categoryList->addItem(category);
    }
}

void EmoteMenuFilter::onCategorySelected(QListWidgetItem *item) {
	selectedButtons.clear();
    QString selectedCategory = item->text();
    if (selectedCategory != "Default Emotes") {
        QMap<QString, QStringList> categories = ao_app->read_emote_categories(courtroom->get_current_char());
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

        // Only save tags in group if more than one button
        // is selected
        if (selectedButtons.size() > 1) {
            for (AOEmoteButton *selectedButton : selectedButtons) {
                for (const QString &tag : selectedTags) {
                    tagsToSave[tag].append(selectedButton->get_comment());
                }
            }
        } else {
            for (const QString &tag : selectedTags) {
                tagsToSave[tag].append(button->get_comment());
            }
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

    // Sort categories and write to file
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        
        // Get sorted category keys
        QStringList sortedCategories = existingTags.keys();
        std::sort(sortedCategories.begin(), sortedCategories.end());

        for (const QString &category : sortedCategories) {
            out << "[" << category << "]\n";
            for (const QString &value : existingTags[category]) {
                out << value << "\n";
            }
            out << "\n";
        }
        file.close();
    }
}

void EmoteMenuFilter::onButtonClicked(AOEmoteButton *button) {
    bool shiftPressed = QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
    bool ctrlPressed = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);

    if (shiftPressed && !selectedButtons.isEmpty()) {
        // Handle Shift + Click (select range)
        int startIndex = spriteButtons.indexOf(selectedButtons.last());
        int endIndex = spriteButtons.indexOf(button);

        if (startIndex > endIndex) {
            qSwap(startIndex, endIndex);
        }
        
        // Select all buttons in the chosen range
        for (int i = startIndex; i <= endIndex; ++i) {
            AOEmoteButton *btn = spriteButtons[i];
            if (!selectedButtons.contains(btn)) {
                selectedButtons.append(btn);
                updateButtonSelection(btn, true);
            }
        }
    } else if (ctrlPressed) {
        // Alternate selection
        if (selectedButtons.contains(button)) {
            updateButtonSelection(button, false);  // Deselect
            selectedButtons.removeOne(button);
        } else {
            updateButtonSelection(button, true);  // Selecct
            selectedButtons.append(button);
        }
    } else {
        // Clear previous selection if Ctrl is not being held
        for (AOEmoteButton *btn : selectedButtons) {
            updateButtonSelection(btn, false);
        }
        selectedButtons.clear();

        // Select a new button
        updateButtonSelection(button, true);
        selectedButtons.append(button);
    }
}

void EmoteMenuFilter::updateButtonSelection(AOEmoteButton *button, bool isSelected) {
	QString state;
	if (isSelected) {
	  state = "_on";
	} else {
      state = "_off";
	}
	  
    QString baseImagePath = ao_app->get_image_suffix(ao_app->get_character_path(courtroom->get_current_char(), 
	                                                  "emotions/button" + QString::number(button->get_id()) + state));

    button->set_image(baseImagePath, "");
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
    : QDialog(parent), mainLayout(new QVBoxLayout(this)), groupBox(new QGroupBox("Emote Tags", this)), 
      groupBoxLayout(new QVBoxLayout), scrollArea(new QScrollArea(this)), scrollWidget(new QWidget(this))
{
    // Set up the scroll widget and its layout
    scrollWidget->setLayout(scrollLayout);
    
    // Set up the scroll area
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);

    // Add checkboxes to the scroll layout
    for (const QString &category : categories) {
        if (category == "Default Emotes") {
            continue; // Unneeded checkbox
        }
        QCheckBox *checkBox = new QCheckBox(category, scrollWidget);
        scrollLayout->addWidget(checkBox);
        checkboxes.append(checkBox);
    }

    // Add a scroll area to the group box
    groupBox->setLayout(groupBoxLayout);
    groupBoxLayout->addWidget(scrollArea);

    // Add group box to the main layout
    mainLayout->addWidget(groupBox);

    // Add buttons
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
