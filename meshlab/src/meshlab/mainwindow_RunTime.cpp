/****************************************************************************
* MeshLab                                                           o o     *
* An extendible mesh processor                                    o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005, 2006                                          \/)\/    *
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


#include <QtGui>
#include <QToolBar>
#include <QProgressBar>
#include <QHttp>
#include <QDesktopServices>

#include "mainwindow.h"
#include "plugindialog.h"
#include "filterScriptDialog.h"
#include "customDialog.h"
#include "saveSnapshotDialog.h"
#include "ui_aboutDialog.h"
#include "savemaskexporter.h"
#include "alnParser.h"


#include <wrap/io_trimesh/io_mask.h>
#include <vcg/complex/trimesh/update/normal.h>
#include <vcg/complex/trimesh/update/bounding.h>
#include <vcg/complex/trimesh/clean.h>
#include "../common/scriptinterface.h"
#include "../common/meshlabdocumentxml.h"

using namespace std;
using namespace vcg;

void MainWindow::updateRecentFileActions()
{
	QSettings settings;
	QStringList files = settings.value("recentFileList").toStringList();
	int numRecentFiles = qMin(files.size(), (int)MAXRECENTFILES);
	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
		recentFileActs[i]->setText(text);
		recentFileActs[i]->setData(files[i]);
		recentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MAXRECENTFILES; ++j)	recentFileActs[j]->setVisible(false);
	separatorAct->setVisible(numRecentFiles > 0);
}

// creates the standard plugin window
void MainWindow::createStdPluginWnd()
{
	//checks if a MeshlabStdDialog is already open and closes it
	if (stddialog!=0){
		stddialog->close();
		delete stddialog;
	}
	stddialog = new MeshlabStdDialog(this);
	stddialog->setAllowedAreas (    Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea,stddialog);
	//stddialog->setAttribute(Qt::WA_DeleteOnClose,true);
	stddialog->setFloating(true);
	stddialog->hide();
  connect(GLA(),SIGNAL(glareaClosed()),this,SLOT(updateStdDialog()));
  connect(GLA(),SIGNAL(glareaClosed()),stddialog,SLOT(close()));
}

void MainWindow::createXMLStdPluginWnd()
{
	//checks if a MeshlabStdDialog is already open and closes it
	if (xmldialog!=0){
		xmldialog->close();
		delete xmldialog;
	}
	xmldialog = new MeshLabXMLStdDialog(this);
	connect(xmldialog,SIGNAL(dialogEvaluateExpression(const Expression&,Value**)),this,SLOT(evaluateExpression(const Expression&,Value**)),Qt::DirectConnection);
	xmldialog->setAllowedAreas (    Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea,xmldialog);
	//stddialog->setAttribute(Qt::WA_DeleteOnClose,true);
	xmldialog->setFloating(true);
	xmldialog->hide();
	connect(GLA(),SIGNAL(glareaClosed()),xmldialog,SLOT(close()));
}


// When we switch the current model (and we change the active window)
// we have to close the stddialog.
// this one is called when user switch current window.
void MainWindow::updateStdDialog()
{
	if(stddialog!=0){
		if(GLA()!=0){
      if(stddialog->curModel != meshDoc()->mm()){
				stddialog->curgla=0; // invalidate the curgla member that is no more valid.
				stddialog->close();
			}
		}
	}
}

void MainWindow::updateCustomSettings()
{
	emit dispatchCustomSettings(currentGlobalParams);
}

void MainWindow::updateWindowMenu()
{
	windowsMenu->clear();
	windowsMenu->addAction(closeAllAct);
	windowsMenu->addSeparator();
	windowsMenu->addAction(windowsTileAct);
	windowsMenu->addAction(windowsCascadeAct);
	windowsMenu->addAction(windowsNextAct);
	windowsNextAct->setEnabled(mdiarea-> subWindowList().size()>1);

	windowsMenu->addSeparator();

	
	if((mdiarea-> subWindowList().size()>0)){
		// Split/Unsplit SUBmenu
		splitModeMenu = windowsMenu->addMenu(tr("&Split current view"));

		splitModeMenu->addAction(setSplitHAct);
		splitModeMenu->addAction(setSplitVAct);

		windowsMenu->addAction(setUnsplitAct);

		// Link act
		windowsMenu->addAction(linkViewersAct);

		// View From SUBmenu
		viewFromMenu = windowsMenu->addMenu(tr("&View from"));
    foreach(QAction *ac, viewFromGroupAct->actions())
      viewFromMenu->addAction(ac);

		// View From File act
		windowsMenu->addAction(viewFromFileAct);
    windowsMenu->addAction(viewFromMeshAct);
    windowsMenu->addAction(viewFromRasterAct);

		// Copy and paste shot acts
		windowsMenu->addAction(copyShotToClipboardAct);
		windowsMenu->addAction(pasteShotFromClipboardAct);

		//Enabling the actions
    MultiViewer_Container *mvc = currentViewContainer();
		if(mvc)
		{
			setUnsplitAct->setEnabled(mvc->viewerCounter()>1);
			Viewer* current = mvc->currentView();
      if(current)
      {
        setSplitHAct->setEnabled(current->size().height()/2 > current->minimumSizeHint().height());
        setSplitVAct->setEnabled(current->size().width()/2 > current->minimumSizeHint().width());

        linkViewersAct->setEnabled(currentViewContainer()->viewerCounter()>1);
        if(currentViewContainer()->viewerCounter()==1)
          linkViewersAct->setChecked(false);

        windowsMenu->addSeparator();
      }
		}
	}

	QList<QMdiSubWindow*> windows = mdiarea->subWindowList();

	if(windows.size() > 0)
			windowsMenu->addSeparator();

	int i=0;
	foreach(QWidget *w,windows)
	{
		QString text = tr("&%1. %2").arg(i+1).arg(QFileInfo(w->windowTitle()).fileName());
		QAction *action  = windowsMenu->addAction(text);
		action->setCheckable(true);
		action->setChecked(w == mdiarea->currentSubWindow());
		// Connect the signal to activate the selected window
		connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
		windowMapper->setMapping(action, w);
		++i;
	}
}

void MainWindow::setColorMode(QAction *qa)
{
  if(qa->text() == tr("&None"))					GLA()->setColorMode(GLW::CMNone);
  if(qa->text() == tr("Per &Mesh"))     GLA()->setColorMode(GLW::CMPerMesh);
  if(qa->text() == tr("Per &Vertex"))		GLA()->setColorMode(GLW::CMPerVert);
  if(qa->text() == tr("Per &Face"))			GLA()->setColorMode(GLW::CMPerFace);
}

void MainWindow::updateMenus()
{

	bool active = (bool) !mdiarea->subWindowList().empty() && mdiarea->currentSubWindow();
  if(active && meshDoc()->meshList.empty())
  {
    filterMenuCreate->setEnabled(true);
    return;
  }
  openInAct->setEnabled(active);
	closeAct->setEnabled(active);
	reloadAct->setEnabled(active);
	saveAct->setEnabled(active);
	saveAsAct->setEnabled(active);
	saveSnapshotAct->setEnabled(active);
	filterMenu->setEnabled(active && !filterMenu->actions().isEmpty());
	editMenu->setEnabled(active && !editMenu->actions().isEmpty());
	renderMenu->setEnabled(active);
	fullScreenAct->setEnabled(active);
	trackBallMenu->setEnabled(active);
	logMenu->setEnabled(active);
	windowsMenu->setEnabled(active);
	preferencesMenu->setEnabled(active);

	renderToolBar->setEnabled(active);

	showToolbarRenderAct->setChecked(renderToolBar->isVisible());
	showToolbarStandardAct->setChecked(mainToolBar->isVisible());
  if(active && GLA()){
				const RenderMode &rm=GLA()->getCurrentRenderMode();
				switch (rm.drawMode) {
					case GLW::DMBox:				renderBboxAct->setChecked(true);                break;
					case GLW::DMPoints:			renderModePointsAct->setChecked(true);      		break;
					case GLW::DMWire: 			renderModeWireAct->setChecked(true);      			break;
					case GLW::DMFlat:				renderModeFlatAct->setChecked(true);    				break;
					case GLW::DMSmooth:			renderModeSmoothAct->setChecked(true);  				break;
					case GLW::DMFlatWire:		renderModeFlatLinesAct->setChecked(true);				break;
					case GLW::DMHidden:			renderModeHiddenLinesAct->setChecked(true);			break;
				default: break;
				}
        colorModePerFaceAct->setEnabled(HasPerFaceColor(meshDoc()->mm()->cm));
				switch (rm.colorMode)
				{
                                        case GLW::CMNone:	colorModeNoneAct->setChecked(true);	      break;
                                        case GLW::CMPerMesh:	colorModePerMeshAct->setChecked(true);	      break;
					case GLW::CMPerVert:	colorModePerVertexAct->setChecked(true);  break;
					case GLW::CMPerFace:	colorModePerFaceAct->setChecked(true);    break;
					default: break;
				}

				lastFilterAct->setEnabled(false);
				if(GLA()->getLastAppliedFilter() != NULL)
				{
					lastFilterAct->setText(QString("Apply filter ") + GLA()->getLastAppliedFilter()->text());
					lastFilterAct->setEnabled(true);
				}
				else
				{
					lastFilterAct->setText(QString("Apply filter "));
				}


				// Management of the editing toolbar
				// when you enter in a editing mode you can toggle between editing
				// and camera moving by esc;
				// you exit from editing mode by pressing again the editing button
				// When you are in a editing mode all the other editing are disabled.

				foreach (QAction *a,PM.editActionList)
						 {
								a->setChecked(false);
								a->setEnabled( GLA()->getCurrentEditAction() == NULL );
						 }

				suspendEditModeAct->setChecked(GLA()->suspendedEditor);
				suspendEditModeAct->setDisabled(GLA()->getCurrentEditAction() == NULL);

				if(GLA()->getCurrentEditAction())
						{
								GLA()->getCurrentEditAction()->setChecked(! GLA()->suspendedEditor);
								GLA()->getCurrentEditAction()->setEnabled(true);
						}

				showInfoPaneAct->setChecked(GLA()->infoAreaVisible);
				showTrackBallAct->setChecked(GLA()->isTrackBallVisible());
				backFaceCullAct->setChecked(GLA()->getCurrentRenderMode().backFaceCull);
        renderModeTextureAct->setEnabled(meshDoc()->mm() && !meshDoc()->mm()->cm.textures.empty());
				renderModeTextureAct->setChecked(GLA()->getCurrentRenderMode().textureMode != GLW::TMNone);

				setLightAct->setIcon(rm.lighting ? QIcon(":/images/lighton.png") : QIcon(":/images/lightoff.png") );
				setLightAct->setChecked(rm.lighting);

				setFancyLightingAct->setChecked(rm.fancyLighting);
				setDoubleLightingAct->setChecked(rm.doubleSideLighting);
        setSelectFaceRenderingAct->setChecked(rm.selectedFace);
        setSelectVertRenderingAct->setChecked(rm.selectedVert);

				// Check only the active decorations
				foreach (QAction *a,      PM.decoratorActionList){a->setChecked(false);a->setEnabled(true);}
				foreach (QAction *a,   GLA()->iDecoratorsList){a->setChecked(true);}

	} // if active
	else
	{
		foreach (QAction *a,PM.editActionList)
		{
				a->setEnabled(false);
		}
		foreach (QAction *a,PM.decoratorActionList)
				a->setEnabled(false);

	}

	if(GLA())
	{
		showLayerDlgAct->setChecked(layerDialog->isVisible());
		//if(GLA()->layerDialog->isVisible())
		layerDialog->updateTable();
		layerDialog->updateLog(*GLA()->log);
		layerDialog->updateDecoratorParsView();
	}
	else
	{
		if(layerDialog->isVisible())
		{
			layerDialog->updateTable();
			layerDialog->updateDecoratorParsView();
		}
	}
}

void MainWindow::setSplit(QAction *qa)
{
  MultiViewer_Container *mvc = currentViewContainer();
	if(mvc) 
	{
		GLArea *glwClone=new GLArea(mvc, &currentGlobalParams);	
		bool isRaster = GLA()->isRaster();
		if(qa->text() == tr("&Horizontally"))	
			mvc->addView(glwClone, Qt::Vertical);
		else if(qa->text() == tr("&Vertically"))
			mvc->addView(glwClone, Qt::Horizontal);

		//The loading of the raster must be here
		if(isRaster){
			glwClone->setIsRaster(true);
			if(GLA()->meshDoc->rm()->id()>=0)
				glwClone->loadRaster(GLA()->meshDoc->rm()->id());
		}

		updateMenus();

		glwClone->resetTrackBall();

		glwClone->update();
	}

}

void MainWindow::setUnsplit()
{
  MultiViewer_Container *mvc = currentViewContainer();
	if(mvc) 
	{
		assert(mvc->viewerCounter() >1);
		
		mvc->removeView(mvc->currentView()->getId());

		updateMenus();
	}
}

//set the split/unsplit menu that appears right clicking on a splitter's handle 
void MainWindow::setHandleMenu(QPoint point, Qt::Orientation orientation, QSplitter *origin){
    MultiViewer_Container *mvc =  currentViewContainer();
		int epsilon =10;
		splitMenu->clear();
		unSplitMenu->clear();
		//the viewer to split/unsplit is chosen through picking

		//Vertical handle allows to split horizontally
		if(orientation == Qt::Vertical)
		{
			splitUpAct->setData(point);
			splitDownAct->setData(point);

			//check if the viewer on the top is splittable according to its size
			int pickingId = mvc->getViewerByPicking(QPoint(point.x(), point.y()-epsilon));
			if(pickingId>=0)
				splitUpAct->setEnabled(mvc->getViewer(pickingId)->size().width()/2 > mvc->getViewer(pickingId)->minimumSizeHint().width());

			//the viewer on top can be closed only if the splitter over the handle that orginated the event has one child 
			bool unSplittabilityUp = true;
			Splitter * upSplitter = qobject_cast<Splitter *>(origin->widget(0));
			if(upSplitter)
				unSplittabilityUp = !(upSplitter->count()>1);
			unsplitUpAct->setEnabled(unSplittabilityUp);
			
			//check if the viewer below is splittable according to its size
			pickingId = mvc->getViewerByPicking(QPoint(point.x(), point.y()+epsilon));
			if(pickingId>=0)
				splitDownAct->setEnabled(mvc->getViewer(pickingId)->size().width()/2 > mvc->getViewer(pickingId)->minimumSizeHint().width());

			//the viewer below can be closed only if the splitter ounder the handle that orginated the event has one child 
			bool unSplittabilityDown = true;
			Splitter * downSplitter = qobject_cast<Splitter *>(origin->widget(1));
			if(downSplitter)
				unSplittabilityDown = !(downSplitter->count()>1);
			unsplitDownAct->setEnabled(unSplittabilityDown);

			splitMenu->addAction(splitUpAct);
			splitMenu->addAction(splitDownAct);

			unsplitUpAct->setData(point);
			unsplitDownAct->setData(point);

			unSplitMenu->addAction(unsplitUpAct);
			unSplitMenu->addAction(unsplitDownAct);
		}
		//Horizontal handle allows to split vertically
		else if (orientation == Qt::Horizontal)
		{
			splitRightAct->setData(point);
			splitLeftAct->setData(point);

			//check if the viewer on the right is splittable according to its size
			int pickingId =mvc->getViewerByPicking(QPoint(point.x()+epsilon, point.y()));
			if(pickingId>=0)
				splitRightAct->setEnabled(mvc->getViewer(pickingId)->size().height()/2 > mvc->getViewer(pickingId)->minimumSizeHint().height());
			
			//the viewer on the rigth can be closed only if the splitter on the right the handle that orginated the event has one child 
			bool unSplittabilityRight = true;
			Splitter * rightSplitter = qobject_cast<Splitter *>(origin->widget(1));
			if(rightSplitter)
				unSplittabilityRight = !(rightSplitter->count()>1);
			unsplitRightAct->setEnabled(unSplittabilityRight);

			//check if the viewer on the left is splittable according to its size
			pickingId =mvc->getViewerByPicking(QPoint(point.x()-epsilon, point.y()));
			if(pickingId>=0)
				splitLeftAct->setEnabled(mvc->getViewer(pickingId)->size().height()/2 > mvc->getViewer(pickingId)->minimumSizeHint().height());
			
			//the viewer on the left can be closed only if the splitter on the left of the handle that orginated the event has one child 		
			bool unSplittabilityLeft = true;
			Splitter * leftSplitter = qobject_cast<Splitter *>(origin->widget(0));
			if(leftSplitter)
				unSplittabilityLeft = !(leftSplitter->count()>1);
			unsplitLeftAct->setEnabled(unSplittabilityLeft);

			splitMenu->addAction(splitRightAct);
			splitMenu->addAction(splitLeftAct);

			unsplitRightAct->setData(point);
			unsplitLeftAct->setData(point);

			unSplitMenu->addAction(unsplitRightAct);
			unSplitMenu->addAction(unsplitLeftAct);
		}

		handleMenu->popup(point);	
}


void MainWindow::splitFromHandle(QAction *qa )
{
  MultiViewer_Container *mvc = currentViewContainer();
	QPoint point = qa->data().toPoint();
	int epsilon =10;

	if(qa->text() == tr("&Right"))
		point.setX(point.x()+ epsilon);
	else if(qa->text() == tr("&Left"))
		point.setX(point.x()- epsilon);
	else if(qa->text() == tr("&Up"))
		point.setY(point.y()- epsilon);
	else if(qa->text() == tr("&Down"))
		point.setY(point.y()+ epsilon);
	
	int newCurrent = mvc->getViewerByPicking(point);
	mvc->updateCurrent(newCurrent);

	QAction *act;
	if(qa->text() == tr("&Right")||qa->text() == tr("&Left"))
		act= new QAction(tr("&Horizontally"), this);
	else if(qa->text() == tr("&Up")||qa->text() == tr("&Down"))
		act= new QAction(tr("&Vertically"), this);

	setSplit(act);
}

void MainWindow::unsplitFromHandle(QAction * qa)
{
  MultiViewer_Container *mvc = currentViewContainer();
	
	QPoint point = qa->data().toPoint();
	int epsilon =10;

	if(qa->text() == tr("&Right"))
		point.setX(point.x()+ epsilon);
	else if(qa->text() == tr("&Left"))
		point.setX(point.x()- epsilon);
	else if(qa->text() == tr("&Up"))
		point.setY(point.y()- epsilon);
	else if(qa->text() == tr("&Down"))
		point.setY(point.y()+ epsilon);
	
	int newCurrent = mvc->getViewerByPicking(point);
	mvc->updateCurrent(newCurrent);

	setUnsplit();
}

void MainWindow::linkViewers()
{
  MultiViewer_Container *mvc = currentViewContainer();
	mvc->updateTrackballInViewers();
}

void MainWindow::viewFrom(QAction *qa)
{
 if(GLA()) GLA()->createOrthoView(qa->text());
}

void MainWindow::readViewFromFile()
{
  if(GLA()) GLA()->viewFromFile();
	updateMenus();
}


void MainWindow::viewFromCurrentMeshShot()
{
  if(GLA()) GLA()->viewFromCurrentShot("Mesh");
  updateMenus();
}

void MainWindow::viewFromCurrentRasterShot()
{
  if(GLA()) GLA()->viewFromCurrentShot("Raster");
  updateMenus();
}

void MainWindow::copyViewToClipBoard()
{
  if(GLA()) GLA()->viewToClipboard();
}

void MainWindow::pasteViewFromClipboard()
{
  if(GLA()) GLA()->viewFromClipboard();
	updateMenus();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	//qDebug("dragEnterEvent: %s",event->format());
	event->accept();
}

void MainWindow::dropEvent ( QDropEvent * event )
{
  //qDebug("dropEvent: %s",event->format());
	const QMimeData * data = event->mimeData();
	if (data->hasUrls())
	{
		QList< QUrl > url_list = data->urls();
		for (int i=0, size=url_list.size(); i<size; i++)
		{
			QString path = url_list.at(i).toLocalFile();
			if( ( QApplication::keyboardModifiers () == Qt::ControlModifier ) && ( GLA() != NULL ) )
				open(path,GLA());
				else open(path);
		}
	}
}

void MainWindow::delCurrentMesh()
{
	//MeshDoc accessed through current container
  currentViewContainer()->meshDoc.delMesh(currentViewContainer()->meshDoc.mm());
  currentViewContainer()->updateAllViewer();
	updateMenus();
}

void MainWindow::delCurrentRaster()
{
	//MeshDoc accessed through current container
  currentViewContainer()->meshDoc.delRaster(currentViewContainer()->meshDoc.rm());
	updateMenus();
}

void MainWindow::endEdit()
{
	GLA()->endEdit();
}
void MainWindow::applyLastFilter()
{
  if(GLA()==0) return;
  GLA()->getLastAppliedFilter()->activate(QAction::Trigger);
}
void MainWindow::showFilterScript()
{
  FilterScriptDialog dialog(this);
  dialog.setScript(&(meshDoc()->filterHistory));
	if (dialog.exec()==QDialog::Accepted)
	{
			runFilterScript();
      return ;
	}

}

void MainWindow::showScriptEditor()
{
	EditorScriptDialog dialog(this);

	if (dialog.exec()==QDialog::Accepted)
	{
		registerTypes(&PM.env);
		//QScriptValue val = PM.env.newQObject(meshDoc());
		//PM.env.globalObject().setProperty("md",val);
		PM.updateDocumentScriptBindings(*meshDoc());
		//PM.env.globalObject().setProperty("md.current",QScriptValue(meshDoc()->mm()->id()));
		QString code = dialog.scriptCode();
		QScriptValue result = PM.env.evaluate(code);
		if (result.isError())
		{
			meshDoc()->Log.Logf(GLLogStream::SYSTEM,"Interpreter Error: line %i: %s",result.property("lineNumber").toInt32(),qPrintable(result.toString()));
			layerDialog->updateLog(meshDoc()->Log);
		}
		else
      GLA()->update();
	}
}

void MainWindow::runFilterScript()
{
  FilterScript::iterator ii;
  for(ii= meshDoc()->filterHistory.actionList.begin();ii!= meshDoc()->filterHistory.actionList.end();++ii)
  {
    QAction *action = PM.actionFilterMap[ (*ii).first];
	  MeshFilterInterface *iFilter = qobject_cast<MeshFilterInterface *>(action->parent());

    int req=iFilter->getRequirements(action);
    meshDoc()->mm()->updateDataMask(req);
    iFilter->setLog(GLA()->log);
		
    RichParameterSet &parameterSet = (*ii).second;
		
		for(int i = 0; i < parameterSet.paramList.size(); i++)
		{	
			//get a modifieable reference
			RichParameter* parameter = parameterSet.paramList[i];

			//if this is a mesh paramter and the index is valid
			if(parameter->val->isMesh())
			{  
				MeshDecoration* md = reinterpret_cast<MeshDecoration*>(parameter->pd);
        if(	md->meshindex < meshDoc()->size() &&
					md->meshindex >= 0  )
				{
          RichMesh* rmesh = new RichMesh(parameter->name,meshDoc()->getMesh(md->meshindex),meshDoc());
					parameterSet.paramList.replace(i,rmesh);
				} else
				{
          printf("Meshes loaded: %i, meshes asked for: %i \n", meshDoc()->size(), md->meshindex );
					printf("One of the filters in the script needs more meshes than you have loaded.\n");
					exit(-1);
				}
				delete parameter;
			}
		}
    //iFilter->applyFilter( action, *(meshDoc()->mm()), (*ii).second, QCallBack );

		//WARNING!!!!!!!!!!!!
		/* to be changed */
    iFilter->applyFilter( action, *meshDoc(), (*ii).second, QCallBack );
		if(iFilter->getClass(action) & MeshFilterInterface::FaceColoring ) {
			GLA()->setColorMode(vcg::GLW::CMPerFace);
      meshDoc()->mm()->updateDataMask(MeshModel::MM_FACECOLOR);
		}
		if(iFilter->getClass(action) & MeshFilterInterface::VertexColoring ){
			GLA()->setColorMode(vcg::GLW::CMPerVert);
      meshDoc()->mm()->updateDataMask(MeshModel::MM_VERTCOLOR);
		}
		if(iFilter->postCondition(action) & MeshModel::MM_COLOR)
		{
			GLA()->setColorMode(vcg::GLW::CMPerMesh);
      meshDoc()->mm()->updateDataMask(MeshModel::MM_COLOR);
		}
		if(iFilter->getClass(action) & MeshFilterInterface::Selection )
    {
        GLA()->setSelectFaceRendering(true);
        GLA()->setSelectVertRendering(true);
    }
		if(iFilter->getClass(action) & MeshFilterInterface::MeshCreation )
			GLA()->resetTrackBall();
		/* to be changed */

    GLA()->log->Logf(GLLogStream::SYSTEM,"Re-Applied filter %s",qPrintable((*ii).first));
	}
}

