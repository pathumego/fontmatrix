//
// C++ Implementation: fmrepair
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "fmrepair.h"
#include "typotek.h"
#include "fontitem.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

FmRepair::FmRepair(QWidget *parent)
	:QDialog(parent)
{
	setupUi(this);
	fillLists();
	doConnect();
	
}

FmRepair::~ FmRepair()
{
}

void FmRepair::doConnect()
{
	connect(closeButton,SIGNAL(clicked( )),this,SLOT(close()));
}

void FmRepair::fillLists()
{
	fillDeadLink();
	fillActNotLinked();
	fillDeactLinked();
}

void FmRepair::fillDeadLink()
{
	typotek *t = typotek::getInstance();
	QDir md(t->getManagedDir());
	md.setFilter( QDir::Files );
	QFileInfoList list = md.entryInfoList();
	for(int i(0); i < list.count(); ++i)
	{
		if(list[i].isSymLink())
		{
			if( !QFileInfo(list[i].symLinkTarget()).exists() )
			{
				QListWidgetItem *lit = new QListWidgetItem(list[i].absoluteFilePath());
				lit->setCheckState(Qt::Unchecked);
				lit->setToolTip(list[i].absoluteFilePath());
				deadList->addItem(lit);
			}
		}
	}
	
}

void FmRepair::fillActNotLinked()
{
	typotek *t = typotek::getInstance();
	
	QList<FontItem*> flist(t->getAllFonts());
	QStringList activated;
	for(int i(0); i < flist.count();++i)
	{
		if(!flist[i]->isLocked() && flist[i]->isActivated())
			activated << flist[i]->path();
	}
	
	QStringList linked;
	QDir md(t->getManagedDir());
	md.setFilter( QDir::Files );
	QFileInfoList list = md.entryInfoList();
	for(int i(0); i < list.count(); ++i)
	{
		if(list[i].isSymLink())
		{
			if(  QFileInfo(list[i].symLinkTarget()).exists()  )
			{
				qDebug()<< "Inserting "<<list[i].symLinkTarget();
				linked << list[i].symLinkTarget();
			}
			else
			{
				qDebug()<<list[i].filePath()<<" is a broken symlink";
			}
		}
		else
		{
			qDebug()<<list[i].filePath() << " is not a symlink";
		}
	}
	
	for(int i(0); i < activated.count(); ++i)
	{
		if(!linked.contains(activated[i]))
		{
			QListWidgetItem *lit = new QListWidgetItem(activated[i]);
			lit->setCheckState(Qt::Unchecked);
			lit->setToolTip(activated[i]);
			actNotLinkList->addItem(lit);
		}
	}
}

void FmRepair::fillDeactLinked()
{
	typotek *t = typotek::getInstance();
	
	QList<FontItem*> flist(t->getAllFonts());
	QStringList activated;
	for(int i(0); i < flist.count();++i)
	{
		if(!flist[i]->isLocked() && flist[i]->isActivated())
			activated << flist[i]->path();
	}
	
	QStringList linked;
	QDir md(t->getManagedDir());
	md.setFilter( QDir::Files );
	QFileInfoList list = md.entryInfoList();
	for(int i(0); i < list.count(); ++i)
	{
		if(list[i].isSymLink())
		{
			if(  !QFileInfo(list[i].symLinkTarget()).exists()  )
			{
				linked << list[i].symLinkTarget();
			}
		}
	}
	
	for(int i(0); i < linked.count(); ++i)
	{
		if(!activated.contains(activated[i]))
		{
			QListWidgetItem *lit = new QListWidgetItem(linked[i]);
			lit->setCheckState(Qt::Unchecked);
			lit->setToolTip(linked[i]);
			deactLinkList->addItem(lit);
		}
	}
}

void FmRepair::slotSelAllDead()
{
	for(int i(0); i < deadList->count(); ++i)
	{
		deadList->item(i)->setCheckState(Qt::Checked);
	}
}

