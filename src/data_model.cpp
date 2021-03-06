// SVN-Externals-Disposer 
//
// Copyright (C) 2016  Christian Haettich [feddischson]
//
// This program is free software; you can redistribute it 
// and/or modify it under the terms of the GNU General Public 
// License as published by the Free Software Foundation; 
// either version 3 of the License, or (at your option) 
// any later version.
//
// This program is distributed in the hope that it will 
// be useful, but WITHOUT ANY WARRANTY; without even the 
// implied warranty of MERCHANTABILITY or FITNESS FOR A 
// PARTICULAR PURPOSE. See the GNU General Public License 
// for more details.
//
// You should have received a copy of the GNU General 
// Public License along with this program; if not, see 
// <http://www.gnu.org/licenses/>. 
//
#include <QDebug>
#include <QFileInfoList>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QDomDocument>
#include <QBrush>
#include <QMessageBox>

#include "const.h"
#include "data_model.h"
#include "external_command.h"

namespace SVN_EXTERNALS_DISPOSER
{


Data_Model::Data_Model( QObject *parent )
   : QFileSystemModel( parent ), 
     root_path( "" ),
     undo_stack( this )
{
   header  << GUI_HEAD_TARGET
           << GUI_HEAD_EXTERNAL  
           << GUI_HEAD_PEG      
           << GUI_HEAD_OPERATIVE
           << GUI_HEAD_PATH;
}

QAction * Data_Model::create_undo_action(QObject * parent, const QString & prefix ) const
{
   return undo_stack.createUndoAction( parent, prefix );
}

QAction * Data_Model::create_redo_action(QObject * parent, const QString & prefix ) const
{
   return undo_stack.createRedoAction( parent, prefix );
}


T_SP_External Data_Model::get_external( const QModelIndex & index ) const
{
   if( index.isValid() )
      return get_external( QFileSystemModel::filePath( index ) );
   else
      return T_SP_External();
}


T_SP_External Data_Model::get_external( const QString & path ) const
{
   if( external_map.contains( path ) )
      return external_map.value( path );
   else
      return T_SP_External();
}



QVariant Data_Model::data(const QModelIndex &index, int role) const 
{
   if (!index.isValid())
       QFileSystemModel::data( index, role );

   if( role == Qt::BackgroundRole )
   {

      T_SP_External external = get_external( index );
      if( !external )
         return QFileSystemModel::data( index, role );

      QString path = QFileSystemModel::filePath( index );

      if( is_external_modified( path ) )
         return QBrush( COL_MODIFIED );
      else
         return QFileSystemModel::data( index, role );
   }

   if( index.column() >= 4)
   {

      if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();
      else
      {
         T_SP_External external = get_external( index );

         if( !external )
            return QVariant();

         switch( index.column() - 4 )
         {
            case  0: return external->local_path;
            case  1: return external->url;
            case  2: return external->peg_revision;
            case  3: return external->operative_revision;
            case  4: return external->storage_path;
            default: return QVariant();
         }

      }
   }

   return QFileSystemModel::data( index, role );
}


void Data_Model::change_external( 
      const QString & path,
      QVariant new_value,
      int index,
      bool push_to_undostack
      ) 
{
   QVariant old_value;
   bool modified = false;


   if( !external_map.contains( path ) )
      return;

   // get the external
   T_SP_External external = external_map.value( path );


   switch( index )
   {
      case  0: 
         {
            if( external->local_path != new_value )
            {
               old_value = external->local_path;
               external->local_path = new_value;
               modified = true;
            }
            break;
         }
      case  1: 
         {
            if( external->url != new_value )
            {
               old_value = external->url;
               external->url = new_value;
               modified = true;
            }
            break;
         }
      case  2: 
         {
            if( external->peg_revision != new_value )
            {
               old_value = external->peg_revision;
               external->peg_revision = new_value;
               modified = true;
            }
            break;
         }
      case  3: 
         {
            if( external->operative_revision != new_value )
            {
               old_value = external->operative_revision;
               external->operative_revision = new_value;
               modified = true;
            }
            break;
         }
      case  4: 
         {
            if( external->storage_path != new_value )
            {
               old_value = external->storage_path;
               external->storage_path = new_value;
               modified = true;
            }
            break;
         }
      default: 
         {
            modified = false; 
            break;
         }
   }

   if( modified )
   {
      external->modified = true;

      if( push_to_undostack )
      {
         undo_stack.push( new External_Command(
                  this,
                  path,
                  index,
                  new_value,
                  old_value ) );
      }
      refresh_path( path );
   }
}

void Data_Model::refresh_path( const QString & path )
{
   auto index = QFileSystemModel::index( path, 0 );
   emit( dataChanged( index, 
         QFileSystemModel::index( path , columnCount( index )-1 ) ) );
}

bool Data_Model::setData(const QModelIndex & index, const QVariant & value, int role)
{
   int column = index.column() - 4;
   if( column >= 0 )
   {

      if( role == Qt::EditRole )
      {
         QString path = QFileSystemModel::filePath( index );
         change_external( 
               path, 
               value,
               column
               );

         return true;
      }
      else
         return false;
   }
   else
      return QFileSystemModel::setData( index, value, role );
}


Qt::ItemFlags Data_Model::flags(const QModelIndex &index) const 
{
   if (!index.isValid())
      return QFileSystemModel::flags( index );

   if( index.column() >= 4)
      return QAbstractItemModel::flags(index)  | Qt::ItemIsSelectable | Qt::ItemIsEditable;
   else
      return QFileSystemModel::flags( index );
}


QVariant Data_Model::headerData(int section, Qt::Orientation orientation,
                     int role ) const 
{
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 4 )
      return header[ section-4 ];
   else if( section >= 4 )
      return header[ section-4 ];
   else
      return QFileSystemModel::headerData( section, orientation, role );
}