// Receives the action that wants to show a tooltip and display it
// on screen at the current mouse position.
// TODO: have the tooltip always display with fixed width at the right
//       hand side of the menu entry (not invasive)
void MainWindow::showTooltip(QAction* q){
  QString tip = q->toolTip();
  QToolTip::showText(QCursor::pos(), tip);
}

// /////////////////////////////////////////////////
// The Very Important Procedure of applying a filter
// /////////////////////////////////////////////////
// It is splitted in two part
// - startFilter that setup the dialogs and asks for parameters
// - executeFilter callback invoked when the params have been set up.


void MainWindow::startFilter()
{	
	if(currentViewContainer() == NULL) return;
	if(GLA() == NULL) return;

	// In order to avoid that a filter changes something assumed by the current editing tool,
	// before actually starting the filter we close the current editing tool (if any).
	if(GLA()) GLA()->endEdit();
	updateMenus();

	QStringList missingPreconditions;
	QAction *action = qobject_cast<QAction *>(sender());
	MeshFilterInterface *iFilter = qobject_cast<MeshFilterInterface *>(action->parent());
	//OLD FILTER PHILOSOPHY
	if (iFilter != NULL)
	{
		if(iFilter->getClass(action) == MeshFilterInterface::MeshCreation)
		{
			qDebug("MeshCreation");
			//		MultiViewer_Container *mvcont = new MultiViewer_Container(mdiarea);
			//		connect(mvcont,SIGNAL(updateMainWindowMenus()),this,SLOT(updateMenus()));
			//		GLArea *gla=new GLArea(mvcont, &currentGlobalParams);

			GLA()->meshDoc->addNewMesh("untitled.ply");
			//		gla->setFileName("untitled.ply");
			//    mdiarea->addSubWindow(mvcont);
			//    iFilter->setLog(mvcont->LogPtr());
			//    mvcont->LogPtr()->SetBookmark();

			//if(mdiarea->isVisible()) mvcont->showMaximized();
		}
		else
			if (!iFilter->isFilterApplicable(action,(*meshDoc()->mm()),missingPreconditions))
			{
				QString enstr = missingPreconditions.join(",");
				QMessageBox::warning(0, tr("PreConditions' Failure"), QString("Warning the filter <font color=red>'" + iFilter->filterName(action) + "'</font> has not been applied.<br>"
						"Current mesh does not have <i>" + enstr + "</i>."));
				return;
			}

			if(currentViewContainer())
			{
				iFilter->setLog(currentViewContainer()->LogPtr());
				currentViewContainer()->LogPtr()->SetBookmark();
			}
			// just to be sure...
			createStdPluginWnd();

			// (2) Ask for filter parameters and eventally directly invoke the filter
			// showAutoDialog return true if a dialog have been created (and therefore the execution is demanded to the apply event)
			// if no dialog is created the filter must be executed immediately
			if(! stddialog->showAutoDialog(iFilter, meshDoc()->mm(), (meshDoc()), action, this, GLA()) )
			{
				RichParameterSet dummyParSet;
				executeFilter(action, dummyParSet, false);
			}
	}
	else // NEW XML PHILOSOPHY 
	{
		MeshLabFilterInterface *iXMLFilter = qobject_cast<MeshLabFilterInterface *>(action->parent());
		QString fname = action->text();
		MeshLabXMLFilterContainer filt  = PM.stringXMLFilterMap.value(fname);
		try
		{
			QString filterClasses = filt.xmlInfo->filterAttribute(fname,MLXMLElNames::filterClass);
			QStringList filterClassesList = filterClasses.split(QRegExp("\\W+"), QString::SkipEmptyParts);
			if(filterClassesList.contains("MeshCreation"))
			{
			  qDebug("MeshCreation");
			  GLA()->meshDoc->addNewMesh("untitled.ply");
			}
			else
			{
				QString preCond = filt.xmlInfo->filterAttribute(fname,MLXMLElNames::filterPreCond);
				QStringList preCondList = filterClasses.split(QRegExp("\\W+"), QString::SkipEmptyParts);
				int preCondMask = MeshLabFilterInterface::convertStringListToMeshElementEnum(preCondList);
				if (!MeshLabFilterInterface::arePreCondsValid(preCondMask,(*meshDoc()->mm()),missingPreconditions))
				{
					QString enstr = missingPreconditions.join(",");
					QMessageBox::warning(0, tr("PreConditions' Failure"), QString("Warning the filter <font color=red>'" + filt.act->text() + "'</font> has not been applied.<br>"
					  "Current mesh does not have <i>" + enstr + "</i>."));
					return;
				}
			}
			//INIT PARAMETERS WITH EXPRESSION : Both are defined inside the XML file
			
			
			
			//Inside the MapList there are QMap<QString,QString> containing info about parameters. In particular:
			// "type" - "Boolean","Real" etc
			// "name" - "parameter name"
			// "defaultExpression" - "defExpression"
			// "help" - "parameter help"
			// "typeGui" - "ABSPERC_GUI" "CHECKBOX_GUI" etc
			// "label" - "gui label"
			// Depending to the typeGui could be inside the map other info:
			// for example for ABSPERC_GUI there are also
			// "minExpr" - "minExpr"
			// "maxExpr" - "maxExpr" 

			XMLFilterInfo::XMLMapList params = filt.xmlInfo->filterParametersExtendedInfo(fname);
			

			/*****IMPORTANT NOTE******/
			//the popContext will be called:
			//- or in the executeFilter if the filter will be executed
			//- or in the close Event of stdDialog window if the filter will NOT be executed
			//- or in the catch exception if something went wrong during parsing/scanning
			PM.env.pushContext();
			try
			{
					//each map inside the list contains info (type,name,def_expr) on each parameter inside the filter
				for(XMLFilterInfo::XMLMapList::const_iterator it = params.constBegin();it != params.constEnd();++it)
				{	
					XMLFilterInfo::XMLMap mp = *(it);
						//Initilize the parameters inside the environment
					Expression* exp = ExpressionFactory::create(mp[MLXMLElNames::paramType],mp[MLXMLElNames::paramDefExpr]);
					PM.env.insertLocalExpressionBinding(mp[MLXMLElNames::paramName],exp);
					ExpressionFactory::destroy(exp);	
				}
				if(currentViewContainer())
				{
					iXMLFilter->setLog(currentViewContainer()->LogPtr());
					currentViewContainer()->LogPtr()->SetBookmark();
				}
				// just to be sure...
				createXMLStdPluginWnd();

				// (2) Ask for filter parameters and eventally directly invoke the filter
				// showAutoDialog return true if a dialog have been created (and therefore the execution is demanded to the apply event)
				// if no dialog is created the filter must be executed immediately
				if(! xmldialog->showAutoDialog(&filt, meshDoc(),  this, GLA()) )
				{
					//RichParameterSet dummyParSet;
					//executeFilter(action, dummyParSet, false);
					meshDoc()->Log.Logf(GLLogStream::SYSTEM,"Problem with showAutoDialog.");
				}
			}
			catch (ParsingException e)
			{
				meshDoc()->Log.Logf(GLLogStream::SYSTEM,e.what());	
				PM.env.popContext();
			}
		}
		catch(ParsingException e)
		{
			meshDoc()->Log.Logf(GLLogStream::SYSTEM,e.what());	
		}
	}
}

