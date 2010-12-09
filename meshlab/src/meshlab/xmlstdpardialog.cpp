#include "xmlstdpardialog.h"
#include <QtGui>

MeshLabXMLStdDialog::MeshLabXMLStdDialog( QWidget *p )
:QDockWidget(QString("Plugin"), p),showHelp(false)
{
	curmask = 0;
	qf = NULL;
	stdParFrame=NULL;
	clearValues();
}

MeshLabXMLStdDialog::~MeshLabXMLStdDialog()
{

}

void MeshLabXMLStdDialog::clearValues()
{
	curAction = NULL;
	curModel = NULL;
	curmfc = NULL;
	curmwi = NULL;
}

void MeshLabXMLStdDialog::createFrame()
{
	if(qf) delete qf;

	QFrame *newqf= new QFrame(this);
	setWidget(newqf);
	qf = newqf;
}

void MeshLabXMLStdDialog::loadFrameContent( )
{
	assert(qf);
	qf->hide();
	QLabel *ql;


	QGridLayout *gridLayout = new QGridLayout(qf);
	qf->setLayout(gridLayout);
	QString fname(curmfc->act->text());
	setWindowTitle(fname);
	ql = new QLabel("<i>"+curmfc->xmlInfo->filterHelp(fname)+"</i>",qf);
	ql->setTextFormat(Qt::RichText);
	ql->setWordWrap(true);
	gridLayout->addWidget(ql,0,0,1,2,Qt::AlignTop); // this widgets spans over two columns.

	stdParFrame = new XMLStdParFrame(this,curgla);
	connect(stdParFrame,SIGNAL(frameEvaluateExpression(const Expression&,Value**)),this,SIGNAL(dialogEvaluateExpression(const Expression&,Value**)),Qt::DirectConnection);

	XMLFilterInfo::XMLMapList mplist = curmfc->xmlInfo->filterParametersExtendedInfo(fname);
	stdParFrame->loadFrameContent(mplist);
	gridLayout->addWidget(stdParFrame,1,0,1,2);
	
	//int buttonRow = 2;  // the row where the line of buttons start

	QPushButton *helpButton = new QPushButton("Help", qf);
	//helpButton->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);
	QPushButton *closeButton = new QPushButton("Close", qf);
	//closeButton->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);
	QPushButton *applyButton = new QPushButton("Apply", qf);
	//applyButton->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);
	QPushButton *defaultButton = new QPushButton("Default", qf);
	//defaultButton->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);
	ExpandButtonWidget* exp = new ExpandButtonWidget(qf);
	connect(exp,SIGNAL(expandView(bool)),this,SLOT(extendedView(bool)));

#ifdef Q_WS_MAC
	// Hack needed on mac for correct sizes of button in the bottom of the dialog.
	helpButton->setMinimumSize(100, 25);
	closeButton->setMinimumSize(100,25);
	applyButton->setMinimumSize(100,25);
	defaultButton->setMinimumSize(100, 25);
#endif 	

	if(isDynamic())
	{
		previewCB = new QCheckBox("Preview", qf);
		previewCB->setCheckState(Qt::Unchecked);
		gridLayout->addWidget(previewCB,    gridLayout->rowCount(),0,Qt::AlignBottom);
		connect(previewCB,SIGNAL(toggled(bool)),this,SLOT( togglePreview() ));
		//buttonRow++;
	}

	gridLayout->addWidget(exp,gridLayout->rowCount(),0,1,2,Qt::AlignJustify);
	int firstButLine =  gridLayout->rowCount();
	gridLayout->addWidget(helpButton,   firstButLine,1,Qt::AlignBottom);
	gridLayout->addWidget(defaultButton,firstButLine,0,Qt::AlignBottom);
	int secButLine = gridLayout->rowCount();
	gridLayout->addWidget(closeButton,  secButLine,0,Qt::AlignBottom);
	gridLayout->addWidget(applyButton,  secButLine,1,Qt::AlignBottom);


	connect(helpButton,SIGNAL(clicked()),this,SLOT(toggleHelp()));
	connect(applyButton,SIGNAL(clicked()),this,SLOT(applyClick()));
	connect(closeButton,SIGNAL(clicked()),this,SLOT(closeClick()));
	connect(defaultButton,SIGNAL(clicked()),this,SLOT(resetValues()));

	qf->showNormal();
	qf->adjustSize();

	//set the minimum size so it will shrink down to the right size	after the help is toggled
	//this->setMinimumSize(qf->sizeHint());
	this->showNormal();
	this->adjustSize();
	//setSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);
}

