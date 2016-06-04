/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

#include <QFileDialog>

#include "ui_filterScriptDialog.h"
#include "filterScriptDialog.h"
#include "mainwindow.h"

//using namespace vcg;


FilterScriptDialog::FilterScriptDialog(QWidget * parent)
		:QDialog(parent)
{
	ui = new Ui::scriptDialog();
  FilterScriptDialog::ui->setupUi(this);
  scriptPtr=0;
  connect(ui->okButton, SIGNAL(clicked()), this, SLOT(applyScript()));
  connect(ui->clearScriptButton,SIGNAL(clicked()), this, SLOT(clearScript()));
  connect(ui->saveScriptButton, SIGNAL(clicked()), this, SLOT(saveScript()));
  connect(ui->openScriptButton, SIGNAL(clicked()), this, SLOT(openScript()));
  connect(ui->moveUpButton,SIGNAL(clicked()), this, SLOT(moveSelectedFilterUp()));
  connect(ui->moveDownButton, SIGNAL(clicked()), this, SLOT(moveSelectedFilterDown()));
  connect(ui->removeFilterButton, SIGNAL(clicked()), this, SLOT(removeSelectedFilter()));
  connect(ui->editParameterButton, SIGNAL(clicked()), this, SLOT(editSelectedFilterParameters()));
}

void FilterScriptDialog::setScript(FilterScript *scr)
{
	scriptPtr=scr;
  ui->scriptListWidget->clear();
  
  for( FilterScript::iterator li=scr->filtparlist.begin();li!=scr->filtparlist.end() ;++li)
     ui->scriptListWidget->addItem((*li)->filterName());
}

void FilterScriptDialog::applyScript()
{
	//get the main window
	MainWindow *mainWindow = qobject_cast<MainWindow*>(parentWidget());
		
	if(NULL == mainWindow){
		qDebug() << "problem casting parent of filterscriptdialog to main window";
	}
	accept();
}

void FilterScriptDialog::clearScript()
{
  assert(scriptPtr);
  scriptPtr->filtparlist.clear();
  ui->scriptListWidget->clear();
}

void FilterScriptDialog::saveScript()
{
	QString filt;
	QString mlx("mlx");
	QString xml("xml");
	QString mlxast("(*." + mlx + ")");
	QString xmlast("(*." + xml + ")");
	QString fileName = QFileDialog::getSaveFileName(this,tr("Save Filter Script File"),".", "MeshLab Scripting File Format " + mlxast + ";;Extensible Markup Language " + xmlast,&filt);
	if (fileName.isEmpty())	return;
	
	QFileInfo fi(fileName);
	QString suf(fi.suffix().toLower());
	if (filt.contains(mlxast))
	{
		if(suf != mlx)
			fileName.append("." + mlx);
	}
	else 
	{
		if(suf != xml)
			fileName.append("." + xml);
	}	
	scriptPtr->save(fileName);
}

void FilterScriptDialog::openScript()
{
	QString fileName = QFileDialog::getOpenFileName(this,tr("Open Filter Script File"),".", "MeshLab Scripting File Format (*.mlx);;Extensible Markup Language (*.xml)");
	if (fileName.isEmpty())	return;
  scriptPtr->open(fileName);
  setScript(scriptPtr);
}

void FilterScriptDialog::moveSelectedFilterUp()
{
	//NOTE if this class gets to complex using the QT model/view may be a good idea
	//however, i found it to be over complicated and not too helpful for reording
	
	int currentRow = ui->scriptListWidget->currentRow();
    if ((currentRow == -1) || (currentRow == 0))
        return;

	//move item up in list
	FilterNameParameterValuesPair* pair = scriptPtr->filtparlist.takeAt(currentRow);
    QString filtername = ui->scriptListWidget->currentItem()->text();
	if (pair->filterName() == filtername)
        scriptPtr->filtparlist.insert(currentRow-1, pair);
    else
        throw MLException("Something bad happened: A filter item has been selected in filterScriptDialog being NOT a XML filter or old-fashioned c++ filter.");
	
	//move item up on ui
	QListWidgetItem * item = ui->scriptListWidget->takeItem(currentRow);
	ui->scriptListWidget->insertItem(currentRow-1, item);
	
	//set selected 
	ui->scriptListWidget->setCurrentItem(item);
}

void FilterScriptDialog::moveSelectedFilterDown()
{
    int currentRow = ui->scriptListWidget->currentRow();
    if ((currentRow == -1) || (currentRow == scriptPtr->filtparlist.size() - 1))
        return;

    //move item up in list
    FilterNameParameterValuesPair* pair = scriptPtr->filtparlist.takeAt(currentRow);
    QString filtername = ui->scriptListWidget->currentItem()->text();
    if (pair->filterName() == filtername)
        scriptPtr->filtparlist.insert(currentRow+1, pair);
    else
        throw MLException("Something bad happened: A filter item has been selected in filterScriptDialog being NOT a XML filter or old-fashioned c++ filter.");

    //move item up on ui
    QListWidgetItem * item = ui->scriptListWidget->takeItem(currentRow);
    ui->scriptListWidget->insertItem(currentRow+1, item);

    //set selected 
    ui->scriptListWidget->setCurrentItem(item);
}