/*
	callback function that actually start the chosen filter.
  it is called once the parameters have been filled.
	It can be called
	from the automatic dialog
	from the user defined dialog
*/
void MainWindow::executeFilter(QAction *action, RichParameterSet &params, bool isPreview)
{

	MeshFilterInterface         *iFilter    = qobject_cast<        MeshFilterInterface *>(action->parent());

  qb->show();
  iFilter->setLog(&meshDoc()->Log);

	// Ask for filter requirements (eg a filter can need topology, border flags etc)
  // and statisfy them
	qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	MainWindow::globalStatusBar()->showMessage("Starting Filter...",5000);
  int req=iFilter->getRequirements(action);
  meshDoc()->mm()->updateDataMask(req);
  qApp->restoreOverrideCursor();

	// (3) save the current filter and its parameters in the history
  if(!isPreview)
  {
    meshDoc()->filterHistory.actionList.append(qMakePair(action->text(),params));
    meshDoc()->Log.ClearBookmark();
  }
  else
    meshDoc()->Log.BackToBookmark();
  // (4) Apply the Filter
	bool ret;
  qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
	QTime tt; tt.start();
  meshDoc()->setBusy(true);
  RichParameterSet MergedEnvironment(params);
  MergedEnvironment.join(currentGlobalParams);

  //GLA() is only the parent
  QGLWidget* filterWidget = new QGLWidget(GLA());
  QGLFormat defForm = QGLFormat::defaultFormat();
  iFilter->glContext = new QGLContext(defForm,filterWidget->context()->device());
  bool retres = iFilter->glContext->create(filterWidget->context());
  ret=iFilter->applyFilter(action, *(meshDoc()), MergedEnvironment, QCallBack);

  meshDoc()->setBusy(false);

  qApp->restoreOverrideCursor();

  // (5) Apply post filter actions (e.g. recompute non updated stuff if needed)

	if(ret)
	{
    meshDoc()->Log.Logf(GLLogStream::SYSTEM,"Applied filter %s in %i msec",qPrintable(action->text()),tt.elapsed());
    MainWindow::globalStatusBar()->showMessage("Filter successfully completed...",2000);
    if(GLA())
    {
      GLA()->setWindowModified(true);
      GLA()->setLastAppliedFilter(action);
    }
		lastFilterAct->setText(QString("Apply filter ") + action->text());
		lastFilterAct->setEnabled(true);
	}
  else // filter has failed. show the message error.
	{
		QMessageBox::warning(this, tr("Filter Failure"), QString("Failure of filter: '%1'\n\n").arg(action->text())+iFilter->errorMsg()); // text
    MainWindow::globalStatusBar()->showMessage("Filter failed...",2000);
	}
  // at the end for filters that change the color, or selection set the appropriate rendering mode
  if(iFilter->getClass(action) & MeshFilterInterface::FaceColoring ) {
    GLA()->setColorMode(vcg::GLW::CMPerFace);
    meshDoc()->mm()->updateDataMask(MeshModel::MM_FACECOLOR);
  }
  if(iFilter->getClass(action) & MeshFilterInterface::VertexColoring ){
    GLA()->setColorMode(vcg::GLW::CMPerVert);
    meshDoc()->mm()->updateDataMask(MeshModel::MM_VERTCOLOR);
  }
  if(iFilter->postCondition(action) & MeshModel::MM_COLOR)
  {
    GLA()->setColorMode(vcg::GLW::CMPerMesh);
    meshDoc()->mm()->updateDataMask(MeshModel::MM_COLOR);
  }
	if(iFilter->getClass(action) & MeshFilterInterface::Selection )
  {
      GLA()->setSelectVertRendering(true);
      GLA()->setSelectFaceRendering(true);
  }
  if(iFilter->getClass(action) & MeshFilterInterface::MeshCreation )
      GLA()->resetTrackBall();

	if(iFilter->getClass(action) & MeshFilterInterface::Texture )
	    GLA()->updateTexture();

  qb->reset();

  updateMenus();
  GLA()->update(); //now there is the container
  MultiViewer_Container* mvc = currentViewContainer();
  if(mvc)
    mvc->updateAllViewer();

}