bool MeshLabXMLStdDialog::showAutoDialog(MeshLabXMLFilterContainer *mfc,MeshDocument * md, MainWindowInterface *mwi, QWidget *gla/*=0*/ )
{
	if (mfc == NULL) 
		return false;
	if (mfc->filterInterface == NULL)
		return false;
	if (mfc->xmlInfo == NULL)
		return false;
	if (mfc->act == NULL)
		return false;

	validcache = false;
	curAction=mfc->act;
	curmfc=mfc;
	curmwi=mwi;
	curParMap.clear();
	prevParMap.clear();
	curModel = md->mm();
	curMeshDoc = md;
	curgla=gla;

	QString fname = mfc->act->text();
	//mfi->initParameterSet(action, *mdp, curParSet);
  XMLFilterInfo::XMLMapList mplist = mfc->xmlInfo->filterParametersExtendedInfo(fname);
	curParMap = mplist;
	//curmask = mfc->xmlInfo->filterAttribute(mfc->act->text(),QString("postCond"));
	if(curParMap.isEmpty() && !isDynamic()) 
		return false;

	createFrame();
	loadFrameContent();
	QString postCond = mfc->xmlInfo->filterAttribute(fname,MLXMLElNames::filterPostCond);
	QStringList postCondList = postCond.split(QRegExp("\\W+"), QString::SkipEmptyParts);
	curmask = MeshLabFilterInterface::convertStringListToMeshElementEnum(postCondList);
	if(isDynamic())
	{
		meshState.create(curmask, curModel);
		connect(stdParFrame,SIGNAL(dynamicFloatChanged(int)), this, SLOT(applyDynamic()));
		connect(stdParFrame,SIGNAL(parameterChanged()), this, SLOT(applyDynamic()));
	}
	connect(curMeshDoc, SIGNAL(currentMeshChanged(int)),this, SLOT(changeCurrentMesh(int)));
	return true;
}

void MeshLabXMLStdDialog::applyClick()
{
	QAction *q = curAction;
	filtEnv.clear();
	assert(curParMap.size() == stdParFrame->xmlfieldwidgets.size());
	for(int ii = 0;ii < curParMap.size();++ii)	
	{
		XMLMeshLabWidget* wid = stdParFrame->xmlfieldwidgets[ii];
		Expression* exp = wid->getWidgetExpression();
		//the result value will be destructed when the FilterEnv will be disposed
		Value* res = NULL;
		emit dialogEvaluateExpression(*exp,&res);
		filtEnv.insertValueBinding(curParMap[ii][MLXMLElNames::paramName],res);
		delete exp;
	}
	////int mask = 0;//curParSet.getDynamicFloatMask();
	if(curmask)	
		meshState.apply(curModel);

	//PreView Caching: if the apply parameters are the same to those used in the preview mode
	//we don't need to reapply the filter to the mesh
	//bool isEqual = (curParSet == prevParSet);
	//if ((isEqual) && (validcache))
	//	meshCacheState.apply(curModel);
	//else
	//curmwi->executeFilter(curmfc, curParSet, false);
	

	if(curmask)	
		meshState.create(curmask, curModel);
	if(this->curgla) 
		this->curgla->update();
}

void MeshLabXMLStdDialog::closeClick()
{

}

void MeshLabXMLStdDialog::resetValues()
{
	//curParSet.clear();
	//curmfi->initParameterSet(curAction, *curMeshDoc, curParSet);

	//assert(qf);
	//assert(qf->isVisible());
	//// assert(curParSet.paramList.count() == stdfieldwidgets.count());
	//stdParFrame->resetValues(curParSet);
}