int Data_Model::columnCount( const QModelIndex & ) const 
{
   return QFileSystemModel::columnCount() + 5;
}


void Data_Model::setup_model_data( 
      const QString                  & path )
{
   QDir dir( path ); 

   // read the external property of 'path'
   auto files = read_externals( path );

   // loop over all entries 
   foreach(const auto &file, files) 
   {
      QDir child_path( file );  

      if( child_path.exists() ) 
         // recursive call to add all sub-paths
         setup_model_data( child_path.absolutePath() );
   }
}

bool Data_Model::save_externals( void )
{
   bool result = true;
   foreach( auto  path, property_map.uniqueKeys() )
   {
      auto i = property_map.find( path );
      QString property = "";
      bool modified = false;
      while (i != property_map.end() && i.key() == path)
      {
         if( i.value()->modified )
            modified = true;

         property = *(i.value()) + "\n" + property;
         i++;
      }

      if( !modified )
         continue;


      // write the svn:external via svn propset
      QProcess process;
      process.start( SVN_CMD, QStringList()
            << SVN_PROPSET
            << SVN_EXTERNALS
            << SVN_DEPTH
            << SVN_FILES
            << property
            << path
            );
      // wait until the process is done
      // and check the result
      if( process.waitForFinished( SYS_PROCESS_TIMEOUT ) && 
          process.exitCode() == 0 )
      {
         // nothing todo here
      }
      else
      {
         qDebug() << "Failed to set properties";
         qDebug() << process.readAllStandardError();
         result = false;
      }
   }
   return result;
}


QList<QString> Data_Model::read_externals( 
      const QString & path )
{

   // start a svn propget svn:externals process
   QProcess process;
   process.start( SVN_CMD, QStringList() 
         << SVN_PROPGET
         << SVN_EXTERNALS
         << SVN_XML
         << SVN_RECURSIVE     
         << path );


   // The resulting list for the recursive call
   QList< QString > result;

   // wait until the process is done
   // and check the result
   if( process.waitForFinished( SYS_PROCESS_TIMEOUT ) && 
       process.exitCode() == 0 )
   {

      // Parse the XML result
      QDomDocument doc;
      if ( !doc.setContent( process.readAllStandardOutput() ) )
      {
         qDebug() << "Failed to read svn:externals property";
         return result;
      }

      QDomNodeList targets = doc.elementsByTagName( XML_NAME_TARGET );

      // and iterate over all svn:externals properties.
      // If we have a valid entry, add the entry to the external_map.
      for (int i = 0; i < targets.size(); i++) {

         QDomNode target   = targets.item(i);
         QDomElement value = target.firstChildElement( XML_NAME_PROPERTY );

         if( !value.isNull()          && 
              target.hasAttributes( ) && 
              target.attributes().contains( XML_NAME_PATH ) )
         {
            QDomElement e    = target.toElement();
            QString     path = QDir::cleanPath( e.attribute( XML_NAME_PATH, "" ) );
            QString     prop = value.text();

            auto tmp = extract_externals( path, prop );
            result += tmp;
         }
      }
   }
   return result;
}


