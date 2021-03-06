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

#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#include <QProcess>
#include <QCheckBox>
#include <QMessageBox>
#include <QKeyEvent>

#include "const.h"
#include "mainwindow.h"
#include "log_dialog.h"
#include "browser_dialog.h"

namespace SVN_EXTERNALS_DISPOSER
{

Main_Window::Main_Window( 
      const QString & path,
      QWidget *parent    ) 
   : QMainWindow( parent ),
     data_model( nullptr ),
     proxy_filter( nullptr ),
     working_cp_path( path ),
     expanded( false ),
     select_state( 0 ),
     externals_dialog( nullptr )
{
   ui.setupUi(this);


   externals_TV = new Tree_View( this );
   externals_TV->setContextMenuPolicy(Qt::CustomContextMenu);
   externals_TV->header()->setContextMenuPolicy(Qt::CustomContextMenu);

   connect( 
         externals_TV->header(), 
         SIGNAL( customContextMenuRequested( QPoint ) ),
         this,
         SLOT( open_header_menu( QPoint ) ) );


   connect(
         ui.working_copy_path_LE, 
         SIGNAL( textChanged( const QString & ) ),
         this,
         SLOT( select_working_copy( const QString & ) ) );



   QVBoxLayout *layout = new QVBoxLayout;
   layout->addWidget( externals_TV );
   ui.externals_W->setLayout(layout);
   setup_actions( );

   // this loads also working_cp_path
   load_settings( );

   externals_TV->show();

   setWindowTitle( APP_NAME + " v" + APP_VERSION );
}

Main_Window::~Main_Window( )
{
   save_settings( );
   del_external_model( );
}


void Main_Window::del_external_model( void )
{
   externals_TV->setModel( nullptr );

   if( data_model != nullptr )
   {
      delete data_model;
      data_model = nullptr;
   }
}

/// @details
/// The function does the following:
///   - Load the working copy path setting
///   - Call update_tree(), which initializes the model
//      and calls load_column_settings().
void Main_Window::load_settings( void )
{
   QSettings settings;
   select_state = settings.value( SET_SELECT, 0 ).toInt();
   ui.only_externals_CB->setChecked( settings.value( SET_EXTERNAL_FILTER, false ).toBool() );
   restoreGeometry(settings.value( SET_WINDOW_SIZE ).toByteArray());
   if( working_cp_path.size() == 0 )
      working_cp_path = settings.value( SET_CP_PATH, "" ).toString();

   // this causes that update_tree is called.
   ui.working_copy_path_LE->setText( working_cp_path );
}


void Main_Window::save_column_settings( void )
{
   QSettings settings;

   if( data_model == nullptr )
      return;

   settings.beginWriteArray( SET_COLUMN );
   for( int i=0; i < data_model->columnCount(); i++ )
   {
      settings.setArrayIndex( i );

      // save it only, if it is > 0
      if( externals_TV->columnWidth(i)  > 0 )
         settings.setValue( SET_COLUMN_W, externals_TV->columnWidth(i) );
   }
   settings.endArray();
}

void Main_Window::load_column_settings( void )
{
   QSettings settings;

   int size = settings.beginReadArray( SET_COLUMN );
   for( int i=0; i < size; i++ )
   {
      settings.setArrayIndex( i );
      externals_TV->setColumnWidth( i, settings.value( SET_COLUMN_W, 100 ).toInt() );
   }
   settings.endArray();
}

/// @details
/// This function saves the path and calls save_column_settings().
void Main_Window::save_settings( void )
{
   QSettings settings;
   settings.setValue( SET_CP_PATH, working_cp_path );
   settings.setValue( SET_SELECT, select_state );
   settings.setValue( SET_WINDOW_SIZE, saveGeometry() );

   settings.setValue( SET_EXTERNAL_FILTER, ui.only_externals_CB->isChecked() );

   // we only save the column settings, if we have a valid
   // model loaded
   if( data_model != nullptr )
      save_column_settings();
}


void Main_Window::setup_actions( void )
{
   context_menu          = new QMenu( this->externals_TV );
   revision_menu         = new QMenu( this->externals_TV );
   header_menu           = new QMenu( this->externals_TV );

   quit_action           = new QAction(tr( "&Quit" ),                    this);
   open_action           = new QAction(tr( "&Open SVN Working Copy" ),   this);
   reload_action         = new QAction(tr( "&Reload"),                   this);
   discard_action        = new QAction(tr( "&Discard"),                  this);
   save_action           = new QAction(tr( "&Save"),                     this);
   browse_rev_action     = new QAction(tr( "&Browse"),                   this);
   edit_externals_action = new QAction(tr( "&Edit Externals"),           this);
   about_action          = new QAction(tr( "&About"),                    this);


   connect( quit_action,           SIGNAL(triggered()), qApp, SLOT(quit()));
   connect( open_action,           SIGNAL(triggered()), this, SLOT(on_working_copy_browse_PB_clicked()));
   connect( reload_action,         SIGNAL(triggered()), this, SLOT(on_reload_PB_clicked()));
   connect( discard_action,        SIGNAL(triggered()), this, SLOT(on_discard_PB_clicked()));
   connect( save_action,           SIGNAL(triggered()), this, SLOT(on_save_PB_clicked()));
   connect( browse_rev_action,     SIGNAL(triggered()), this, SLOT(browse_rev()));
   connect( edit_externals_action, SIGNAL(triggered()), this, SLOT(edit_externals()));
   connect( about_action,          SIGNAL(triggered()), this, SLOT( about_triggered()));
   connect( externals_TV,          SIGNAL(customContextMenuRequested(const QPoint &)), 
         this, SLOT(open_context_menu(const QPoint &)));

   ui.file_M->addAction( open_action );
   ui.file_M->addAction( reload_action  );
   ui.file_M->addAction( discard_action );
   ui.file_M->addAction( save_action );
   ui.file_M->addAction( quit_action );


   revision_menu->addAction( browse_rev_action );
   revision_menu->addAction( edit_externals_action );
   context_menu->addAction( edit_externals_action  );
   ui.help_M->addAction( about_action );
}


void Main_Window::update_tree( void )
{
   QProcess process;

   del_external_model();

   // only proceed, if it is an exising directory
   if( ! QDir( working_cp_path ).exists() )
      return;


   // ensure, that we are having a copy of a SVN repo
   // by calling svn info <path>
   process.start( SVN_CMD, QStringList() << SVN_INFO << working_cp_path );
   if( process.waitForFinished( SYS_PROCESS_TIMEOUT ) && 
       process.exitCode() == 0 ) // wait 3 seconds
   {
      data_model = new Data_Model( );

      proxy_filter = new Filter(this);
      proxy_filter->set_filter_externals( ui.only_externals_CB->isChecked() );
      proxy_filter->setSourceModel( data_model );



      externals_TV->setModel( proxy_filter );
      data_model->setRootPath( working_cp_path );
      externals_TV->setRootIndex( proxy_filter->mapFromSource( data_model->index( working_cp_path )  ));


      undo_action = data_model->create_undo_action( this, tr("&Undo" ) );
      undo_action->setShortcuts(QKeySequence::Undo);
      redo_action = data_model->create_redo_action( this, tr("&Redo" ) );
      redo_action->setShortcuts(QKeySequence::Redo);

      setup_header_actions( );

      ui.edit_M->clear();
      ui.edit_M->addAction( undo_action    );
      ui.edit_M->addAction( redo_action    );
      load_column_settings();
   }

}


/// @details
///      This also manages the initial visibility of the columns, depending on `select_state`.
void Main_Window::setup_header_actions( void )
{
   if( data_model == nullptr )
      return;

   // clear the older menu
   header_menu->clear();

   // iterate over all columns and add a menu entry
   // we start with 1, because we force keeping the name column
   for( int i=1; i < data_model->columnCount(); i++ )
   {
      QWidgetAction * sel_action = new QWidgetAction( header_menu );
      QCheckBox *box             = new QCheckBox( header_menu );

      box->setText( data_model->headerData( i, Qt::Horizontal, Qt::DisplayRole  ).toString() );
      sel_action->setDefaultWidget( box );
      header_menu->addAction( sel_action );


      if( !( select_state & ( 1<<i ) ) )
      {
         externals_TV->setColumnHidden( i, false );
         box->setCheckState( Qt::Checked );
      }
      else
      {
         externals_TV->setColumnHidden( i, true );
         box->setCheckState( Qt::Unchecked );
      }

      // connect to a lambda, which handles the individual action
      connect( box, &QCheckBox::stateChanged,
            [=](int state)
            { 
               if( state )
               {
                  QSettings settings;
                  select_state &= ~( 1 << i );
                  externals_TV->setColumnHidden( i, false );

                  load_column_settings();
               }
               else
               {
                  select_state |=  ( 1 << i );
                  externals_TV->setColumnHidden( i, true );
               }
            } );
   }

}


void Main_Window::on_working_copy_browse_PB_clicked( void )
{

   QString dir = QFileDialog::getExistingDirectory(this, 
         tr("Open Directory"),
         ui.working_copy_path_LE->text(),
         QFileDialog::ShowDirsOnly
         | QFileDialog::DontResolveSymlinks);
   if( dir.size() > 0 )
      ui.working_copy_path_LE->setText( dir );
}

void Main_Window::select_working_copy( const QString & dir )
{
   if( dir.size() > 0 )
   {
      working_cp_path = dir;
      update_tree();
   }

}


void Main_Window::on_expand_PB_clicked( void )
{
   if( expanded == true )
   {
      expanded = false;
      externals_TV->collapseAll(); 
      ui.expand_PB->setText( tr("Expand all") );
   }
   else
   {
      expanded = true;
      externals_TV->expandAll(); 
      ui.expand_PB->setText( tr("Collapse all") );
   }
}


void Main_Window::on_reload_PB_clicked( void )
{
   save_settings();
   update_tree();
}


void Main_Window::on_save_PB_clicked( void )
{
   if( data_model != nullptr )
      data_model->save_externals( );
   save_settings();
   update_tree();
}


void Main_Window::on_discard_PB_clicked( void )
{
   if( data_model != nullptr )
      data_model->restore( );
   save_settings();
}

void Main_Window::about_triggered( void )
{
   QMessageBox::about( this, tr("About"), 
         ABOUT_THIS_SOFTWARE );
}




void Main_Window::open_context_menu(const QPoint &point)
{
   QModelIndex index = externals_TV->indexAt(point);
   if( ! index.isValid() )
      return;


   if(  (   index.column() == 5 ||
            index.column() == 6 || 
            index.column() == 7 ) 
       && data_model->is_external( proxy_filter->mapToSource( index ) )
      )
   {
      last_context_index = index;
      revision_menu->exec( externals_TV->mapToGlobal(point));
   }
   else if( data_model->is_directory( proxy_filter->mapToSource( index ) ) )
   {
      last_context_index = index;
      context_menu->exec( externals_TV->mapToGlobal(point));
   }
}

void Main_Window::open_header_menu(const QPoint & point )
{
   header_menu->exec( externals_TV->mapToGlobal( point ) );
}


void Main_Window::browse_rev( void )
{
   if( data_model   == nullptr || 
       proxy_filter == nullptr )
   {
      return;
   }

   QModelIndex index = proxy_filter->mapToSource( last_context_index );

   if( index.column() == 5 )
   {
      T_SP_External e = data_model->get_external( index );

      QString url = e->url.toString();
      QString revision = "HEAD";
      int     revision_from = 0;

      if( e->peg_revision.toString().size() > 0 )
      {
         revision      = e->peg_revision.toString();
         revision_from = 6;
      }
      else if( e->operative_revision.toString().size() > 0 )
      {
         revision      = e->operative_revision.toString();
         revision_from = 7;
      }

      Browser_Dialog *d = new Browser_Dialog( url, revision );

      if( d->load() )
      {
         d->exec();
         if( d->result() )
         {
            data_model->setData( index, d->get_url(), Qt::EditRole  );
            QString new_revision = d->get_revision();
            if( revision_from == 6 )
            {
               auto i = index.sibling( index.row(), 6 );
               if( i.isValid() )
                  data_model->setData( i, new_revision, Qt::EditRole );
            } 
            else if( revision_from == 7 )
            {
               auto i = index.sibling( index.row(), 7 );
               if( i.isValid() )
                  data_model->setData( i, new_revision, Qt::EditRole );
            }
            else if( revision_from == 0 && new_revision.toUpper() != QString("HEAD" ) )
            {
               auto i = index.sibling( index.row(), 6 );
               if( i.isValid() )
                  data_model->setData( i, new_revision, Qt::EditRole );
            }


         }
      }
      else
      {
         QMessageBox * m = new QMessageBox( );
         m->setText( tr("Failed to load the SVN repository browser" ) );
         m->setDetailedText( "URL: " + url );
         m->exec();

      }
   }
   else if( index.column() == 6 || index.column() == 7 )
   {
      QString path = data_model->filePath( index );
      Log_Dialog *d = new Log_Dialog( path, this );
      if( d->load() )
      {
         auto current_revision = data_model->data( index, Qt::DisplayRole ).toString();
         if( current_revision.length() > 0 )
            d->select_revision( current_revision );

         d->exec();
         if( d->result() )
         {
            data_model->setData( index, d->get_revision(), Qt::EditRole  );
         }
      }
      else
      {
         QMessageBox * m = new QMessageBox( );
         m->setText( tr("Failed to load the SVN revision log" ) );
         m->setDetailedText( "Path: " + path );
         m->exec();
      }

   }


}


void Main_Window::edit_externals( void )
{
   if( data_model == nullptr )
      return;

   QString path = data_model->filePath(
         proxy_filter->mapToSource( last_context_index ) );

   // note: we don't set the parent widget and
   // delete the dialog manually
   externals_dialog = new Externals_Dialog(
         path,
         data_model
         );
   externals_dialog->installEventFilter( this );

   auto index = data_model->get_undo_index( );

   externals_dialog->exec();

   if( !externals_dialog->result() )
   {
      // discard changes
      data_model->set_undo_index( index );
   }

   // delete the dialog 
   delete externals_dialog;
   externals_dialog = nullptr;
}


Main_Window::operator QString()
{
   QString s;
   QTextStream ts(&s);
   ts  << "Main_Window (" << reinterpret_cast< void* >( this ) << "): ";
   return s;
}


bool Main_Window::eventFilter( QObject *obj, QEvent * e )
{
   if( obj == externals_dialog && nullptr != externals_dialog )
   {
      if( e->type() == QEvent::KeyPress )
      {
         QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>( e );
         if( static_cast< unsigned >( QKeySequence( QKeySequence::Redo )[0] ) 
               ==  ( keyEvent->key() | keyEvent->modifiers() ) )
         {
            redo_action->trigger();
            externals_dialog->setup_table();
            return true;
         }
         if( static_cast< unsigned >( QKeySequence( QKeySequence::Undo )[0] ) 
               ==  ( keyEvent->key() | keyEvent->modifiers() ) )
         {
            undo_action->trigger();
            externals_dialog->setup_table();
            return true;
         }
      }
   }

   return false;
}


void Main_Window::on_only_externals_CB_stateChanged( int state )
{
   if( proxy_filter )
   {
      proxy_filter->set_filter_externals( state == 2 );
      // this is required in order to refresh the tree-view
      proxy_filter->setFilterRegExp( QString("") );
   }
}


}; // namespace SVN_EXTERNALS_DISPOSER

// vim: filetype=cpp et ts=3 sw=3 sts=3