void MeshLabXMLStdDialog::toggleHelp()
{
	showHelp = !showHelp;
	stdParFrame->toggleHelp(showHelp);
	qf->updateGeometry();	
	qf->adjustSize();
	this->updateGeometry();
	this->adjustSize();
}

void MeshLabXMLStdDialog::extendedView(bool ext)
{
	stdParFrame->extendedView(ext,showHelp);
	qf->updateGeometry();	
	qf->adjustSize();
	this->updateGeometry();
	this->adjustSize();
}

void MeshLabXMLStdDialog::togglePreview()
{
	if(previewCB->isChecked()) 
	{
		applyDynamic();
	}
	else
		meshState.apply(curModel);

	curgla->update();
}

void MeshLabXMLStdDialog::applyDynamic()
{

}

void MeshLabXMLStdDialog::changeCurrentMesh( int meshInd )
{
	if(isDynamic())
	{
		meshState.apply(curModel);
		curModel=curMeshDoc->meshList.at(meshInd);
		meshState.create(curmask, curModel);
		applyDynamic();
	}
}

//void MeshLabXMLStdDialog::closeEvent( QCloseEvent * event )
//{
//
//}

bool MeshLabXMLStdDialog::isDynamic() const
{
	 return ((curmask != MeshModel::MM_UNKNOWN) && (curmask != MeshModel::MM_NONE) && !(curmask & MeshModel::MM_VERTNUMBER) && !(curmask & MeshModel::MM_FACENUMBER));
}

XMLStdParFrame::XMLStdParFrame( QWidget *p, QWidget *gla/*=0*/ )
:QFrame(p),extended(false)
{
	curr_gla=gla;
	vLayout = new QGridLayout(this);
	vLayout->setAlignment(Qt::AlignTop);
	setLayout(vLayout);
	//connect(p,SIGNAL(expandView(bool)),this,SLOT(expandView(bool)));
	//updateFrameContent(parMap,false);
	//this->setMinimumWidth(vLayout->sizeHint().width());
	

	//this->showNormal();
	this->adjustSize();
}

XMLStdParFrame::~XMLStdParFrame()
{

}

void XMLStdParFrame::loadFrameContent(const XMLFilterInfo::XMLMapList& parMap)
{
	for(XMLFilterInfo::XMLMapList::const_iterator it = parMap.constBegin();it != parMap.constEnd();++it)
	{
		XMLMeshLabWidget* widg = XMLMeshLabWidgetFactory::create(*it,this);
		if (widg == NULL)
			return;
		xmlfieldwidgets.push_back(widg); 
		helpList.push_back(widg->helpLabel());
	}
	//showNormal();
	updateGeometry();
	adjustSize();
}

void XMLStdParFrame::toggleHelp(bool help)
{
	for(int i = 0; i < helpList.count(); i++)
		helpList.at(i)->setVisible(help && xmlfieldwidgets[i]->isVisible()) ;
	updateGeometry();
	adjustSize();
}

void XMLStdParFrame::extendedView(bool ext,bool help)
{
	for(int i = 0; i < xmlfieldwidgets.count(); i++)
		xmlfieldwidgets[i]->setVisibility(ext || xmlfieldwidgets[i]->isImportant); 
	if (help)
		toggleHelp(help);
	updateGeometry();
	adjustSize();
}