// Edit Mode Managment
// At any point there can be a single editing plugin active.
// When a plugin is active it intercept the mouse actions.
// Each active editing tools
//
//


void MainWindow::suspendEditMode()
{
   // return if no window is open
  if(!GLA()) return;

	// return if no editing action is currently ongoing
  if(!GLA()->getCurrentEditAction()) return;

	GLA()->suspendEditToggle();
	updateMenus();
	GLA()->update();
}
void MainWindow::applyEditMode()
{
	if(!GLA()) { //prevents crash without mesh
		QAction *action = qobject_cast<QAction *>(sender());
		action->setChecked(false);
		return;
	}

	QAction *action = qobject_cast<QAction *>(sender());

	if(GLA()->getCurrentEditAction()) //prevents multiple buttons pushed
		{
		  if(action==GLA()->getCurrentEditAction()) // We have double pressed the same action and that means disable that actioon
			{
				if(GLA()->suspendedEditor)
				{
					suspendEditMode();
					return;
				}
				GLA()->endEdit();
				updateMenus();
				return;
			}
			assert(0); // it should be impossible to start an action without having ended the previous one.
			return;
		}

	//if this GLArea does not have an instance of this action's MeshEdit tool then give it one
	if(!GLA()->editorExistsForAction(action))
	{
		MeshEditInterfaceFactory *iEditFactory = qobject_cast<MeshEditInterfaceFactory *>(action->parent());
		MeshEditInterface *iEdit = iEditFactory->getMeshEditInterface(action);
		GLA()->addMeshEditor(action, iEdit);
	}
	GLA()->setCurrentEditAction(action);

	updateMenus();
}