void FilterScriptDialog::removeSelectedFilter()
{
	int currentRow = ui->scriptListWidget->currentRow();
    if(currentRow == -1)
        return;

    FilterNameParameterValuesPair* pair = scriptPtr->filtparlist[currentRow];
    QString filtername = ui->scriptListWidget->currentItem()->text();
    if (pair->filterName() == filtername)
    {
        ui->scriptListWidget->takeItem(currentRow);
        scriptPtr->filtparlist.removeAt(currentRow);
    }
    else
        throw MLException("Something bad happened: A filter item has been selected in filterScriptDialog being NOT a XML filter or old-fashioned c++ filter.");
}

void FilterScriptDialog::editSelectedFilterParameters()
{
	//get the selected item
	int currentRow = ui->scriptListWidget->currentRow();	
	
	//return if no row was selected
	if(currentRow == -1)
		return;
	
	QString filtername = ui->scriptListWidget->currentItem()->text();
    FilterNameParameterValuesPair* pair = scriptPtr->filtparlist.at(currentRow);
    if (pair->filterName() == filtername)
        if (!pair->isXMLFilter())
            editOldParameters(currentRow);
        else 
            editXMLParameters(currentRow);
    else
        throw MLException("Something bad happened: A filter item has been selected in filterScriptDialog being NOT a XML filter or old-fashioned c++ filter.");
}

FilterScriptDialog::~FilterScriptDialog()
{
	delete ui;
}

void FilterScriptDialog::editOldParameters( const int row )
{
    if(row == -1)
        return;
    QString actionName = ui->scriptListWidget->currentItem()->text();

    OldFilterNameParameterValuesPair* old = reinterpret_cast<OldFilterNameParameterValuesPair*>(scriptPtr->filtparlist.at(row));
     RichParameterSet oldParameterSet = old->pair.second;
    //get the main window
    MainWindow *mainWindow = qobject_cast<MainWindow*>(parentWidget());

    if(NULL == mainWindow)
        throw MLException("FilterScriptDialog::editXMLParameters : problem casting parent of filterscriptdialog to main window");

    //get a pointer to this action and filter from the main window so we can get the 
    //description of the parameters from the filter
    QAction *action = mainWindow->pluginManager().actionFilterMap[actionName];
    MeshFilterInterface *iFilter = qobject_cast<MeshFilterInterface *>(action->parent());

    if(NULL == iFilter){
        qDebug() << "null filter";
        return;
    }

    //fill the paramter set with all the names and descriptions which are lost in the 
    //filter script
    RichParameterSet newParameterSet;
    iFilter->initParameterSet(action, *(mainWindow->meshDoc()), newParameterSet);

    if(newParameterSet.paramList.size() == oldParameterSet.paramList.size())
    {
        //now set values to be the old values
        RichParameterCopyConstructor cc;
        for(int i = 0; i < newParameterSet.paramList.size(); i++)
        {
            oldParameterSet.paramList[i]->accept(cc);
            newParameterSet.paramList[i]->val = cc.lastCreated->val;
        }	
    } else
        qDebug() << "the size of the given list is not the same as the filter suggests it should be.  your filter script may be out of date, or there is a bug in the filter script class";

    //launch the dialog
    GenericParamDialog parameterDialog(this, &newParameterSet, "Edit Parameters", mainWindow->meshDoc());
    int result = parameterDialog.exec();
    if(result == QDialog::Accepted)
    {
        //keep the changes	
        old->pair.second = newParameterSet;
    }
}

void FilterScriptDialog::editXMLParameters( const int row )
{
    if(row == -1)
        return;
    MainWindow *mainWindow = qobject_cast<MainWindow*>(parentWidget());

    if(NULL == mainWindow)
        throw MLException("FilterScriptDialog::editXMLParameters : problem casting parent of filterscriptdialog to main window");
    
    QString fname = ui->scriptListWidget->currentItem()->text();
    XMLFilterNameParameterValuesPair* xmlparval = reinterpret_cast<XMLFilterNameParameterValuesPair*>(scriptPtr->filtparlist.at(row));

    QMap<QString,MeshLabXMLFilterContainer>::iterator it = mainWindow->PM.stringXMLFilterMap.find(fname);
    if (it == mainWindow->PM.stringXMLFilterMap.end())
    {
        QString err = "FilterScriptDialog::editXMLParameters : filter " + fname + " has not been found.";
        throw MLException(err);
    }

    OldScriptingSystemXMLParamDialog xmldialog(xmlparval->pair.second,it.value(),mainWindow->PM,mainWindow->meshDoc(),mainWindow,this,mainWindow->GLA());
    xmldialog.exec();
}
