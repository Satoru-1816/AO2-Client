#include "widgets/emote_menu_filter.h"
#include "aoemotebutton.h"
#include "aoapplication.h"
#include <QVBoxLayout>
#include <QInputDialog>
#include <QMessageBox>

EmoteMenuFilter::EmoteMenuFilter(QDialog *parent, AOApplication *p_ao_app)
    : QDialog(parent), ao_app(p_ao_app), emote_filter_ui(new Ui::EmoteMenuFilter)
{
    emote_filter_ui->setupUi(this);

    categoryList = new QListWidget(this);
    categoryList->addItem("Default Emotes");
    categoryList->addItem("Favorites");
    
    searchBox = new QLineEdit(this);
    scrollArea = new QScrollArea(this);
    buttonContainer = new QWidget(this);
    gridLayout = new QGridLayout(buttonContainer);
    addCategory = new QPushButton("Add Category", this);
    removeCategory = new QPushButton("Remove Category", this);

    scrollArea->setWidget(buttonContainer);
    scrollArea->setWidgetResizable(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(searchBox);
    mainLayout->addWidget(categoryList);
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(addCategory);
    mainLayout->addWidget(removeCategory);
    setLayout(mainLayout);

    // connect(categoryList, &QListWidget::currentTextChanged, this, &EmoteMenuFilter::onCategoryChanged);
    connect(addCategory, &QPushButton::clicked, this, &EmoteMenuFilter::addCategory);
    connect(removeCategory, &QPushButton::clicked, this, &EmoteMenuFilter::removeCategory);
    // connect(searchBox, &QLineEdit::textChanged, this, &EmoteMenuFilter::onSearchTextChanged);
}

EmoteMenuFilter::~EmoteMenuFilter()
{
    delete emote_filter_ui;
}

void EmoteMenuFilter::onCategoryChanged()
{
    return;
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
