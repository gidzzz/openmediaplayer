#ifndef HEADERAWAREPROXYMODEL_H
#define HEADERAWAREPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "includes.h"

class HeaderAwareProxyModel : public QSortFilterProxyModel
{
public:
    HeaderAwareProxyModel(QObject *parent) : QSortFilterProxyModel(parent) { }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        return this->sourceModel()->index(sourceRow, 0, sourceParent).data(UserRoleHeader).toBool() ||
               QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }
};

#endif // HEADERAWAREPROXYMODEL_H
