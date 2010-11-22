#include "interfaces.h"

void MeshLabInterface::Log(const char * f, ... )
{
  if(log)
  {
    char buf[4096];
    va_list marker;
    va_start( marker, f );
    vsprintf(buf,f,marker);
    va_end( marker );
    log->Log(GLLogStream::FILTER,buf);
  }
}


void MeshLabInterface::Log(int Level, const char * f, ... )
{
  if(log)
  {
    char buf[4096];
    va_list marker;
    va_start( marker, f );
    vsprintf(buf,f,marker);
    va_end( marker );
    log->Log(Level,buf);
  }
}

bool MeshFilterInterface::isFilterApplicable(QAction *act, const MeshModel& m, QStringList &MissingItems) const
{
  int preMask = getPreConditions(act);
  MissingItems.clear();

  if (preMask == MeshModel::MM_NONE) // no precondition specified.
    return true;

  if (preMask & MeshModel::MM_VERTCOLOR && !m.hasDataMask(MeshModel::MM_VERTCOLOR))
      MissingItems.push_back("Vertex Color");

  if (preMask & MeshModel::MM_FACECOLOR && !m.hasDataMask(MeshModel::MM_FACECOLOR))
      MissingItems.push_back("Face Color");

  if (preMask & MeshModel::MM_VERTQUALITY && !m.hasDataMask(MeshModel::MM_VERTQUALITY))
      MissingItems.push_back("Vertex Quality");

  if (preMask & MeshModel::MM_FACEQUALITY && !m.hasDataMask(MeshModel::MM_FACEQUALITY))
      MissingItems.push_back("Face Quality");

  if (preMask & MeshModel::MM_WEDGTEXCOORD && !m.hasDataMask(MeshModel::MM_WEDGTEXCOORD))
      MissingItems.push_back("Per Wedge Texture Coords");

  if (preMask & MeshModel::MM_VERTTEXCOORD && !m.hasDataMask(MeshModel::MM_VERTTEXCOORD))
      MissingItems.push_back("Per Vertex Texture Coords");

  if (preMask & MeshModel::MM_VERTRADIUS && !m.hasDataMask(MeshModel::MM_VERTRADIUS))
      MissingItems.push_back("Vertex Radius");

  if (preMask & MeshModel::MM_FACENUMBER && (m.cm.fn==0))
      MissingItems.push_back("Non empty Face Set");

  return MissingItems.isEmpty();
}


bool MeshLabFilterInterface::arePreCondsValid( const int filterPreConds,const MeshModel& m, QStringList &MissingItems )
{
	MissingItems.clear();

	if (filterPreConds == MeshModel::MM_NONE) // no precondition specified.
		return true;

	if (filterPreConds & MeshModel::MM_VERTCOLOR && !m.hasDataMask(MeshModel::MM_VERTCOLOR))
		MissingItems.push_back("Vertex Color");

	if (filterPreConds & MeshModel::MM_FACECOLOR && !m.hasDataMask(MeshModel::MM_FACECOLOR))
		MissingItems.push_back("Face Color");

	if (filterPreConds & MeshModel::MM_VERTQUALITY && !m.hasDataMask(MeshModel::MM_VERTQUALITY))
		MissingItems.push_back("Vertex Quality");

	if (filterPreConds & MeshModel::MM_FACEQUALITY && !m.hasDataMask(MeshModel::MM_FACEQUALITY))
		MissingItems.push_back("Face Quality");

	if (filterPreConds & MeshModel::MM_WEDGTEXCOORD && !m.hasDataMask(MeshModel::MM_WEDGTEXCOORD))
		MissingItems.push_back("Per Wedge Texture Coords");

	if (filterPreConds & MeshModel::MM_VERTTEXCOORD && !m.hasDataMask(MeshModel::MM_VERTTEXCOORD))
		MissingItems.push_back("Per Vertex Texture Coords");

	if (filterPreConds & MeshModel::MM_VERTRADIUS && !m.hasDataMask(MeshModel::MM_VERTRADIUS))
		MissingItems.push_back("Vertex Radius");

	if (filterPreConds & MeshModel::MM_FACENUMBER && (m.cm.fn==0))
		MissingItems.push_back("Non empty Face Set");

	return MissingItems.isEmpty();
}

int MeshLabFilterInterface::convertStringListToMeshElementEnum( const QStringList& stringListEnum )
{
	int res = 0;
	foreach(QString st,stringListEnum)
		res = res | (int) convertingMap[st];
	return res;
}