void MainWindow::applyRenderMode()
{
	QAction *action = qobject_cast<QAction *>(sender());		// find the action which has sent the signal

	// Make the call to the plugin core
	MeshRenderInterface *iRenderTemp = qobject_cast<MeshRenderInterface *>(action->parent());
  iRenderTemp->Init(action,*(meshDoc()),GLA()->getCurrentRenderMode(),GLA());

	if(action->text() == tr("None"))
	{
    GLA()->log->Logf(GLLogStream::SYSTEM,"No Shader");
		GLA()->setRenderer(0,0); //vertex and fragment programs not supported
	} else {
		if(iRenderTemp->isSupported())
		{
			GLA()->setRenderer(iRenderTemp,action);
      GLA()->log->Logf(GLLogStream::SYSTEM,"%s",qPrintable(action->text()));	// Prints out action name
		}
		else
		{
			GLA()->setRenderer(0,0); //vertex and fragment programs not supported
      GLA()->log->Logf(GLLogStream::WARNING,"Shader not supported!");
		}
	}
}


void MainWindow::applyDecorateMode()
{
	QAction *action = qobject_cast<QAction *>(sender());		// find the action which has sent the signal
	
	MeshDecorateInterface *iDecorateTemp = qobject_cast<MeshDecorateInterface *>(action->parent());

  bool found=GLA()->iDecoratorsList.removeOne(action);

  if(found)
  {
    GLA()->log->Logf(0,"Disabled Decorate mode %s",qPrintable(action->text()));
  }
  else{
    QString errorMessage;
    if (iDecorateTemp->isDecorationApplicable(action,*(meshDoc()->mm()),errorMessage)) {
				//RichParameterSet * decoratorParams = new RichParameterSet();
				//iDecorateTemp->initGlobalParameterSet(action,decoratorParams);
				bool ret = iDecorateTemp->startDecorate(action,*meshDoc(), &currentGlobalParams, GLA());
				if(ret) {
						GLA()->iDecoratorsList.push_back(action);
						GLA()->log->Logf(GLLogStream::SYSTEM,"Enabled Decorate mode %s",qPrintable(action->text()));
						}
						else GLA()->log->Logf(GLLogStream::SYSTEM,"Failed Decorate mode %s",qPrintable(action->text()));
			}
	}
	layerDialog->updateDecoratorParsView();
	layerDialog->updateLog(*GLA()->log);
	layerDialog->update();
	GLA()->update();
	
}

