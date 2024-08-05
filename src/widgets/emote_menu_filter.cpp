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

    containerWidget = new QWidget(this);
    containerWidget->setLayout(gridLayout);
    
    scrollArea->setWidget(containerWidget);
    scrollArea->setWidgetResizable(true);

    loadButtons();

    connect(categoryList, &QListWidget::itemSelectionChanged, this, &EmoteMenuFilter::onCategorySelected);
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
        delete item;
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

void EmoteMenuFilter::loadButtons(const QStringList &emoteIds = QStringList()) {
    int total_emotes = ao_app->get_emote_number(courtroom->get_current_char());

    // Button size (width and height)
    int buttonSize = 40;
    
    qDeleteAll(spriteButtons);
    spriteButtons.clear();

    for (int n = 0; n < total_emotes; ++n) {    	
        QString emoteId = QString::number(n + 1);
        if (!emoteIds.isEmpty() && !emoteIds.contains(emoteId)) {
            continue;
        }
        
        QString emotePath = ao_app->get_image_suffix(ao_app->get_character_path(
            courtroom->get_current_char(), "emotions/button" + QString::number(n + 1) + "_off"));
        
        AOEmoteButton *spriteButton = new AOEmoteButton(this, ao_app, 0, 0, buttonSize, buttonSize);
        spriteButton->set_image(emotePath, "");
        spriteButton->set_id(n + 1);
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

void EmoteMenuFilter::onCategorySelected() {
    QString selectedCategory = item->text();
    if (selectedCategory != "Default Emotes") {
        QString category = selectedItem->text();
        QHash<QString, QStringList> categories = ao_app->read_emote_categories(courtroom->get_current_char());
        QStringList emoteIds = categories.value(category);
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
            tagsToSave[tag].append(QString::number(button->get_id()));
        }

        saveTagsToFile(tagsToSave);
    }
}

void EmoteMenuFilter::saveTagsToFile(const QHash<QString, QStringList> &tags) {
    QString filePath = ao_app->get_real_path
	                   (ao_app->get_character_path
			           (courtroom->get_current_char(), "emote_tags.ini"));
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    QHashIterator<QString, QStringList> i(tags);

    while (i.hasNext()) {
        i.next();
        out << "[" << i.key() << "]\n";
        for (const QString &value : i.value()) {
            out << value << "\n";
        }
        out << "\n";
    }

    file.close();
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