XMLMeshLabWidget::XMLMeshLabWidget(const XMLFilterInfo::XMLMap& mp,QWidget* parent )
:QWidget(parent)
{
	//WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//It's not nice at all doing the connection for an external object! The connect should be called in XMLStdParFrame::loadFrameContent but in this way 
	//we must break the construction of the widget in two steps because otherwise in the constructor (called by XMLMeshLabWidgetFactory::create) the emit is invoked
	//before the connection!
	connect(this,SIGNAL(widgetEvaluateExpression(const Expression&,Value**)),parent,SIGNAL(frameEvaluateExpression(const Expression&,Value**)),Qt::DirectConnection);

	Value* isImp = NULL;
	BoolExpression isImpExp(mp[MLXMLElNames::paramIsImportant]);
	emit widgetEvaluateExpression(isImpExp,&isImp);
	isImportant = isImp->getBool();
	delete isImp;
	setVisible(isImportant);
		
	helpLab = new QLabel("<small>"+ mp[MLXMLElNames::paramHelpTag] +"</small>",this);
	helpLab->setTextFormat(Qt::RichText);
	helpLab->setWordWrap(true);
	helpLab->setVisible(false);
	helpLab->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	helpLab->setMinimumWidth(250);
	helpLab->setMaximumWidth(QWIDGETSIZE_MAX);
	gridLay = qobject_cast<QGridLayout*>(parent->layout());
	assert(gridLay != 0);

	row = gridLay->rowCount();
	////WARNING!!!!!!!!!!!!!!!!!! HORRIBLE PATCH FOR THE BOOL WIDGET PROBLEM
	if ((row == 1) && (mp[MLXMLElNames::paramType] == MLXMLElNames::boolType))	
	{

		QLabel* lb = new QLabel("",this);
		gridLay->addWidget(lb);
		gridLay->addWidget(helpLab,row+1,3,1,1,Qt::AlignTop);
	}
	/////////////////////////////////////////////////////////////////////////
	else
		gridLay->addWidget(helpLab,row,3,1,1,Qt::AlignTop);
}

void XMLMeshLabWidget::setVisibility( const bool vis )
{
	helpLabel()->setVisible(helpLabel()->isVisible() && vis);
	updateVisibility(vis);
	setVisible(vis);
}

XMLCheckBoxWidget::XMLCheckBoxWidget( const XMLFilterInfo::XMLMap& xmlWidgetTag,QWidget* parent )
:XMLMeshLabWidget(xmlWidgetTag,parent)
{
	cb = new QCheckBox(xmlWidgetTag[MLXMLElNames::paramName],this);
	cb->setToolTip(xmlWidgetTag[MLXMLElNames::paramHelpTag]);
	BoolExpression exp(xmlWidgetTag[MLXMLElNames::paramDefExpr]);
	Value* boolVal = NULL;
	emit widgetEvaluateExpression(exp,&boolVal);
	cb->setChecked(boolVal->getBool());
	cb->setVisible(isImportant);
	delete boolVal;

	//gridlay->addWidget(this,i,0,1,1,Qt::AlignTop);

	//int row = gridLay->rowCount() -1 ;
	//WARNING!!!!!!!!!!!!!!!!!! HORRIBLE PATCH FOR THE BOOL WIDGET PROBLEM
	if (row == 1)
		gridLay->addWidget(cb,row + 1,0,1,2,Qt::AlignTop);
	/////////////////////////////////////////////////////////////////////////
	else
		gridLay->addWidget(cb,row,0,1,2,Qt::AlignTop);

	cb->setVisible(isImportant);
	//cb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	connect(cb,SIGNAL(stateChanged(int)),parent,SIGNAL(parameterChanged()));
}

XMLCheckBoxWidget::~XMLCheckBoxWidget()
{

}

void XMLCheckBoxWidget::resetWidgetValue()
{

}

void XMLCheckBoxWidget::collectWidgetValue()
{

}

void XMLCheckBoxWidget::setWidgetExpression( const QString& /*nv*/ )
{

}

void XMLCheckBoxWidget::updateWidget( const XMLFilterInfo::XMLMap& /*xmlWidgetTag*/ )
{

}

void XMLCheckBoxWidget::updateVisibility( const bool vis )
{
	cb->setVisible(vis);
}

Expression* XMLCheckBoxWidget::getWidgetExpression()
{
	QString state;
	if (cb->isChecked())
		state = QString("true");
	else
		state = QString("false");
	return new BoolExpression(state);
}

