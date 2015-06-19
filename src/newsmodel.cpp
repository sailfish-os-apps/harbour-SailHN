/*
  The MIT License (MIT)

  Copyright (c) 2015 Andrea Scarpino <me@andreascarpino.it>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "newsmodel.h"

const static int MAX_ITEMS = 30;

NewsModel::NewsModel(QObject *parent) :
    QAbstractListModel(parent)
  , api(new HackerNewsAPI(this))
{
}

NewsModel::~NewsModel()
{
    delete api;
}

QHash<int, QByteArray> NewsModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[UrlRole] = "url";
    return roles;
}

void NewsModel::loadNewStories()
{
    beginResetModel();
    backing.clear();
    endResetModel();
    api->getNewStories();
    connect(api, SIGNAL(multipleStoriesFetched(QVariantList)), this, SLOT(loadItems(QVariantList)));
}

void NewsModel::loadTopStories()
{
    beginResetModel();
    backing.clear();
    endResetModel();
    api->getTopStories();
    connect(api, SIGNAL(multipleStoriesFetched(QList<int>)), this, SLOT(loadItems(QList<int>)));
}

QVariant NewsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (roleNames().contains(role)) {
        return backing[index.row()].value(roleNames().value(role));
    }

    return QVariant();
}

void NewsModel::onItemFetched(QVariantMap item)
{
    const QString idRole = roleNames().value(IdRole);
    const int pos = order.indexOf(item.value(idRole).toInt());
    beginInsertRows(QModelIndex(), pos, pos + 1);
    backing.insert(pos, item);
    endInsertRows();
}

void NewsModel::loadItems(QList<int> ids)
{
    connect(api, SIGNAL(itemFetched(QVariantMap)), this, SLOT(onItemFetched(QVariantMap)));

    QList<int> limited = ids.mid(0, MAX_ITEMS);
    Q_FOREACH (const int id, limited) {
        order.append(id);
        api->getItem(id);
    }
}
