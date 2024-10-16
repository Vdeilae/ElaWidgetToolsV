#include "ElaSuggestBox.h"

#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListView>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

#include "ElaIcon.h"
#include "ElaLineEdit.h"
#include "ElaShadowWidget.h"
#include "ElaSuggestDelegate.h"
#include "ElaSuggestModel.h"
#include "ElaSuggestView.h"
#include "private/ElaSuggestBoxPrivate.h"

Q_PROPERTY_CREATE_Q_CPP(ElaSuggestBox, int, BorderRadius)
Q_PROPERTY_CREATE_Q_CPP(ElaSuggestBox, Qt::CaseSensitivity, CaseSensitivity)
ElaSuggestBox::ElaSuggestBox(QWidget* parent)
    : QWidget{parent}, d_ptr(new ElaSuggestBoxPrivate())
{
    Q_D(ElaSuggestBox);
    setFixedSize(280, 35);
    d->q_ptr = this;
    d->_pBorderRadius = 6;
    d->_pCaseSensitivity = Qt::CaseInsensitive;
    d->_searchEdit = new ElaLineEdit(this);
    d->_searchEdit->setFixedHeight(35);
    d->_searchEdit->setPlaceholderText("查找功能");
    d->_searchEdit->setClearButtonEnabled(true);
    QAction* searchAction = new QAction(ElaIcon::getInstance()->getElaIcon(ElaIconType::Fingerprint), "Search", this);
    d->_searchEdit->addAction(searchAction, QLineEdit::TrailingPosition);
    connect(searchAction, &QAction::triggered, this, [=](bool checked) { qDebug() << "Search"; });
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(d->_searchEdit);
    d->_shadowWidget = new ElaShadowWidget(window());
    QVBoxLayout* shadowLayout = new QVBoxLayout(d->_shadowWidget);
    shadowLayout->setContentsMargins(1, 0, 1, 0);
    d->_searchView = new ElaSuggestView(d->_shadowWidget);
    d->_shadowWidget->resize(280, 300);
    shadowLayout->addWidget(d->_searchView);

    d->_searchModel = new ElaSuggestModel(this);
    d->_searchDelegate = new ElaSuggestDelegate(this);
    d->_searchView->setModel(d->_searchModel);
    d->_searchView->setItemDelegate(d->_searchDelegate);
    d->_shadowWidget->hide();
    connect(d->_searchEdit, &ElaLineEdit::textEdited, d, &ElaSuggestBoxPrivate::onSearchEditTextEdit);
    connect(d->_searchEdit, &ElaLineEdit::focusIn, d, &ElaSuggestBoxPrivate::onSearchEditTextEdit);
    connect(d->_searchView, &ElaSuggestView::clicked, d, &ElaSuggestBoxPrivate::onSearchViewClicked);

    // 焦点事件
    connect(d->_searchEdit, &ElaLineEdit::wmFocusOut, this, [d]() {
        d->_startCloseAnimation();
    });
}

ElaSuggestBox::~ElaSuggestBox()
{
}

void ElaSuggestBox::setPlaceholderText(const QString& placeholderText)
{
    Q_D(ElaSuggestBox);
    d->_searchEdit->setPlaceholderText(placeholderText);
}

void ElaSuggestBox::addSuggestion(const QString& suggestText, const QVariantMap& suggestData)
{
    Q_D(ElaSuggestBox);
    ElaSuggestion* suggest = new ElaSuggestion(this);
    suggest->setSuggestText(suggestText);
    suggest->setSuggestData(suggestData);
    d->_suggestionVector.append(suggest);
}

void ElaSuggestBox::addSuggestion(ElaIconType icon, const QString& suggestText, const QVariantMap& suggestData)
{
    Q_D(ElaSuggestBox);
    ElaSuggestion* suggest = new ElaSuggestion(this);
    suggest->setElaIcon(icon);
    suggest->setSuggestText(suggestText);
    suggest->setSuggestData(suggestData);
    d->_suggestionVector.append(suggest);
}

void ElaSuggestBox::removeSuggestion(const QString& suggestText)
{
    Q_D(ElaSuggestBox);
    foreach (auto suggest, d->_suggestionVector)
    {
        if (suggest->getSuggestText() == suggestText)
        {
            d->_suggestionVector.removeOne(suggest);
            suggest->deleteLater();
        }
    }
}

void ElaSuggestBox::removeSuggestion(int index)
{
    Q_D(ElaSuggestBox);
    if (index >= d->_suggestionVector.count())
    {
        return;
    }
    ElaSuggestion* suggest = d->_suggestionVector[index];
    d->_suggestionVector.removeOne(suggest);
    suggest->deleteLater();
}
