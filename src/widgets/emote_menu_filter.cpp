#include "widgets/emote_menu_filter.h"
#include "aoemotebutton.h"
#include "aoapplication.h"
#include "courtroom.h"
#include <QResizeEvent>
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

    QWidget *containerWidget = new QWidget();
    containerWidget->setLayout(gridLayout);
    scrollArea->setWidget(containerWidget);
    scrollArea->setWidgetResizable(true);
    
    loadButtons();

    // connect(categoryList, &QListWidget::currentTextChanged, this, &EmoteMenuFilter::onCategoryChanged);
    connect(addCategoryButton, &QPushButton::clicked, this, &EmoteMenuFilter::addCategory);
    connect(removeCategoryButton, &QPushButton::clicked, this, &EmoteMenuFilter::removeCategory);
    // connect(searchBox, &QLineEdit::textChanged, this, &EmoteMenuFilter::onSearchTextChanged);

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
    }

    arrangeButtons();
}

void EmoteMenuFilter::arrangeButtons() {
    int buttonSize = 40;
    int containerWidth = scrollArea->width() - 80;
    int columns = containerWidth / buttonSize;

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
