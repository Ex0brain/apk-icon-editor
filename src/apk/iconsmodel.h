#ifndef ICONSMODEL_H
#define ICONSMODEL_H

#include "icon.h"
#include <QAbstractListModel>

class IconsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit IconsModel(QObject *parent = 0) : QAbstractListModel(parent) {}
    ~IconsModel();

    void add(const QString &filename);
    void clone(Icon *source);
    void save();

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

private:
    QList<Icon *> icons;
};

#endif // ICONSMODEL_H