QList< QString > Data_Model::extract_externals( 
      const QString       & path,
      const QString       & property )
{

   // The resulting list for the recursive call
   QList< QString > result;

   foreach (
         const QString &str, 
         property.split( SVN_EXTERNAL_SEP, QString::SkipEmptyParts )  ) 
   {

      T_SP_External external( new External( str, path ) );

      if( !external )
         continue;

      QDir local_dir( external->local_path.toString() );


      // try it as an absolute path
      if(    QDir( external->local_path.toString() ).exists() 
          || QFile(external->local_path.toString() ).exists() )
      {
         QString tmp_path = external->local_path.toString();
         external_map.insert( tmp_path, external );
         property_map.insertMulti( path, external );
         result.append( tmp_path );
      }
      else 
      // try it as a relative path
      {
         QString abs_path = QDir( path ).filePath( external->local_path.toString() );
         if(    QDir ( abs_path ).exists() 
             || QFile( abs_path ).exists() )
         {
            external_map.insert( abs_path, external );
            property_map.insertMulti( path, external );
            result.append( abs_path );
         }
         else
         {
            QMessageBox * m = new QMessageBox( );
            m->setDetailedText(  "Can not find " + 
                              external->local_path.toString() +
                              " in " + 
                              path    + 
                              ".\nPlease run svn update." );
            m->setText( "Missing folder!" );
            m->exec();
         }
      }
   }
   return result;
}


bool Data_Model::is_external_modified( const QString & path ) const
{
   if( !external_map.contains( path ) )
   {
      qDebug() << "Fatal error: " << path << " does not exist";
      return false;
   }
   return external_map.value( path ) != external_map_backup.value( path );
}

QModelIndex Data_Model::setRootPath( const QString & new_path )
{
   root_path = new_path;
   external_map.clear();
   property_map.clear();
   setup_model_data( new_path );
   backup();
   return QFileSystemModel::setRootPath( new_path );
}


void Data_Model::transfer( 
   QMultiMap< QString, T_SP_External > * from_property_map,
   QHash< QString, T_SP_External >     * to_external_map,
   QMultiMap< QString, T_SP_External > * to_property_map )
{
   to_external_map->clear();
   to_property_map->clear();
   auto i = from_property_map->constBegin();
   while( i != from_property_map->constEnd() )
   {
      // create a copy of the external (via copy-ctor)
      T_SP_External backup( new External( *i.value( ) ) );

      QString abs_path = QDir( i.key() ).filePath( backup->local_path.toString() );

      // insert the backup into our back-storages
      to_external_map->insert( abs_path, backup);
      to_property_map->insert( i.key(), backup );
      i++;
   }
}


void Data_Model::restore( void )
{
   transfer( &property_map_backup, 
             &external_map,
             &property_map );

   // go through all externals and refresh them
   foreach(const auto & path, external_map.keys() )
      refresh_path( path );
}

bool Data_Model::is_external( const QModelIndex & i )
{
   QString path = QFileSystemModel::filePath( i );
   if( external_map.contains( path ) )
      return true;
   else
      return false;
}

bool Data_Model::is_directory( const QModelIndex & i )
{
   QDir dir( QFileSystemModel::filePath( i ) ); 
   return dir.exists();
}



QList< QString > Data_Model::get_externals_targets( const QString path )
{
   QList< QString > result;
   auto i = property_map.find( path );

   // iterate over all entries for this path and add a copy into our result hash
   while (i != property_map.end() && i.key() == path)
   {
      QString target_path = i.value()->local_path.toString();
      QString abs_path = QDir( i.key() ).filePath( target_path );
      result.append( abs_path );
      i++;
   }
   return result;
}


void Data_Model::backup( void )
{
   external_map_backup.clear();
   property_map_backup.clear();
   auto i = property_map.constBegin();
   while( i != property_map.constEnd() )
   {
      // create a copy of the external (via copy-ctor)
      T_SP_External backup( new External( *i.value( ) ) );

      QString abs_path = QDir( i.key() ).filePath( backup->local_path.toString() );

      // insert the backup into our back-storages
      external_map_backup.insert( abs_path, backup);
      property_map_backup.insert( i.key(), backup );
      i++;
   }

}

/// @details
///   Just forwards the call to the internal `undo_stack`.
int Data_Model::get_undo_index( void )
{
   return undo_stack.index();
}

///   Just forwards the call to the internal `undo_stack`.
void Data_Model::set_undo_index( int index )
{
   undo_stack.setIndex( index );
}


bool Data_Model::is_external_related( const QModelIndex & index )
{
   QString path  = QFileSystemModel::filePath( index );
   QString rpath = rootPath();


   // if the path is above the root-path, return true
   if( abs_path_contains( path, rpath ) )
      return true;
   else
   {
      // search if the path is above an external
      foreach(const auto &epath, external_map.keys() )
      {
         if( abs_path_contains( path, epath ) )
            return true;
      }
   }
   return false;
}

/// @brief Returns true, if child is below parent
bool  Data_Model::abs_path_contains( const QString & parent, const QString & child )
{
   if( parent.size() > child.size() )
      return false;

   QStringRef beginning( &child, 0, parent.size() );
   return (beginning == parent );
}


bool Data_Model::is_above_root_path( const QString & path )
{
   QString rpath = rootPath();
   return abs_path_contains( path, rpath );
}


}; // namespace SVN_EXTERNALS_DISPOSER

// vim: filetype=cpp et ts=3 sw=3 sts=3