void FmRepair::slotRemoveDead()
{
	QList<int> toBeCleared;
	for(int i(0); i < deadList->count(); ++i)
	{
		if(deadList->item(i)->checkState() == Qt::Checked)
		{
			QFile f(deadList->item(i)->text());
			f.remove();
			toBeCleared << i;
		}
	}
	for(int i( toBeCleared.count() - 1 ); i >= 0 ; --i)
	{
		deadList->removeItemWidget(deadList->item(toBeCleared[i]));
	}
}

void FmRepair::slotSelAllActNotLinked()
{
	for(int i(0); i < actNotLinkList->count(); ++i)
	{
		actNotLinkList->item(i)->setCheckState(Qt::Checked);
	}
}

void FmRepair::slotRelinkActNotLinked()
{
	typotek *t = typotek::getInstance();
	QList<int> toBeCleared;
	for(int i(0); i < actNotLinkList->count() ; ++i)
	{
		if(actNotLinkList->item(i)->checkState() == Qt::Checked)
		{
			QFile f(actNotLinkList->item(i)->text());
			QFileInfo fi(f);
			f.link( t->getManagedDir() + fi.fileName() );
			FontItem *font = 0;
			if(font = t->getFont(actNotLinkList->item(i)->text()))
			{
				if(!font->afm().isEmpty())
				{
					QFile af(font->afm());
					QFileInfo afi(af);
					af.link( t->getManagedDir() + afi.fileName() );
				}
			}
			toBeCleared << i;
		}
	}
	for(int i( toBeCleared.count() - 1 ); i >= 0 ; --i)
	{
		actNotLinkList->removeItemWidget(actNotLinkList->item(toBeCleared[i]));
	}
}

void FmRepair::slotDeactivateActNotLinked()
{
	typotek *t = typotek::getInstance();
	QList<int> toBeCleared;
	for(int i(0); i < actNotLinkList->count() ; ++i)
	{
		if(actNotLinkList->item(i)->checkState() == Qt::Checked)
		{
			FontItem *font = 0;
			if(font = t->getFont(actNotLinkList->item(i)->text()))
			{
				font->setActivated(false);
			}
			toBeCleared << i;
		}
	}
	for(int i( toBeCleared.count() - 1 ); i >= 0 ; --i)
	{
		actNotLinkList->removeItemWidget(actNotLinkList->item(toBeCleared[i]));
	}
}

void FmRepair::slotSelAllDeactLinked()
{
	for(int i(0); i < deactLinkList->count(); ++i)
	{
		deactLinkList->item(i)->setCheckState(Qt::Checked);
	}
}

void FmRepair::slotDelinkDeactLinked()
{
	typotek *t = typotek::getInstance();
	QList<int> toBeCleared;
	for(int i(0); i < deactLinkList->count(); ++i)
	{
		if(deactLinkList->item(i)->checkState() == Qt::Checked)
		{
			QFileInfo fi(deactLinkList->item(i)->text());
			QFile f(t->getManagedDir() + fi.fileName());
			toBeCleared << i;
		}
	}
	for(int i( toBeCleared.count() - 1 ); i >= 0 ; --i)
	{
		deactLinkList->removeItemWidget(deactLinkList->item(toBeCleared[i]));
	}
}

void FmRepair::slotActivateDeactLinked()
{
	typotek *t = typotek::getInstance();
	QList<int> toBeCleared;
	
	for(int i(0); i < deactLinkList->count(); ++i)
	{
		if(deactLinkList->item(i)->checkState() == Qt::Checked)
		{
			FontItem *font = 0;
			if(font = t->getFont(deactLinkList->item(i)->text()))
			{
				font->setActivated(true);
			}
			toBeCleared << i;
		}
	}
	
	for(int i( toBeCleared.count() - 1 ); i >= 0 ; --i)
	{
		deactLinkList->removeItemWidget(deactLinkList->item(toBeCleared[i]));
	}
}