XMLMeshLabWidget* XMLMeshLabWidgetFactory::create(const XMLFilterInfo::XMLMap& widgetTable,QWidget* parent)
{
	QString guiType = widgetTable[MLXMLElNames::guiType];
	if (guiType == MLXMLElNames::editTag)
		return new XMLEditWidget(widgetTable,parent);

	if (guiType == MLXMLElNames::checkBoxTag)
		return new XMLCheckBoxWidget(widgetTable,parent);

	if (guiType == MLXMLElNames::absPercTag)
		return new XMLAbsWidget(widgetTable,parent);
	return NULL;
}

XMLEditWidget::XMLEditWidget(const XMLFilterInfo::XMLMap& xmlWidgetTag,QWidget* parent)
:XMLMeshLabWidget(xmlWidgetTag,parent)
{
	fieldDesc = new QLabel(xmlWidgetTag[MLXMLElNames::paramName],this);
	lineEdit = new QLineEdit(this);
	//int row = gridLay->rowCount() -1;

	fieldDesc->setToolTip(xmlWidgetTag[MLXMLElNames::paramHelpTag]);
	lineEdit->setText(xmlWidgetTag[MLXMLElNames::paramDefExpr]);
	
	gridLay->addWidget(fieldDesc,row,0,Qt::AlignTop);
	gridLay->addWidget(lineEdit,row,1,Qt::AlignTop);
	//connect(lineEdit,SIGNAL(editingFinished()),p,SIGNAL(parameterChanged()));
	connect(lineEdit,SIGNAL(selectionChanged()),this,SLOT(tooltipEvaluation()));
	
	fieldDesc->setVisible(isImportant);
	this->lineEdit->setVisible(isImportant);
}



XMLEditWidget::~XMLEditWidget()
{

}

void XMLEditWidget::resetWidgetValue()
{

}

void XMLEditWidget::collectWidgetValue()
{

}


void XMLEditWidget::setWidgetExpression( const QString& nv )
{

}

void XMLEditWidget::updateWidget( const XMLFilterInfo::XMLMap& xmlWidgetTag )
{

}

void XMLEditWidget::updateVisibility( const bool vis )
{
	fieldDesc->setVisible(vis);
	this->lineEdit->setVisible(vis);
}

void XMLEditWidget::tooltipEvaluation()
{
	QString exp = lineEdit->selectedText();
	FloatExpression flExp(exp);
	Value* val = NULL;
	emit widgetEvaluateExpression(flExp,&val);
	if (val != NULL)
		lineEdit->setToolTip(QString::number(val->getFloat()));
	delete val;
}

Expression* XMLEditWidget::getWidgetExpression()
{
	return new FloatExpression(this->lineEdit->text());
}