void MeshLabFilterInterface::initConvertingMap()
{
	convertingMap["MM_NONE"]       = MeshModel::MeshElement(MeshModel::MM_NONE);
	convertingMap["MM_VERTCOORD"]  = MeshModel::MeshElement(MeshModel::MM_VERTCOORD);
	convertingMap["MM_VERTNORMAL"] = MeshModel::MeshElement(MeshModel::MM_VERTNORMAL);
	convertingMap["MM_VERTFLAG"]   = MeshModel::MeshElement(MeshModel::MM_VERTFLAG);
	convertingMap["MM_VERTCOLOR"]  = MeshModel::MeshElement(MeshModel::MM_VERTCOLOR);
	convertingMap["MM_VERTQUALITY"]= MeshModel::MeshElement(MeshModel::MM_VERTQUALITY);
	convertingMap["MM_VERTMARK"]   = MeshModel::MeshElement(MeshModel::MM_VERTMARK);
	convertingMap["MM_VERTFACETOPO"]     = MeshModel::MeshElement(MeshModel::MM_VERTFACETOPO);
	convertingMap["MM_VERTCURV"]	        = MeshModel::MeshElement(MeshModel::MM_VERTCURV);
	convertingMap["MM_VERTCURVDIR"]      = MeshModel::MeshElement(MeshModel::MM_VERTCURVDIR);
	convertingMap["MM_VERTRADIUS"]	    = MeshModel::MeshElement(MeshModel::MM_VERTRADIUS);
	convertingMap["MM_VERTTEXCOORD"]     = MeshModel::MeshElement(MeshModel::MM_VERTTEXCOORD);
	convertingMap["MM_VERTNUMBER"]       = MeshModel::MeshElement(MeshModel::MM_VERTNUMBER);
	convertingMap["MM_FACEVERT"]         = MeshModel::MeshElement(MeshModel::MM_FACEVERT);
	convertingMap["MM_FACENORMAL"]       = MeshModel::MeshElement(MeshModel::MM_FACENORMAL);
	convertingMap["MM_FACEFLAG"]	        = MeshModel::MeshElement(MeshModel::MM_FACEFLAG);
	convertingMap["MM_FACECOLOR"]	        = MeshModel::MeshElement(MeshModel::MM_FACECOLOR);
	convertingMap["MM_FACEQUALITY"]      = MeshModel::MeshElement(MeshModel::MM_FACEQUALITY);
	convertingMap["MM_FACEMARK"]	        = MeshModel::MeshElement(MeshModel::MM_FACEMARK);
	convertingMap["MM_FACEFACETOPO"]     = MeshModel::MeshElement(MeshModel::MM_FACEFACETOPO);
	convertingMap["MM_FACENUMBER"]       = MeshModel::MeshElement(MeshModel::MM_FACENUMBER);
	convertingMap["MM_WEDGTEXCOORD"]     = MeshModel::MeshElement(MeshModel::MM_WEDGTEXCOORD);
	convertingMap["MM_WEDGNORMAL"]       = MeshModel::MeshElement(MeshModel::MM_WEDGNORMAL);
	convertingMap["MM_WEDGCOLOR"]	        = MeshModel::MeshElement(MeshModel::MM_WEDGCOLOR);
	convertingMap["MM_UNKNOWN"]          = MeshModel::MeshElement(MeshModel::MM_UNKNOWN);
	convertingMap["MM_VERTFLAGSELECT"]   = MeshModel::MeshElement(MeshModel::MM_VERTFLAGSELECT);
	convertingMap["MM_FACEFLAGSELECT"]   = MeshModel::MeshElement(MeshModel::MM_FACEFLAGSELECT);
	convertingMap["MM_VERTFLAGBORDER"]   =	MeshModel::MeshElement(MeshModel::MM_VERTFLAGBORDER);
	convertingMap["MM_FACEFLAGBORDER"]   = MeshModel::MeshElement(MeshModel::MM_FACEFLAGBORDER);
	convertingMap["MM_CAMERA"]			= MeshModel::MeshElement(MeshModel::MM_CAMERA);
	convertingMap["MM_TRANSFMATRIX"]     = MeshModel::MeshElement(MeshModel::MM_TRANSFMATRIX);
	convertingMap["MM_COLOR"]            = MeshModel::MeshElement(MeshModel::MM_COLOR);
	convertingMap["MM_POLYGONAL"]        = MeshModel::MeshElement(MeshModel::MM_POLYGONAL);
	convertingMap["MM_ALL"]				= MeshModel::MeshElement(MeshModel::MM_ALL);
}

MeshLabFilterInterface::MeshLabFilterInterface()
:MeshLabInterface()
{
	if (!init)
	{
		initConvertingMap();
		init = true;
	}
}

QMap<QString,MeshModel::MeshElement> MeshLabFilterInterface::convertingMap;

bool MeshLabFilterInterface::init = false;
