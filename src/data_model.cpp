// SVN-Externals-Disposer 
//
// Copyright (C) 2015  Christian Haettich [feddischson]
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
#include "const.h"
#include "data_model.h"

namespace SVN_EXTERNALS_DISPOSER
{


Data_Model::Data_Model( QObject *parent )
   : QFileSystemModel( parent ),
     external_matcher( EXTERNAL_REGEX ),
     old_external_matcher( OLD_EXTERNAL_REGEX )
{
   header  << GUI_HEAD_TARGET
           << GUI_HEAD_EXTERNAL  
           << GUI_HEAD_PEG      
           << GUI_HEAD_OPERATIVE
           << GUI_HEAD_PATH;
}

T_SP_External Data_Model::get_external( const QModelIndex & index ) const
{
   QString path = QFileSystemModel::filePath( index );
   return external_map.value( path );
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

      if( external->modified )
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

         //qDebug() << "is an external " << QFileSystemModel::filePath( index );
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

bool Data_Model::setData(const QModelIndex & index, const QVariant & value, int role)
{
   if( index.column() >= 4 )
   {

      if( role == Qt::EditRole )
      {

         T_SP_External external = get_external( index );
         if( !external )
            return false;

         switch( index.column() - 4 )
         {
            case  0: 
               {
                  if( external->local_path != value )
                  {
                     external->local_path = value;
                     external->modified = true;
                     break;
                  }
               }
            case  1: 
               {
                  if( external->url != value )
                  {
                     external->url = value;
                     external->modified = true; 
                     break;
                  }
               }
            case  2: 
               {
                  if( external->peg_revision != value )
                  {
                     external->peg_revision = value;
                     external->modified = true; 
                     break;
                  }
               }
            case  3: 
               {
                  if( external->operative_revision != value )
                  {
                     external->operative_revision = value;
                     external->modified = true; 
                     break;
                  }
               }
            case  4: 
               {
                  if( external->storage_path != value )
                  {
                     external->storage_path = value;
                     external->modified = true; 
                     break;
                  }
               }
            default: return false;
         }
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
   if( !read_externals( path ) )
      qDebug() << "Failed to read svn:externals property";

   // get all directories without . and ..
   QFileInfoList files = dir.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot );

   // loop over all entries 
   foreach(const QFileInfo &file, files) 
   {
      QString child_path = file.absoluteFilePath();  

      if( file.isDir() ) 
         // recursive call to add all sub-paths
         setup_model_data( child_path );
   }
}

bool Data_Model::save_externals( void )
{
   bool result = true;
   foreach( auto  path, property_map.uniqueKeys() )
   {
      auto i = property_map.find( path );
      QString property = "";
      while (i != property_map.end() && i.key() == path)
      {
         property = *(i.value()) + "\n" + property;
         i++;
      }

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


bool Data_Model::read_externals( 
      const QString & path )
{

   // start a svn propget svn:externals process
   QProcess process;
   process.start( SVN_CMD, QStringList() 
         << SVN_PROPGET
         << SVN_EXTERNALS
         << SVN_XML
         << SVN_DEPTH
         << SVN_FILES
         << path );


   // wait until the process is done
   // and check the result
   if( process.waitForFinished( SYS_PROCESS_TIMEOUT ) && 
       process.exitCode() == 0 )
   {

      // Parse the XML result
      QDomDocument doc;
      if ( !doc.setContent( process.readAllStandardOutput() ) )
         return false;
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
            QString     path = e.attribute( XML_NAME_PATH, "" );
            QString     prop = value.text();

            extract_externals( path, prop );
         }
      }
   }
   else
      return false;

   return true;
}


void Data_Model::extract_externals( 
      const QString       & path,
      const QString       & property )
{

   foreach (
         const QString &str, 
         property.split( SVN_EXTERNAL_SEP, QString::SkipEmptyParts )  ) 
   {

      T_SP_External external =  parse_external( str, path );

      if( !external )
         continue;

      QDir local_dir( external->local_path.toString() );

      // try it as an absolute path
      if( QDir( external->local_path.toString() ).exists() )
      {
         external_map.insert( external->local_path.toString(), external );
         property_map.insertMulti( path, external );
      }
      else 
      // try it as a relative path
      {
         QString abs_path = QDir( path ).filePath( external->local_path.toString() );
         QDir abs_dir( abs_path );
         if( abs_dir.exists() )
         {
            external_map.insert( abs_path, external );
            property_map.insertMulti( path, external );
         }
         else
            qDebug() << "cannot find " << external->local_path.toString() << "in " << path;
      }


   }
}


T_SP_External Data_Model::parse_external( const QString & entry, const QString & path )
{
   int pos;

   // first: try the old syntax
   pos = old_external_matcher.indexIn( entry );
   if( pos > -1 )
   {
      return T_SP_External( new External(  
            old_external_matcher.cap(1), // local path
            old_external_matcher.cap(4), // url
            QString(""),                 // peg-revision
            old_external_matcher.cap(3), // operative-revision
            path ) );
   }
   else
   {
      // second: try the current syntax
      pos = external_matcher.indexIn( entry );
      if( pos > -1 )
      {
         return T_SP_External( new External(
            external_matcher.cap(6), // local path
            external_matcher.cap(3), // url
            external_matcher.cap(5), // peg
            external_matcher.cap(2), // operative
            path ) );
      }
   }

   // returns an invalid External struct
   return T_SP_External( new External() );
}


QModelIndex Data_Model::setRootPath( const QString & new_path )
{
   external_map.clear();
   property_map.clear();
   setup_model_data( new_path );
   return QFileSystemModel::setRootPath( new_path );
}

}; // namespace SVN_EXTERNALS_DISPOSER

// vim: filetype=cpp et ts=3 sw=3 sts=3

