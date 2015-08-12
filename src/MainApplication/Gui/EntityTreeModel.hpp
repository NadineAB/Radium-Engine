/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef RADIUMENGINE_ENTITYTREEMODEL_HPP
#define RADIUMENGINE_ENTITYTREEMODEL_HPP

#include <vector>

#include <QAbstractItemModel>
#include <QList>
#include <QVariant>

namespace Ra
{
    namespace Engine
    {
        class Entity;
    }
}

namespace Ra
{
    namespace Gui
    {
        class EntityTreeItem;
    }
}

namespace Ra
{
    namespace Gui
    {

        class EntityTreeModel : public QAbstractItemModel
        {
            Q_OBJECT

        public:
            explicit EntityTreeModel( const QStringList& headers,
                                      QObject* parent = nullptr );
            ~EntityTreeModel();

            QVariant data( const QModelIndex& index, int role ) const override;
            QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

            QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
            QModelIndex parent( const QModelIndex& child ) const override;

            int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
            int columnCount( const QModelIndex& parent = QModelIndex() ) const override;

            Qt::ItemFlags flags( const QModelIndex& index ) const override;
            bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;
            bool setHeaderData( int section, Qt::Orientation orientation, const QVariant& value,
                                int role = Qt::EditRole ) override;

            bool insertColumns( int column, int count, const QModelIndex& parent = QModelIndex() ) override;
            bool removeColumns( int position, int columns, const QModelIndex& parent = QModelIndex() ) override;
            bool insertRows( int position, int rows, const QModelIndex& parent = QModelIndex() ) override;
            bool removeRows( int position, int rows, const QModelIndex& parent = QModelIndex() ) override;

        public slots:
            void entitiesUpdated( const std::vector<Engine::Entity*>& entities );
            void handleRename( const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& );
            void handleSelect( const QModelIndex& index );

        protected:

        private:
            EntityTreeItem* getItem( const QModelIndex& index ) const;

            void insertComponents( Engine::Entity* entity, EntityTreeItem* parent );

        private:
            EntityTreeItem* m_rootItem;
        };

    } // namespace Gui
} // namespace Ra

#endif // RADIUMENGINE_ENTITYTREEMODEL_HPP
