#include "qualitymapperdialog.h"
#include <limits>
#include <QPen>
#include <QBrush>


QualityMapperDialog::QualityMapperDialog(QWidget *parent)
: QDialog(parent)
{
	ui.setupUi(this);
}

QualityMapperDialog::~QualityMapperDialog()
{

}

void QualityMapperDialog::setValues(const QualityMapperSettings& qms)
{
	_settings=qms;
	QString qnum="%1";
	ui.minSpinBox->setValue(_settings.meshMinQ);
	ui.midSpinBox->setValue(_settings.meshMidQ);
	ui.maxSpinBox->setValue(_settings.meshMaxQ);
}

QualityMapperSettings QualityMapperDialog::getValues()
{
	_settings.manualMinQ=ui.minSpinBox->value();
	_settings.manualMidQ=ui.midSpinBox->value();
	_settings.manualMaxQ=ui.maxSpinBox->value();

	return _settings;
}


void QualityMapperDialog::drawCartesianChartBasics(QGraphicsScene& scene, QGraphicsView *view)
{
	int width = /*300;*/view->width();
	int height = /*200;*/view->height();

	border = 5;
	chartRectangleThickness = 2;
	leftBorder = border;
	rightBorder = width - border;
	upperBorder = border;
	lowerBorderForCartesians = height - (int)(border * 3);

	// Create a LightBlue background
	//_equalizerScene.addRect(chartRectangleThickness, chartRectangleThickness, width - (2 * chartRectangleThickness), height - (2 * chartRectangleThickness), QPen(), QBrush(QColor(230, 230, 255), Qt::SolidPattern));

	//drawing chart title
	//this.writeString(g, title, titleLabelSize, Color.Black, Positions.North);

	//drawing scale values on X and Y axis
	//this.drawXYValuesScale(chartType, g, maxRoundedY, vals, xyLabelsSize);

	QPen p( Qt::black, 2 );

	//drawing axis
	//x axis
	scene.addLine(leftBorder, lowerBorderForCartesians, rightBorder, lowerBorderForCartesians, p );
	//y axis
	scene.addLine(leftBorder, upperBorder, leftBorder, lowerBorderForCartesians, p );

	/*
	//drawing axis labels
	//drawing X label
	this.writeString(g, header.xLabel, xyLabelsSize, Color.Black, Positions.South);
	//drawing Y label
	String yLabel = header.yLabel;
	//if the y label represent a delay, the time unit is added to the string (min.) [MINUTE]
	if ((yLabel.ToLower()).Contains("delay"))
	yLabel += " min.";
	this.writeString(g, yLabel, xyLabelsSize, Color.Black, Positions.West);
	*/
}


//il nome non mi sembra giusto. Questo dovrebbe essere piuttosto drawHistogram
//_equalizerScene dovrebbe chiamarsi in realt� histogramScene
//l'histogram e la transfer function potrebbero diventare attributi di questa classe? valutare l'impatto.
//in generale il codice di questo metodo va ripulito un po'...
void QualityMapperDialog::initEqualizerHistogram( Histogramf& h )
{
	//_equalizerScene();
	_equalizerScene.addText("Hello World!");

	int width = /*300;*/ui.equalizerGraphicsView->width();
	int height = /*200;*/ui.equalizerGraphicsView->height();

	border = 5;
	chartRectangleThickness = 2;
	leftBorder = border;
	rightBorder = width - border;
	upperBorder = border;
	lowerBorderForCartesians = height - (int)(border * 3);

	int chartWidth = rightBorder - leftBorder;
	int chartHeightForCartesians = lowerBorderForCartesians - upperBorder;
	int yScaleStep = 5;

	float dX = (float)chartWidth / (float)h.n;
	float dY = (float)chartHeightForCartesians / (float)h.n;

	int maxY = 0;
	int minY = std::numeric_limits<int>::max();

	//processing minX, maxX, minY and maxY values
	for (int i=0; i<h.n; i++) 
	{
		if (h.H[i] > maxY)
			maxY = h.H[i];

		if (h.H[i] < minY)
			minY = h.H[i];
	}

	int maxRoundedY = (int)(maxY + yScaleStep - (maxY % yScaleStep));    //the highest value represented in the y values scale
	float variance = maxY - minY;          //variance of y values
	float barHeight = 0.0F;                //initializing height of the histogram bars
	float barWidth = dX * 4.0F / 5.0F;     //processing width of the histogram bars (4\5 of dX)
	float barSeparator = dX / 5.0F;        //processing space between consecutive bars of the histogram bars (1\5 of dX)

	QPen drawingPen(Qt::black);
	QBrush drawingBrush (QColor(32, 32, 32),Qt::SolidPattern);

	QPointF startBarPt;
	QSizeF barSize;

	//QColor valuesColor;

	//drawing chart basics
	drawCartesianChartBasics( _equalizerScene, ui.equalizerGraphicsView );
	//QSizeF valuesStringSize;
	//Font f = new Font("Verdana", valuesLabelSize);

	
	//drawing histogram bars
	for (int i = 0; i < h.n; i++)
	{
		barHeight = (float)(chartHeightForCartesians * h.H[i]) / (float)maxRoundedY;
		startBarPt.setX( leftBorder + ((barSeparator + barWidth) * i) );
		startBarPt.setY( (float)lowerBorderForCartesians - barHeight );

		barSize.setWidth(barWidth);
		barSize.setHeight(barHeight);

		//drawing histogram bar
		_equalizerScene.addRect(startBarPt.x(), startBarPt.y(), barSize.width(), barSize.height(), drawingPen, drawingBrush);

		/*
		//drawing bar (numeric) value
		//if the bar is large enough to contain a numeric value...
		valuesStringSize = g.MeasureString(values[i].yValue.ToString(), f);

		//choosing text color (red for min and max value, white else)
		if ((Convert.ToSingle(values[i].yValue) == maxY) || (Convert.ToSingle(values[i].yValue) == minY))
		valuesColor = Color.Red;
		else
		valuesColor = Color.Gray;
		valuesColor = QColor(128,128,128);

		if (barHeight < (valuesStringSize.Height + chartRectangleThickness))
		startBarPt.Y -= (valuesStringSize.Height + chartRectangleThickness);

		//the value is written only if there's space enough between the bars or if the value to represent is the min or the max value
		if ((barSize.Width >= (valuesStringSize.Width * 0.9F)) || (valuesColor == Color.Red))
		{
		this.writeString(g, Convert.ToString(values[i].yValue), valuesLabelSize, valuesColor, Positions.Custom, startBarPt);
		}*/
	}

	ui.equalizerGraphicsView->setScene(&_equalizerScene);
}


void QualityMapperDialog::drawTransferFunction(TransferFunction& tf)
{
	this->drawCartesianChartBasics( _transferFunctionScene, ui.transferFunctionView );
	_transferFunctionScene.addLine(0, 0, 100, 430, QPen(Qt::green, 3));
	ui.transferFunctionView->setScene( &_transferFunctionScene );
}