XMLAbsWidget::XMLAbsWidget(const XMLFilterInfo::XMLMap& xmlWidgetTag, QWidget* parent )
:XMLMeshLabWidget(xmlWidgetTag,parent),minVal(NULL),maxVal(NULL)
{
	FloatExpression minExp(xmlWidgetTag[MLXMLElNames::guiMinExpr]);
	FloatExpression maxExp(xmlWidgetTag[MLXMLElNames::guiMaxExpr]);
	emit widgetEvaluateExpression(minExp,&minVal);
	emit widgetEvaluateExpression(maxExp,&maxVal);


	fieldDesc = new QLabel(xmlWidgetTag[MLXMLElNames::paramName] + " (abs and %)",this);
	fieldDesc->setToolTip(xmlWidgetTag[MLXMLElNames::paramHelpTag]);
	absSB = new QDoubleSpinBox(this);
	percSB = new QDoubleSpinBox(this);

	//called with m_* only to maintain backward compatibility
	float m_min = minVal->getFloat();
	float m_max = maxVal->getFloat();

	absSB->setMinimum(m_min-(m_max-m_min));
	absSB->setMaximum(m_max*2);
	absSB->setAlignment(Qt::AlignRight);

	int decimals= 7-ceil(log10(fabs(m_max-m_min)) ) ;
	//qDebug("range is (%f %f) %f ",m_max,m_min,fabs(m_max-m_min));
	//qDebug("log range is %f ",log10(fabs(m_max-m_min)));
	absSB->setDecimals(decimals);
	absSB->setSingleStep((m_max-m_min)/100.0);
	//float initVal = rp->val->getAbsPerc();
	float initVal = 0.0f;
	absSB->setValue(initVal);

	percSB->setMinimum(-200);
	percSB->setMaximum(200);
	percSB->setAlignment(Qt::AlignRight);
	percSB->setSingleStep(0.5);
	percSB->setValue((100*(initVal - m_min))/(m_max - m_min));
	percSB->setDecimals(3);
	absLab=new QLabel("<i> <small> world unit</small></i>",this);
	percLab=new QLabel("<i> <small> perc on"+QString("(%1 .. %2)").arg(m_min).arg(m_max)+"</small></i>",this);

	gridLay->addWidget(fieldDesc,row,0,Qt::AlignLeft);
	QGridLayout* lay = new QGridLayout();
	lay->addWidget(absLab,0,0,Qt::AlignHCenter);
	lay->addWidget(percLab,0,1,Qt::AlignHCenter);
	lay->addWidget(absSB,1,0,Qt::AlignTop);
	lay->addWidget(percSB,1,1,Qt::AlignTop);
	gridLay->addLayout(lay,row,1,1,2,Qt::AlignTop);
	

		//connect(absSB,SIGNAL(valueChanged(double)),this,SLOT(on_absSB_valueChanged(double)));
		//connect(percSB,SIGNAL(valueChanged(double)),this,SLOT(on_percSB_valueChanged(double)));
		//connect(this,SIGNAL(dialogParamChanged()),parent,SIGNAL(parameterChanged()));
	this->absLab->setVisible(isImportant);
	//this->absLab->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	this->percLab->setVisible(isImportant);
	//this->percLab->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	this->fieldDesc->setVisible(isImportant);
	//this->fieldDesc->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	this->absSB->setVisible(isImportant);
	//this->absSB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	this->percSB->setVisible(isImportant);
	//this->percSB->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
}

XMLAbsWidget::~XMLAbsWidget()
{
	delete minVal;
	delete maxVal;
}

void XMLAbsWidget::resetWidgetValue()
{

}

void XMLAbsWidget::collectWidgetValue()
{
	//rp->val->set(FloatValue(float(absSB->value())));
}

void XMLAbsWidget::setWidgetExpression( const QString& /*nv*/ )
{

}

void XMLAbsWidget::updateWidget( const XMLFilterInfo::XMLMap& /*xmlWidgetTag*/ )
{

}

void XMLAbsWidget::updateVisibility( const bool vis )
{
	this->absLab->setVisible(vis);
	this->percLab->setVisible(vis);
	this->fieldDesc->setVisible(vis);
	this->absSB->setVisible(vis);
	this->percSB->setVisible(vis);
}

Expression* XMLAbsWidget::getWidgetExpression()
{
	return new FloatExpression(QString::number(absSB->value()));
}

ExpandButtonWidget::ExpandButtonWidget( QWidget* parent )
:QWidget(parent),up(0x0035),down(0x0036),isExpanded(false)
{
	arrow = down;
	hlay = new QHBoxLayout(this);
	//QChar ch(0x0036);
	exp = new QPushButton(arrow,this);
	exp->setFlat(true);
	exp->setFont(QFont("Webdings",12));
	//connect(exp,SIGNAL(clicked(bool)),this,SLOT(expandFrame(bool)));
	QFontMetrics mt(exp->font(),exp);
	QSize sz = mt.size(Qt::TextSingleLine,arrow);
	sz.setWidth(sz.width() + 10);
	exp->setMaximumSize(sz);
	hlay->addWidget(exp,0,Qt::AlignHCenter);
	connect(exp,SIGNAL(clicked(bool)),this,SLOT(changeIcon()));
}

ExpandButtonWidget::~ExpandButtonWidget()
{

}

void ExpandButtonWidget::changeIcon()
{
	isExpanded = !isExpanded;
	if (isExpanded)
		arrow = up;
	else
		arrow = down;
	exp->setText(arrow);
	emit expandView(isExpanded);
}