void MainWindow::setLight()
{
	GLA()->setLight(!GLA()->getCurrentRenderMode().lighting);
	updateMenus();
};

void MainWindow::setDoubleLighting()
{
	const RenderMode &rm = GLA()->getCurrentRenderMode();
	GLA()->setLightMode(!rm.doubleSideLighting,LDOUBLE);
}

void MainWindow::setFancyLighting()
{
	const RenderMode &rm = GLA()->getCurrentRenderMode();
	GLA()->setLightMode(!rm.fancyLighting,LFANCY);
}

void MainWindow::toggleBackFaceCulling()
{
	RenderMode &rm = GLA()->getCurrentRenderMode();
	GLA()->setBackFaceCulling(!rm.backFaceCull);
}

void MainWindow::toggleSelectFaceRendering()
{
  RenderMode &rm = GLA()->getCurrentRenderMode();
  GLA()->setSelectFaceRendering(!rm.selectedFace);
}
void MainWindow::toggleSelectVertRendering()
{
  RenderMode &rm = GLA()->getCurrentRenderMode();
  GLA()->setSelectVertRendering(!rm.selectedVert);
}

bool MainWindow::openIn(QString /* fileName */)
{
	bool wasLayerVisible=layerDialog->isVisible();
	layerDialog->setVisible(false);
	bool ret= open(QString(),GLA());
	layerDialog->setVisible(wasLayerVisible);
	return ret;
}

void MainWindow::saveProject()
{

  QString fileName = QFileDialog::getSaveFileName(this,tr("Save Project File"),lastUsedDirectory.path().append("/test.mlp"), tr("*.aln,*.mlp"));
bool ret;
	qDebug("Saving aln file %s\n",qPrintable(fileName));
	if (fileName.isEmpty()) return;
	else
	{
		//save path away so we can use it again
		QString path = fileName;
		path.truncate(path.lastIndexOf("/"));
		lastUsedDirectory.setPath(path);
	}
  QFileInfo fi(fileName);
  if (QString(fi.suffix()).toLower() == "aln")
  {
  vector<string> meshNameVector;
	vector<Matrix44f> transfVector;

  foreach(MeshModel * mp, meshDoc()->meshList)
	{
        meshNameVector.push_back(qPrintable(mp->shortName()));
		transfVector.push_back(mp->cm.Tr);
	}
   ret= ALNParser::SaveALN(qPrintable(fileName),meshNameVector,transfVector);
}
  else
  {
    ret = MeshDocumentToXMLFile(*meshDoc(),fileName);
  }
	if(!ret)
	 QMessageBox::critical(this, tr("Meshlab Saving Error"), QString("Unable to save project file %1\n").arg(fileName));

}

bool MainWindow::openProject(QString fileName)
{
    if (fileName.isEmpty())
	    fileName = QFileDialog::getOpenFileName(this,tr("Open Project File"), lastUsedDirectory.path(), "*.aln");
	if (fileName.isEmpty()) return false;
	else
	{
		//save path away so we can use it again
		QString path = fileName;
		path.truncate(path.lastIndexOf("/"));
		lastUsedDirectory.setPath(path);
	}

	vector<RangeMap> rmv;

    int retVal=ALNParser::ParseALN(rmv,qPrintable(fileName));
    if(retVal != ALNParser::NoError)
    {
        QMessageBox::critical(this, tr("Meshlab Opening Error"), "Unable to open ALN file");
        return false;
    }

	// this change of dir is needed for subsequent textures/materials loading
	QFileInfo fi(fileName);
	QDir::setCurrent(fi.absoluteDir().absolutePath());

    bool openRes=true;
    vector<RangeMap>::iterator ir;
	for(ir=rmv.begin();ir!=rmv.end() && openRes;++ir)
	{
		if(ir==rmv.begin()) openRes = open((*ir).filename.c_str());
        else				openRes = open((*ir).filename.c_str(),GLA());

        if(openRes) meshDoc()->mm()->cm.Tr=(*ir).trasformation;
	}
    if(this->GLA() == 0) return false;
	this->GLA()->resetTrackBall();
	return true;
}

GLArea* MainWindow::newDocument()
{
  MultiViewer_Container *mvcont = new MultiViewer_Container(mdiarea);
  connect(mvcont,SIGNAL(updateMainWindowMenus()),this,SLOT(updateMenus()));
  GLArea *gla=new GLArea(mvcont, &currentGlobalParams);
  mvcont->addView(gla, Qt::Horizontal);
  mdiarea->addSubWindow(gla->mvc);
  if(mdiarea->isVisible()) gla->mvc->showMaximized();
  return gla;
}

