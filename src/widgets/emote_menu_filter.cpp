#include "widgets/emote_menu_filter.h"
#include "aoemotebutton.h"
#include "aoapplication.h"
#include "courtroom.h"
#include <QResizeEvent>
#include <QFocusEvent>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>

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

    // connect(categoryList, &QListWidget::currentTextChanged, this, &EmoteMenuFilter::onCategoryChanged);
    connect(addCategoryButton, &QPushButton::clicked, this, &EmoteMenuFilter::addCategory);
    connect(removeCategoryButton, &QPushButton::clicked, this, &EmoteMenuFilter::removeCategory);
    // connect(searchBox, &QLineEdit::textChanged, this, &EmoteMenuFilter::onSearchTextChanged);

    setWindowFlags(Qt::Tool);

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

void EmoteMenuFilter::loadButtons() {
    int total_emotes = ao_app->get_emote_number(courtroom->get_current_char());

    // Button size (width and height)
    int buttonSize = 40;
    
    // qDeleteAll(spriteButtons);
    // spriteButtons.clear();

    for (int n = 0; n < total_emotes; ++n) {
        QString emotePath = ao_app->get_image_suffix(ao_app->get_character_path(
            courtroom->get_current_char(), "emotions/button" + QString::number(n + 1) + "_off"));
        
        AOEmoteButton *spriteButton = new AOEmoteButton(this, ao_app, 0, 0, buttonSize, buttonSize);
        spriteButton->set_image(emotePath, "");

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

void EmoteMenuFilter::showTagDialog(AOEmoteButton *button) {
    QStringList categories = getCategoryList();
    TagDialog dialog(categories, this);
    if (dialog.exec() == QDialog::Accepted) {
        QStringList selectedTags;
        // Do something with it
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
    : QDialog(parent), mainLayout(new QVBoxLayout(this)), groupBox(new QGroupBox("Emote Tags", this)), groupBoxLayout(new QVBoxLayout(groupBox)) {

    for (const QString &category : categories) {
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