bool MainWindow::open(QString fileName, GLArea *gla)
{
	// Opening files in a transparent form (IO plugins contribution is hidden to user)
	QStringList filters;
	
    // HashTable storing all supported formats together with
	// the (1-based) index  of first plugin which is able to open it
	QHash<QString, MeshIOInterface*> allKnownFormats;
  PM.LoadFormats(filters, allKnownFormats,PluginManager::IMPORT);
	filters.push_back("ALN project ( *.aln)");
	filters.front().chop(1);
	filters.front().append(" *.aln)");
	//Add filters for images
    filters.push_back("Images (*.png *.xpm *.jpg)");
	QStringList fileNameList;
	if (fileName.isEmpty())
		fileNameList = QFileDialog::getOpenFileNames(this,tr("Open File"), lastUsedDirectory.path(), filters.join(";;"));
	else fileNameList.push_back(fileName);

	if (fileNameList.isEmpty())	return false;
	else
	{
		//save path away so we can use it again
		QString path = fileNameList.first();
		path.truncate(path.lastIndexOf("/"));
		lastUsedDirectory.setPath(path);
	}

	foreach(fileName,fileNameList)
	{
			QFileInfo fi(fileName);
			if(fi.suffix().toLower()=="aln") openProject(fileName);
			else if( fi.suffix().toLower()=="png" || fi.suffix().toLower()=="xpm" || fi.suffix().toLower()=="jpg")
			{
				qb->show();

				if(!fi.exists()) 	{
					QString errorMsgFormat = "Unable to open file:\n\"%1\"\n\nError details: file %1 does not exist.";
					QMessageBox::critical(this, tr("Meshlab Opening Error"), errorMsgFormat.arg(fileName));
					return false;
				}
				if(!fi.isReadable()) 	{
					QString errorMsgFormat = "Unable to open file:\n\"%1\"\n\nError details: file %1 is not readable.";
					QMessageBox::critical(this, tr("Meshlab Opening Error"), errorMsgFormat.arg(fileName));
					return false;
				}

				// this change of dir is needed for subsequent textures/materials loading
				QDir::setCurrent(fi.absoluteDir().absolutePath());

				if(!GLA())
					newDocument();

        GLA()->meshDoc->setBusy(true);
        RasterModel *rm= GLA()->meshDoc->addNewRaster(qPrintable(fileName));
        meshDoc()->setBusy(false);

				if(mdiarea->isVisible()) GLA()->mvc->showMaximized();
				updateMenus();
			}
			else
			{
				if(!fi.exists()) 	{
					QString errorMsgFormat = "Unable to open file:\n\"%1\"\n\nError details: file %1 does not exist.";
					QMessageBox::critical(this, tr("Meshlab Opening Error"), errorMsgFormat.arg(fileName));
					return false;
				}
				if(!fi.isReadable()) 	{
					QString errorMsgFormat = "Unable to open file:\n\"%1\"\n\nError details: file %1 is not readable.";
					QMessageBox::critical(this, tr("Meshlab Opening Error"), errorMsgFormat.arg(fileName));
					return false;
				}

				// this change of dir is needed for subsequent textures/materials loading
				QDir::setCurrent(fi.absoluteDir().absolutePath());

				QString extension = fi.suffix();

				// retrieving corresponding IO plugin
				MeshIOInterface *pCurrentIOPlugin = allKnownFormats[extension.toLower()];
				if (pCurrentIOPlugin == 0)
				{
					QString errorMsgFormat = "Error encountered while opening file:\n\"%1\"\n\nError details: The \"%2\" file extension does not correspond to any supported format.";
					QMessageBox::critical(this, tr("Opening Error"), errorMsgFormat.arg(fileName, extension));
					return false;
				}
				//MeshIOInterface* pCurrentIOPlugin = meshIOPlugins[idx-1];

				if(!gla) {
					gla= newDocument();
					// gla=GLA();
				}
				assert(gla);

				pCurrentIOPlugin->setLog(gla->log);

				qb->show();
				RichParameterSet prePar;
				pCurrentIOPlugin->initPreOpenParameter(extension, fileName,prePar);
				if(!prePar.isEmpty())
				{
					GenericParamDialog preOpenDialog(this, &prePar, tr("Pre-Open Options"));
					preOpenDialog.exec();
				}

				int mask = 0;
        //MeshModel *mm= new MeshModel(gla->meshDoc);
        MeshModel *mm=gla->meshDoc->addNewMesh(qPrintable(fileName));

        QTime t;t.start();
				if (!pCurrentIOPlugin->open(extension, fileName, *mm ,mask,prePar,QCallBack,this /*gla*/))
				{
					QMessageBox::warning(this, tr("Opening Failure"), QString("While opening: '%1'\n\n").arg(fileName)+pCurrentIOPlugin->errorMsg()); // text
          gla->meshDoc->delMesh(mm);
				}
        else{
          if(gla) gla->log->Logf(0,"Opened mesh %s in %i msec",qPrintable(fileName),t.elapsed());
					// After opening the mesh lets ask to the io plugin if this format
					// requires some optional, or userdriven post-opening processing.
					// and in that case ask for the required parameters and then
					// ask to the plugin to perform that processing
					RichParameterSet par;
					pCurrentIOPlugin->initOpenParameter(extension, *mm, par);
					if(!par.isEmpty())
					{
						GenericParamDialog postOpenDialog(this, &par, tr("Post-Open Processing"));
						postOpenDialog.exec();
						pCurrentIOPlugin->applyOpenParameter(extension, *mm, par);
					}
          gla->meshDoc->setBusy(true);
          //gla->meshDoc->addNewMesh(qPrintable(fileName),mm);

					//gla->mm()->ioMask |= mask;				// store mask into model structure
					gla->setFileName(mm->shortName());

					if(mdiarea->isVisible()) gla->mvc->showMaximized();
					setCurrentFile(fileName);

					if( mask & vcg::tri::io::Mask::IOM_FACECOLOR)
						gla->setColorMode(GLW::CMPerFace);
					if( mask & vcg::tri::io::Mask::IOM_VERTCOLOR)
					{
						gla->setColorMode(GLW::CMPerVert);
					}
					renderModeTextureAct->setChecked(false);
					renderModeTextureAct->setEnabled(false);
					if(!meshDoc()->mm()->cm.textures.empty())
					{
						renderModeTextureAct->setChecked(true);
						renderModeTextureAct->setEnabled(true);
						if(tri::HasPerVertexTexCoord(meshDoc()->mm()->cm) )
							GLA()->setTextureMode(GLW::TMPerVert);
						if(tri::HasPerWedgeTexCoord(meshDoc()->mm()->cm) )
							GLA()->setTextureMode(GLW::TMPerWedgeMulti);
					}

					// In case of polygonal meshes the normal should be updated accordingly
					if( mask & vcg::tri::io::Mask::IOM_BITPOLYGONAL) 
					{
						mm->updateDataMask(MeshModel::MM_POLYGONAL); // just to be sure. Hopefully it should be done in the plugin...
						int degNum = tri::Clean<CMeshO>::RemoveDegenerateFace(mm->cm);
						if(degNum) GLA()->log->Logf(0,"Warning model contains %i degenerate faces. Removed them.",degNum);
						mm->updateDataMask(MeshModel::MM_FACEFACETOPO);
						vcg::tri::UpdateNormals<CMeshO>::PerBitQuadFaceNormalized(mm->cm);
						vcg::tri::UpdateNormals<CMeshO>::PerVertexFromCurrentFaceNormal(mm->cm);
					} // standard case
					else {
						if( mask & vcg::tri::io::Mask::IOM_VERTNORMAL)
							vcg::tri::UpdateNormals<CMeshO>::PerFace(mm->cm);
						else
							vcg::tri::UpdateNormals<CMeshO>::PerVertexNormalizedPerFaceNormalized(mm->cm);
					}
					vcg::tri::UpdateBounding<CMeshO>::Box(mm->cm);					// updates bounding box

					if(gla->mm()->cm.fn==0){
						gla->setDrawMode(vcg::GLW::DMPoints);
						if(!(mask & vcg::tri::io::Mask::IOM_VERTNORMAL)) 
							gla->setLight(false);
						else 
							mm->updateDataMask(MeshModel::MM_VERTNORMAL);
					}
					else 
						mm->updateDataMask(MeshModel::MM_VERTNORMAL);

					updateMenus();
					int delVertNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(mm->cm);
					int delFaceNum = vcg::tri::Clean<CMeshO>::RemoveDegenerateFace(mm->cm);

					if(delVertNum>0 || delFaceNum>0 )
						QMessageBox::warning(this, "MeshLab Warning", QString("Warning mesh contains %1 vertices with NAN coords and %2 degenerated faces.\nCorrected.").arg(delVertNum).arg(delFaceNum) );
          meshDoc()->setBusy(false);
					//if(newGla)
					GLA()->resetTrackBall();
				}
			}
	}// end foreach file of the input list
	GLA()->update();
	qb->reset();
	return true;
}

void MainWindow::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)	open(action->data().toString());
}

void MainWindow::reload()
{
	// Discards changes and reloads current file
	// save current file name
	QString file = GLA()->getFileName();
	// close current window
	mdiarea->closeActiveSubWindow();

	// open a new window with old file
	open(file);
}

bool MainWindow::save()
{
	return saveAs(meshDoc()->mm()->fullName());
}


bool MainWindow::saveAs(QString fileName)
{
	QStringList filters;

	QHash<QString, MeshIOInterface*> allKnownFormats;

  PM.LoadFormats( filters, allKnownFormats,PluginManager::EXPORT);
  MeshModel *mmm=meshDoc()->mm();

  QString defaultExt = "*." + mmm->suffixName().toLower();
  if(defaultExt == "*.") defaultExt = "*.ply";

  QFileDialog saveDialog(this,tr("Save Current Layer"), mmm->fullName());
  saveDialog.setNameFilters(filters);
  saveDialog.setAcceptMode(QFileDialog::AcceptSave);
  QStringList matchingExtensions=filters.filter(defaultExt);
  if(!matchingExtensions.isEmpty())
    saveDialog.selectNameFilter(matchingExtensions.last());

 if (fileName.isEmpty()){
   int dialogRet = saveDialog.exec();
   if(dialogRet==QDialog::Rejected	) return false;
    fileName=saveDialog.selectedFiles ().first();
    QFileInfo fi(fileName);
    if(fi.suffix().isEmpty())
    {
      QString ext = saveDialog.selectedNameFilter();
      ext.chop(1); ext = ext.right(4);
      fileName = fileName + ext;
      qDebug("File without extension adding it by hand '%s'", qPrintable(fileName));
    }
  }

	bool ret = false;

	QStringList fs = fileName.split(".");

	if(!fileName.isEmpty() && fs.size() < 2)
	{
		QMessageBox::warning(new QWidget(),"Save Error","You must specify file extension!!");
		return ret;
	}

	if (!fileName.isEmpty())
	{
		//save path away so we can use it again
		QString path = fileName;
		path.truncate(path.lastIndexOf("/"));
		lastUsedDirectory.setPath(path);

		QString extension = fileName;
		extension.remove(0, fileName.lastIndexOf('.')+1);

		QStringListIterator itFilter(filters);

		MeshIOInterface *pCurrentIOPlugin = allKnownFormats[extension.toLower()];
		if (pCurrentIOPlugin == 0)
		{
			QMessageBox::warning(this, "Unknown type", "File extension not supported!");
			return false;
		}
		//MeshIOInterface* pCurrentIOPlugin = meshIOPlugins[idx-1];
    pCurrentIOPlugin->setLog(GLA()->log);

		int capability=0,defaultBits=0;
		pCurrentIOPlugin->GetExportMaskCapability(extension,capability,defaultBits);

		// optional saving parameters (like ascii/binary encoding)
		RichParameterSet savePar;

    pCurrentIOPlugin->initSaveParameter(extension,*(this->meshDoc()->mm()),savePar);

    SaveMaskExporterDialog maskDialog(new QWidget(),this->meshDoc()->mm(),capability,defaultBits,&savePar,this->GLA());
		maskDialog.exec();
		int mask = maskDialog.GetNewMask();
		maskDialog.close();
		if(maskDialog.result() == QDialog::Rejected)
			return false;
		if(mask == -1)
			return false;

		qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
		qb->show();
		QTime tt; tt.start();
    ret = pCurrentIOPlugin->save(extension, fileName, *this->meshDoc()->mm() ,mask,savePar,QCallBack,this);
		qb->reset();
    GLA()->log->Logf(GLLogStream::SYSTEM,"Saved Mesh %s in %i msec",qPrintable(fileName),tt.elapsed());

		qApp->restoreOverrideCursor();
        //meshDoc()->mm()->fileName = fileName.toStdString();
		GLA()->setFileName(fileName);
		QSettings settings;
		int savedMeshCounter=settings.value("savedMeshCounter",0).toInt();
		settings.setValue("savedMeshCounter",savedMeshCounter+1);
		GLA()->setWindowModified(false);

	}
	return ret;
}

bool MainWindow::saveSnapshot()
{

	SaveSnapshotDialog dialog(this);

  dialog.setValues(GLA()->ss);

	if (dialog.exec()==QDialog::Accepted)
	{
    GLA()->ss=dialog.getValues();
    GLA()->saveSnapshot();
		return true;
	}

	return false;
}
void MainWindow::about()
{
	QDialog *about_dialog = new QDialog();
	Ui::aboutDialog temp;
	temp.setupUi(about_dialog);
	temp.labelMLName->setText(appName()+"   ("+__DATE__+")");
	//about_dialog->setFixedSize(566,580);
	about_dialog->show();
}

void MainWindow::aboutPlugins()
{
	qDebug( "aboutPlugins(): Current Plugins Dir: %s ",qPrintable(pluginManager().getPluginDirPath()));
        PluginDialog dialog(pluginManager().getPluginDirPath(), pluginManager().pluginsLoaded, this);
	dialog.exec();
}

void MainWindow::helpOnscreen()
{
if(GLA()) GLA()->toggleHelpVisible();
}

void MainWindow::helpOnline()
{
  checkForUpdates(false);
	QDesktopServices::openUrl(QUrl("http://meshlab.sourceforge.net/wiki"));
}


void MainWindow::showToolbarFile(){
		mainToolBar->setVisible(!mainToolBar->isVisible());
}

void MainWindow::showToolbarRender(){
	renderToolBar->setVisible(!renderToolBar->isVisible());
}

void MainWindow::showInfoPane()  {if(GLA() != 0)	GLA()->infoAreaVisible =!GLA()->infoAreaVisible;}
void MainWindow::showTrackBall() {if(GLA() != 0) 	GLA()->showTrackBall(!GLA()->isTrackBallVisible());}
void MainWindow::resetTrackBall(){if(GLA() != 0)	GLA()->resetTrackBall();}
void MainWindow::showLayerDlg() {if(GLA() != 0) 	layerDialog->setVisible( !layerDialog->isVisible() );}

void MainWindow::setCustomize()
{
	CustomDialog dialog(currentGlobalParams,defaultGlobalParams, this);
	connect(&dialog,SIGNAL(applyCustomSetting()),this,SLOT(updateCustomSettings()));
	dialog.exec();
}

void MainWindow::renderBbox()        { GLA()->setDrawMode(GLW::DMBox     ); }
void MainWindow::renderPoint()       { GLA()->setDrawMode(GLW::DMPoints  ); }
void MainWindow::renderWire()        { GLA()->setDrawMode(GLW::DMWire    ); }
void MainWindow::renderFlat()        { GLA()->setDrawMode(GLW::DMFlat    ); }
void MainWindow::renderFlatLine()    { GLA()->setDrawMode(GLW::DMFlatWire); }
void MainWindow::renderHiddenLines() { GLA()->setDrawMode(GLW::DMHidden  ); }
void MainWindow::renderSmooth()      { GLA()->setDrawMode(GLW::DMSmooth  ); }
void MainWindow::renderTexture()
{
	QAction *a = qobject_cast<QAction* >(sender());
  if( tri::HasPerVertexTexCoord(meshDoc()->mm()->cm))
    GLA()->setTextureMode(!a->isChecked() ? GLW::TMNone : GLW::TMPerVert);
  if( tri::HasPerWedgeTexCoord(meshDoc()->mm()->cm))
    GLA()->setTextureMode(!a->isChecked() ? GLW::TMNone : GLW::TMPerWedgeMulti);
}


void MainWindow::fullScreen(){
  if(!isFullScreen())
  {
	  toolbarState = saveState();
	  menuBar()->hide();
	  mainToolBar->hide();
	  renderToolBar->hide();
    globalStatusBar()->hide();
	  setWindowState(windowState()^Qt::WindowFullScreen);
	  bool found=true;
	  //Caso di piu' finestre aperte in tile:
	  if((mdiarea->subWindowList()).size()>1){
		  foreach(QWidget *w,mdiarea->subWindowList()){if(w->isMaximized()) found=false;}
		  if (found)mdiarea->tileSubWindows();
	  }
  }
  else
  {
    menuBar()->show();
		restoreState(toolbarState);
    globalStatusBar()->show();

		setWindowState(windowState()^ Qt::WindowFullScreen);
		bool found=true;
		//Caso di piu' finestre aperte in tile:
		if((mdiarea->subWindowList()).size()>1){
			foreach(QWidget *w,mdiarea->subWindowList()){if(w->isMaximized()) found=false;}
			if (found){mdiarea->tileSubWindows();}
		}
		fullScreenAct->setChecked(false);
  }
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
  if(e->key()==Qt::Key_Return && e->modifiers()==Qt::AltModifier)
  {
    fullScreen();
    e->accept();
  }
  else e->ignore();
}

bool MainWindow::QCallBack(const int pos, const char * str)
{
	int static lastPos=-1;
	if(pos==lastPos) return true;
	lastPos=pos;

	static QTime currTime;
	if(currTime.elapsed()< 100) return true;
	currTime.start();
	MainWindow::globalStatusBar()->showMessage(str,5000);
	qb->show();
	qb->setEnabled(true);
	qb->setValue(pos);
	MainWindow::globalStatusBar()->update();
	qApp->processEvents();
	return true;
}

void MainWindow::evaluateExpression(const Expression& exp,Value** res )
{
	PM.env.pushContext();
	*res = exp.eval(&PM.env);
	PM.env.popContext();
}
