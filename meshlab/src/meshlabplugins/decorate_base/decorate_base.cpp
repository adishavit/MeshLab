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

#include "decorate_base.h"
#include <wrap/gl/addons.h>
#include <vcg/complex/trimesh/stat.h>
#include <meshlab/glarea.h>
#include <wrap/qt/checkGLError.h>
#include <wrap/qt/gl_label.h>

using namespace vcg;
using namespace std;


QString ExtraMeshDecoratePlugin::filterInfo(QAction *action) const
 {
  switch(ID(action))
  {
    case DP_SHOW_AXIS :									return tr("Draws XYZ axes in world coordinates");
    case DP_SHOW_BOX_CORNERS:						return tr("Draws object's bounding box corners");
		case DP_SHOW_VERT:									return tr("Draw the vertices of the mesh as round dots");
    case DP_SHOW_NON_FAUX_EDGE:								return tr("Draws the edge of the mesh that are on the boundary.");
		case DP_SHOW_BOX_CORNERS_ABS  :			return tr("Show Box Corners (Abs)");
    case DP_SHOW_VERT_NORMALS:					return tr("Draws object vertex normals");
    case DP_SHOW_VERT_PRINC_CURV_DIR :	return tr("Show Vertex Principal Curvature Directions");
    case DP_SHOW_FACE_NORMALS:					return tr("Draws object face normals");
    case DP_SHOW_QUOTED_BOX:						return tr("Draws quoted box");
    case DP_SHOW_VERT_LABEL:						return tr("Draws all the vertex indexes<br> Useful for debugging<br>(do not use it on large meshes)");
    case DP_SHOW_VERT_QUALITY_HISTOGRAM:						return tr("Draws a (colored) Histogram of the per vertex quality");
    case DP_SHOW_FACE_QUALITY_HISTOGRAM:						return tr("Draws a (colored) Histogram of the per face quality");
    case DP_SHOW_FACE_LABEL:						return tr("Draws all the face indexes, <br> Useful for debugging <br>(do not use it on large meshes)");
    case DP_SHOW_CAMERA:								return tr("Draw the position of the camera, if present in the current mesh");
    case DP_SHOW_TEXPARAM: return tr("Draw an overlayed flattened version of the current mesh that show the current parametrization");
	 }
  assert(0);
  return QString();
 }

QString ExtraMeshDecoratePlugin::filterName(FilterIDType filter) const
{
    switch(filter)
    {
    case DP_SHOW_VERT      :	return QString("Show Vertex Dots");
    case DP_SHOW_NON_FAUX_EDGE :	return QString("Show Non-Faux Edges");
    case DP_SHOW_VERT_NORMALS      :	return QString("Show Vertex Normals");
    case DP_SHOW_VERT_PRINC_CURV_DIR :	return QString("Show Vertex Principal Curvature Directions");
    case DP_SHOW_FACE_NORMALS      :	return QString("Show Face Normals");
    case DP_SHOW_BOX_CORNERS  :			return QString("Show Box Corners");
    case DP_SHOW_BOX_CORNERS_ABS  :		return QString("Show Box Corners (Abs)");
    case DP_SHOW_AXIS         :			return QString("Show Axis");
    case DP_SHOW_QUOTED_BOX		:	return QString("Show Quoted Box");
    case DP_SHOW_VERT_LABEL:		return tr("Show Vertex Label");
    case DP_SHOW_FACE_LABEL:			return tr("Show Face Label");
    case DP_SHOW_CAMERA:			return tr("Show Camera");
    case DP_SHOW_TEXPARAM:			return tr("Show UV Tex Param");
    case DP_SHOW_VERT_QUALITY_HISTOGRAM:			return tr("Show Vert Quality Histogram");
    case DP_SHOW_FACE_QUALITY_HISTOGRAM:			return tr("Show Face Quality Histogram");

    default: assert(0);
    }
    return QString("error!");
}

void ExtraMeshDecoratePlugin::decorate(QAction *a, MeshDocument &md, RichParameterSet *rm, GLArea *gla, QPainter *painter)
{
  MeshModel &m=*(md.mm());
  QFont qf;
	glPushMatrix();
	glMultMatrix(m.cm.Tr);
    switch (ID(a))
    {
    case DP_SHOW_FACE_NORMALS:
    case DP_SHOW_VERT_NORMALS:
    case DP_SHOW_VERT_PRINC_CURV_DIR:
        {
            glPushAttrib(GL_ENABLE_BIT );
            float NormalLen=rm->getFloat(NormalLength());
            float LineLen = m.cm.bbox.Diag()*NormalLen;
            CMeshO::VertexIterator vi;
            CMeshO::FaceIterator fi;
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_LINES);
            if(ID(a) == DP_SHOW_VERT_NORMALS)
			{
				glColor4f(.4f,.4f,1.f,.6f);
                for(vi=m.cm.vert.begin();vi!=m.cm.vert.end();++vi) if(!(*vi).IsD())
                {
                    glVertex((*vi).P());
                    glVertex((*vi).P()+(*vi).N()*LineLen);
                }
			}
            else
                if( ID(a) == DP_SHOW_VERT_PRINC_CURV_DIR){
                if(m.hasDataMask(MeshModel::MM_VERTCURVDIR))
                    for(vi=m.cm.vert.begin();vi!=m.cm.vert.end();++vi) if(!(*vi).IsD())
                    {
					glColor4f(1.0,0.0,0.0,.8f);
					glVertex((*vi).P());
					glColor4f(0.0,1.0,0.0,.1f);
					glVertex((*vi).P()+(*vi).PD1()*LineLen*0.25);
					glColor4f(0.0,1.0,0.0,.8f);
					glVertex((*vi).P());
					glColor4f(0.0,1.0,0.0,.1f);
					glVertex((*vi).P()+(*vi).PD2()*LineLen*0.25);
				}
            }
            else
                if(ID(a) == DP_SHOW_FACE_NORMALS)
                {
                glColor4f(.1f,.4f,4.f,.6f);
                for(fi=m.cm.face.begin();fi!=m.cm.face.end();++fi) if(!(*fi).IsD())
                {
                    Point3f b=Barycenter(*fi);
                    glVertex(b);
                    glVertex(b+(*fi).N()*LineLen);
                }
            }
			
            glEnd();
            glPopAttrib();
        } break;
    case DP_SHOW_BOX_CORNERS:	DrawBBoxCorner(m); break;
    case DP_SHOW_CAMERA:	DrawCamera(m,painter,qf);break;
    case DP_SHOW_QUOTED_BOX:		DrawQuotedBox(m,painter,qf);break;
    case DP_SHOW_VERT_LABEL:	DrawVertLabel(m,painter,qf);break;
    case DP_SHOW_FACE_LABEL:	DrawFaceLabel(m,painter,qf);break;
    case DP_SHOW_VERT:	{
			glPushAttrib(GL_ENABLE_BIT|GL_VIEWPORT_BIT|	  GL_CURRENT_BIT |  GL_DEPTH_BUFFER_BIT);
			glDisable(GL_LIGHTING);
			glEnable(GL_POINT_SMOOTH);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor(Color4b::Black);
			glDepthRange (0.0, 0.9999);
			glDepthFunc(GL_LEQUAL);
      float baseSize = rm->getDynamicFloat(this->VertDotSizeParam());

			glPointSize(baseSize+0.5);
			m.glw.DrawPointsBase<GLW::NMNone,GLW::CMNone>();
			glColor(Color4b::White);
			glPointSize(baseSize-1);
			m.glw.DrawPointsBase<GLW::NMNone,GLW::CMNone>();			
			glPopAttrib();
        } break;
    case DP_SHOW_NON_FAUX_EDGE :	{
			glPushAttrib(GL_ENABLE_BIT|GL_VIEWPORT_BIT|	  GL_CURRENT_BIT |  GL_DEPTH_BUFFER_BIT);
			glDisable(GL_LIGHTING);
			glDepthFunc(GL_LEQUAL);
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glLineWidth(1.f);
			glColor(Color4b::DarkGray);
			glDepthRange (0.0, 0.999);
			m.glw.DrawWirePolygonal<GLW::NMNone,GLW::CMNone>();
			glPopAttrib();
        } break;
    case DP_SHOW_TEXPARAM : this->DrawTexParam(m,gla,painter,rm,qf); break;

    case DP_SHOW_VERT_QUALITY_HISTOGRAM :
      {
        CMeshO::PerMeshAttributeHandle<CHist > qH = vcg::tri::Allocator<CMeshO>::GetPerMeshAttribute<CHist>(m.cm,"VertQualityHist");
        CHist &ch=qH();
        this->DrawColorHistogram(ch,gla, painter,rm,qf);
      }
      break;
    case DP_SHOW_FACE_QUALITY_HISTOGRAM :
      {
        CMeshO::PerMeshAttributeHandle<CHist > qH = vcg::tri::Allocator<CMeshO>::GetPerMeshAttribute<CHist>(m.cm,"FaceQualityHist");
        CHist &ch=qH();
        this->DrawColorHistogram(ch,gla, painter,rm,qf);
      }
      break;


    } // end switch;
	glPopMatrix();

  if(ID(a) == DP_SHOW_AXIS)	CoordinateFrame(m.cm.bbox.Diag()/2.0).Render(gla,painter);
  if(ID(a) == DP_SHOW_BOX_CORNERS_ABS)	DrawBBoxCorner(m,false);
}

void ExtraMeshDecoratePlugin::DrawQuotedBox(MeshModel &m,QPainter *gla,QFont qf)
{
	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT );
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);

	// Get gl state values
	double mm[16],mp[16];
	GLint vp[4];
	glGetDoublev(GL_PROJECTION_MATRIX,mp);
	glGetDoublev(GL_MODELVIEW_MATRIX,mm);
	glGetIntegerv(GL_VIEWPORT,vp);

	// Mesh boundingBox
	Box3f b(m.cm.bbox);
	glColor(Color4b::LightGray);
	glBoxWire(b);

	glLineWidth(1.f);
	glPointSize(3.f);

	Point3d p1,p2;

	Point3f c = b.Center();
		
	float s = 1.15f;
  const float LabelSpacing = 30;
	chooseX(b,mm,mp,vp,p1,p2);					// Selects x axis candidate
	glPushMatrix();
	glScalef(1,s,s);
	glTranslatef(0,c[1]/s-c[1],c[2]/s-c[2]);
	drawQuotedLine(p1,p2,b.min[0],b.max[0],CoordinateFrame::calcSlope(p1,p2,b.DimX(),LabelSpacing,mm,mp,vp),gla,qf);	// Draws x axis
	glPopMatrix();

	chooseY(b,mm,mp,vp,p1,p2);					// Selects y axis candidate
	glPushMatrix();
	glScalef(s,1,s);
	glTranslatef(c[0]/s-c[0],0,c[2]/s-c[2]);
	drawQuotedLine(p1,p2,b.min[1],b.max[1],CoordinateFrame::calcSlope(p1,p2,b.DimY(),LabelSpacing,mm,mp,vp),gla,qf);	// Draws y axis
	glPopMatrix();

	chooseZ(b,mm,mp,vp,p1,p2);					// Selects z axis candidate	
	glPushMatrix();
	glScalef(s,s,1);
	glTranslatef(c[0]/s-c[0],c[1]/s-c[1],0);
	drawQuotedLine(p1,p2,b.min[2],b.max[2],CoordinateFrame::calcSlope(p1,p2,b.DimZ(),LabelSpacing,mm,mp,vp),gla,qf);	// Draws z axis
	glPopMatrix();

	glPopAttrib();

}

void ExtraMeshDecoratePlugin::chooseX(Box3f &box,double *mm,double *mp,GLint *vp,Point3d &x1,Point3d &x2)
{
	float d = -std::numeric_limits<float>::max();
	Point3d c;
	// Project the bbox center
	gluProject(box.Center()[0],box.Center()[1],box.Center()[2],mm,mp,vp,&c[0],&c[1],&c[2]);
	c[2] = 0;

	Point3d out1,out2;
	Point3f in1,in2;

	for (int i=0;i<8;i+=2)
	{
		// find the furthest axis
		in1 = box.P(i);
		in2 = box.P(i+1);

		gluProject((double)in1[0],(double)in1[1],(double)in1[2],mm,mp,vp,&out1[0],&out1[1],&out1[2]);
		gluProject((double)in2[0],(double)in2[1],(double)in2[2],mm,mp,vp,&out2[0],&out2[1],&out2[2]);
		out1[2] = out2[2] = 0;

		float currDist = Distance(c,(out1+out2)*.5f);

		if(currDist > d)
		{
			d = currDist;
			x1.Import(in1);
			x2.Import(in2);
		}
	}
}


void ExtraMeshDecoratePlugin::chooseY(Box3f &box,double *mm,double *mp,GLint *vp,Point3d &y1,Point3d &y2)
{
	float d = -std::numeric_limits<float>::max();
	Point3d c;
	// Project the bbox center
	gluProject(box.Center()[0],box.Center()[1],box.Center()[2],mm,mp,vp,&c[0],&c[1],&c[2]);
	c[2] = 0;

	Point3d out1,out2;
	Point3f in1,in2;

	for (int i=0;i<6;++i)
	{
		if(i==2) i = 4;	// skip
		// find the furthest axis
		in1 = box.P(i);
		in2 = box.P(i+2);

		gluProject((double)in1[0],(double)in1[1],(double)in1[2],mm,mp,vp,&out1[0],&out1[1],&out1[2]);
		gluProject((double)in2[0],(double)in2[1],(double)in2[2],mm,mp,vp,&out2[0],&out2[1],&out2[2]);
		out1[2] = out2[2] = 0;

		float currDist = Distance(c,(out1+out2)*.5f);

		if(currDist > d)
		{
			d = currDist;
			y1.Import(in1);
			y2.Import(in2);
		}
	}
}

void ExtraMeshDecoratePlugin::chooseZ(Box3f &box,double *mm,double *mp,GLint *vp,Point3d &z1,Point3d &z2)
{
	float d = -std::numeric_limits<float>::max();
	Point3d c;
	// Project the bbox center
	gluProject(box.Center()[0],box.Center()[1],box.Center()[2],mm,mp,vp,&c[0],&c[1],&c[2]);
	c[2] = 0;

	Point3d out1,out2;
	Point3f in1,in2;

	Point3d m;

	for (int i=0;i<4;++i)
	{
		// find the furthest axis
		in1 = box.P(i);
		in2 = box.P(i+4);


		gluProject((double)in1[0],(double)in1[1],(double)in1[2],mm,mp,vp,&out1[0],&out1[1],&out1[2]);
		gluProject((double)in2[0],(double)in2[1],(double)in2[2],mm,mp,vp,&out2[0],&out2[1],&out2[2]);
		out1[2] = out2[2] = 0;

		float currDist = Distance(c,(out1+out2)*.5f);

		if(currDist > d)
		{
			d = currDist;
			z1.Import(in1);
			z2.Import(in2);
		}
	}
}
/**
  Draw a line with labeled ticks.
  \param a,b the two endpoints of the line (in 3D)
  \param aVal,bVal the two values at the line endpoints.
  \param tickScalarDistance the distance between labeled ticks ( 1/10 small ticks are added between them).

  for example if you want the a quoted line between 0.363 and 1.567 tickScalarDistance == 0.1 it means that
  you will have a line with labeled ticks at 0.4 0.5 etc.
 */
void ExtraMeshDecoratePlugin::drawQuotedLine(const Point3d &a,const Point3d &b, float aVal, float bVal, float tickScalarDistance, QPainter *painter, QFont qf,float angle,bool rightAlign)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHT0);
  glDisable(GL_NORMALIZE);
  float labelMargin =tickScalarDistance /4.0;
  float firstTick;
  // fmod returns the floating-point remainder of numerator/denominator (with the sign of the dividend)
  // fmod ( 104.5 , 10) returns 4.5     --> aVal - fmod(aval/tick) = 100
  // fmod ( -104.5 , 10) returns -4.5
  // So it holds that

  if(aVal > 0 ) firstTick = aVal - fmod(aVal,tickScalarDistance) + tickScalarDistance;
  if(aVal ==0 ) firstTick = tickScalarDistance;
  if(aVal < 0 ) firstTick = aVal + fmod(fabs(aVal),tickScalarDistance);

  // now we are sure that aVal < firstTick
  // let also be sure that there is enough space
  if ( (firstTick-aVal) < (labelMargin) )
    firstTick +=tickScalarDistance;


  float tickDistTen=tickScalarDistance /10.0f;
  float firstTickTen;
  if(aVal > 0) firstTickTen = aVal - fmod(aVal,tickDistTen) + tickDistTen;
          else firstTickTen = aVal - fmod(aVal,tickDistTen);          

  int neededZeros=0;
					
  Point3d Zero = a-((b-a)/(bVal-aVal))*aVal; // 3D Position of Zero.
	Point3d v(b-a);
  //v.Normalize();
  v = v*(1.0/(bVal-aVal));
  if(tickScalarDistance > 0)   // Draw lines only if the two endpoint are not coincident
	{
          neededZeros = ceil(max(0.0,-log10(double(tickScalarDistance))));
					glPointSize(3);
					float i;
					glBegin(GL_POINTS);
          for(i=firstTick;i<bVal;i+=tickScalarDistance)
						glVertex(Zero+v*i);
					glEnd();
          for(i=firstTick; (i+labelMargin)<bVal;i+=tickScalarDistance)
            glLabel::render(painter,Point3f::Construct(Zero+v*i),tr("%1 ").arg(i,4+neededZeros,'f',neededZeros),qf,Color4b::White,angle,rightAlign);
				 glPointSize(1);
				 glBegin(GL_POINTS);
            for(i=firstTickTen;i<bVal;i+=tickDistTen)
							glVertex(Zero+v*i);
				 glEnd();
	}

	// Draws bigger ticks at 0 and at max size
  glPointSize(6);
	
	glBegin(GL_POINTS);
			glVertex(a);	
			glVertex(b);
      if(bVal*aVal<0) glVertex(Zero);
	glEnd();


	// bold font at beginning and at the end
  qf.setBold(true);
  glLabel::render(painter,Point3f::Construct(a), tr("%1").arg(aVal,6+neededZeros,'f',neededZeros+2) ,qf,Color4b::White,angle,rightAlign);
  glLabel::render(painter,Point3f::Construct(b), tr("%1").arg(bVal,6+neededZeros,'f',neededZeros+2) ,qf,Color4b::White,angle,rightAlign);

  glPopAttrib();
}


float ExtraMeshDecoratePlugin::niceRound2(float Val,float base)	{return powf(base,ceil(log10(Val)/log10(base)));}
float ExtraMeshDecoratePlugin::niceRound(float val)	{return powf(10.f,ceil(log10(val)));}

void ExtraMeshDecoratePlugin::DrawBBoxCorner(MeshModel &m, bool absBBoxFlag)
{
	glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_POINT_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT );
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.0);
	glColor(Color4b::Cyan);
	Box3f b;
	if(absBBoxFlag) {
			b=m.cm.bbox;
			glColor(Color4b::Cyan);
	} else {
			b=m.cm.trBB();
			glColor(Color4b::Green);
	}
	Point3f mi=b.min;
	Point3f ma=b.max;
	Point3f d3=(b.max-b.min)/4.0;
	Point3f zz(0,0,0);
	glBegin(GL_LINES);
	glVertex3f(mi[0],mi[1],mi[2]); glVertex3f(mi[0]+d3[0],mi[1]+zz[1],mi[2]+zz[2]);
	glVertex3f(mi[0],mi[1],mi[2]); glVertex3f(mi[0]+zz[0],mi[1]+d3[1],mi[2]+zz[2]);
	glVertex3f(mi[0],mi[1],mi[2]); glVertex3f(mi[0]+zz[0],mi[1]+zz[1],mi[2]+d3[2]);

	glVertex3f(ma[0],mi[1],mi[2]); glVertex3f(ma[0]-d3[0],mi[1]+zz[1],mi[2]+zz[2]);
	glVertex3f(ma[0],mi[1],mi[2]); glVertex3f(ma[0]+zz[0],mi[1]+d3[1],mi[2]+zz[2]);
	glVertex3f(ma[0],mi[1],mi[2]); glVertex3f(ma[0]+zz[0],mi[1]+zz[1],mi[2]+d3[2]);

  glVertex3f(mi[0],ma[1],mi[2]); glVertex3f(mi[0]+d3[0],ma[1]+zz[1],mi[2]+zz[2]);
	glVertex3f(mi[0],ma[1],mi[2]); glVertex3f(mi[0]+zz[0],ma[1]-d3[1],mi[2]+zz[2]);
	glVertex3f(mi[0],ma[1],mi[2]); glVertex3f(mi[0]+zz[0],ma[1]+zz[1],mi[2]+d3[2]);

	glVertex3f(ma[0],ma[1],mi[2]); glVertex3f(ma[0]-d3[0],ma[1]+zz[1],mi[2]+zz[2]);
	glVertex3f(ma[0],ma[1],mi[2]); glVertex3f(ma[0]+zz[0],ma[1]-d3[1],mi[2]+zz[2]);
	glVertex3f(ma[0],ma[1],mi[2]); glVertex3f(ma[0]+zz[0],ma[1]+zz[1],mi[2]+d3[2]);

	glVertex3f(mi[0],mi[1],ma[2]); glVertex3f(mi[0]+d3[0],mi[1]+zz[1],ma[2]+zz[2]);
	glVertex3f(mi[0],mi[1],ma[2]); glVertex3f(mi[0]+zz[0],mi[1]+d3[1],ma[2]+zz[2]);
	glVertex3f(mi[0],mi[1],ma[2]); glVertex3f(mi[0]+zz[0],mi[1]+zz[1],ma[2]-d3[2]);

	glVertex3f(ma[0],mi[1],ma[2]); glVertex3f(ma[0]-d3[0],mi[1]+zz[1],ma[2]+zz[2]);
	glVertex3f(ma[0],mi[1],ma[2]); glVertex3f(ma[0]+zz[0],mi[1]+d3[1],ma[2]+zz[2]);
	glVertex3f(ma[0],mi[1],ma[2]); glVertex3f(ma[0]+zz[0],mi[1]+zz[1],ma[2]-d3[2]);

	glVertex3f(mi[0],ma[1],ma[2]); glVertex3f(mi[0]+d3[0],ma[1]+zz[1],ma[2]+zz[2]);
	glVertex3f(mi[0],ma[1],ma[2]); glVertex3f(mi[0]+zz[0],ma[1]-d3[1],ma[2]+zz[2]);
	glVertex3f(mi[0],ma[1],ma[2]); glVertex3f(mi[0]+zz[0],ma[1]+zz[1],ma[2]-d3[2]);

	glVertex3f(ma[0],ma[1],ma[2]); glVertex3f(ma[0]-d3[0],ma[1]+zz[1],ma[2]+zz[2]);
	glVertex3f(ma[0],ma[1],ma[2]); glVertex3f(ma[0]+zz[0],ma[1]-d3[1],ma[2]+zz[2]);
	glVertex3f(ma[0],ma[1],ma[2]); glVertex3f(ma[0]+zz[0],ma[1]+zz[1],ma[2]-d3[2]);

	glEnd();
	glPopAttrib();
}


bool ExtraMeshDecoratePlugin::isDecorationApplicable(QAction *action, const MeshModel& m, QString &ErrorMessage) const
{
  if( ID(action) == DP_SHOW_VERT_LABEL || ID(action) == DP_SHOW_FACE_LABEL)
        {
          if(m.cm.vn <1000 && m.cm.fn<2000) return true;
          else {
            ErrorMessage=QString("Warning: the mesh contains many faces and vertices.<br>Printing on the screen thousand of numbers is useless and VERY SLOW <br> Do you REALLY want this? ");
            return false;
          }
        }
  if(ID(action) == DP_SHOW_FACE_QUALITY_HISTOGRAM ) return m.hasDataMask(MeshModel::MM_FACEQUALITY);
  if(ID(action) == DP_SHOW_VERT_QUALITY_HISTOGRAM ) return m.hasDataMask(MeshModel::MM_VERTQUALITY);

  if( ID(action) == DP_SHOW_VERT_PRINC_CURV_DIR )
  {
    if(!m.hasDataMask(MeshModel::MM_VERTCURVDIR)) return false;
  }
    if( ID(action) == DP_SHOW_TEXPARAM )
    {
        if(!m.hasDataMask(MeshModel::MM_WEDGTEXCOORD)) return false;
    }
  return true;
}


bool ExtraMeshDecoratePlugin::startDecorate(QAction * action, MeshDocument &md, RichParameterSet *rm, GLArea *)
{	
  switch(ID(action))
  {
    case DP_SHOW_VERT_QUALITY_HISTOGRAM :
      {
        MeshModel *m=md.mm();

        CMeshO::PerMeshAttributeHandle<CHist > qH = vcg::tri::Allocator<CMeshO>::GetPerMeshAttribute<CHist>(m->cm,"VertQualityHist");

            if(!vcg::tri::Allocator<CMeshO>::IsValidHandle(m->cm,qH))
              qH=vcg::tri::Allocator<CMeshO>::AddPerMeshAttribute<CHist>  (m->cm,std::string("VertQualityHist"));
            CHist *H = &qH();
            std::pair<float,float> minmax = tri::Stat<CMeshO>::ComputePerVertexQualityMinMax(m->cm);
            if(rm->getBool(UseFixedHistParam())) {
              minmax.first=rm->getFloat(FixedHistMinParam());
              minmax.second=rm->getFloat(FixedHistMaxParam());
            }

            H->SetRange( minmax.first, minmax.second, rm->getInt(HistBinNumParam()));
            if(rm->getBool(AreaHistParam()))
            {
              for(CMeshO::FaceIterator fi = m->cm.face.begin(); fi!= m->cm.face.end();++fi) if(!(*fi).IsD())
                {
                  float area6=DoubleArea(*fi)/6.0f;
                  for(int i=0;i<3;++i)
                      H->Add((*fi).V(i)->Q(),(*fi).V(i)->C(),area6);
                }
            } else {
              for(CMeshO::VertexIterator vi = m->cm.vert.begin(); vi!= m->cm.vert.end();++vi) if(!(*vi).IsD())
                {
                  H->Add((*vi).Q(),(*vi).C(),1.0f);
                }
            }
      } break;
    case DP_SHOW_FACE_QUALITY_HISTOGRAM :
      {
        MeshModel *m=md.mm();

        CMeshO::PerMeshAttributeHandle<CHist > qH = vcg::tri::Allocator<CMeshO>::GetPerMeshAttribute<CHist>(m->cm,"FaceQualityHist");

            if(!vcg::tri::Allocator<CMeshO>::IsValidHandle(m->cm,qH))
              qH=vcg::tri::Allocator<CMeshO>::AddPerMeshAttribute<CHist>  (m->cm,std::string("FaceQualityHist"));
            CHist *H = &qH();
            std::pair<float,float> minmax = tri::Stat<CMeshO>::ComputePerFaceQualityMinMax(m->cm);

            if(rm->getBool(UseFixedHistParam())) {
              minmax.first=rm->getFloat(FixedHistMinParam());
              minmax.second=rm->getFloat(FixedHistMaxParam());
            }

            H->SetRange( minmax.first,minmax.second, rm->getInt(HistBinNumParam()));
            if(rm->getBool(AreaHistParam())) {
              for(CMeshO::FaceIterator fi = m->cm.face.begin(); fi!= m->cm.face.end();++fi) if(!(*fi).IsD())
                  H->Add((*fi).Q(),(*fi).C(),DoubleArea(*fi)*0.5f);
            } else {
              for(CMeshO::FaceIterator fi = m->cm.face.begin(); fi!= m->cm.face.end();++fi) if(!(*fi).IsD())
                  H->Add((*fi).Q(),(*fi).C(),1.0f);
          }
      } break;

  }
 return true;
}

void ExtraMeshDecoratePlugin::DrawFaceLabel(MeshModel &m, QPainter *painter, QFont qf)
{
	glPushAttrib(GL_LIGHTING_BIT  | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT );
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_LIGHTING);
	glColor3f(.4f,.4f,.4f);
				for(size_t i=0;i<m.cm.face.size();++i)
					if(!m.cm.face[i].IsD())
							{
								Point3f bar=Barycenter(m.cm.face[i]);
                glLabel::render(painter, bar,tr("%1").arg(i));
							}
	glPopAttrib();
}


void ExtraMeshDecoratePlugin::DrawVertLabel(MeshModel &m,QPainter *painter, QFont qf)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT );
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_LIGHTING);
	glColor3f(.4f,.4f,.4f);
    for(size_t i=0;i<m.cm.vert.size();++i){
          if(!m.cm.vert[i].IsD())
                glLabel::render(painter, m.cm.vert[i].P(),tr("%1").arg(i));
    }
	glPopAttrib();			
}

void ExtraMeshDecoratePlugin::DrawCamera(MeshModel &m,QPainter *painter, QFont qf)
{
  if(!m.cm.shot.IsValid()) return;

  glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT );
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_LIGHTING);
	glColor3f(.8f,.8f,.8f);
	
	Point3f vp = m.cm.shot.GetViewPoint();
		
	float len = m.cm.bbox.Diag()/20.0;
	 	glBegin(GL_LINES);
			glVertex3f(vp[0]-len,vp[1],vp[2]); 	glVertex3f(vp[0]+len,vp[1],vp[2]); 
			glVertex3f(vp[0],vp[1]-len,vp[2]); 	glVertex3f(vp[0],vp[1]+len,vp[2]); 
			glVertex3f(vp[0],vp[1],vp[2]-len); 	glVertex3f(vp[0],vp[1],vp[2]+len); 
		glEnd();
	glPopAttrib();			
}
void ExtraMeshDecoratePlugin::DrawColorHistogram(CHist &ch, GLArea *gla, QPainter *painter, RichParameterSet *par, QFont qf)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  float ratio = float(gla->width())/gla->height();
  glOrtho(0,ratio,0,1,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  
  float len = ch.MaxV() - ch.MinV();
  float maxWide = ch.MaxCount();
  float histWide=maxWide;
  if(par->getBool(this->UseFixedHistParam()))
    histWide = par->getFloat(this->FixedHistWidthParam());
  float bn = ch.BinNum();

  float border = 0.1;
  float histH = 1.0f - 2.f*border;
  float histW = 0.3f;

  glBegin(GL_QUAD_STRIP);
  for(float i =0;i<bn;++i)
  {
    float val = ch.MinV() + (i/bn)*(ch.MaxV() - ch.MinV());
    float wide = histW *float(ch.BinCount(val))/histWide;
    wide= std::min(0.5f,wide);
    float ypos  = ( i   /bn)*histH;
    float ypos2 = ((i+1)/bn)*histH;

    glColor(ch.BinColorAvg(val));
    glVertex3f(border,    border+ypos,0);
    glVertex3f(border+wide, border+ypos,0);
    glVertex3f(border,   border+ypos2,0);
    glVertex3f(border+wide,border+ypos2,0);
  }

  glEnd();

  glColor(Color4b::White);
  drawQuotedLine(Point3d(border*4/5.0,border,0),Point3d(border*4/5.0,1.0-border,0),ch.MinV(),ch.MaxV(),len/20.0,painter,qf,0,true);
  glLabel::render(painter,Point3f(border,1-border*0.5,0),QString("MinV %1 MaxV %2 MaxC %3").arg(ch.MinElem()).arg(ch.MaxElem()).arg(maxWide));
  // Closing 2D
  glPopAttrib();
  glPopMatrix(); // restore modelview
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void ExtraMeshDecoratePlugin::DrawTexParam(MeshModel &m, GLArea *gla, QPainter *painter,  RichParameterSet *rm, QFont qf)
{
  if(!m.hasDataMask(MeshModel::MM_WEDGTEXCOORD)) return;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    float ratio = float(gla->width())/gla->height();
    glOrtho(-ratio,ratio,-1,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(ratio-1.0,0.0f,0.0f);
    glScalef(0.9f,0.9f,0.9f);

    QString textureName("-- no texture --");
    if(!m.glw.TMId.empty())
      textureName = qPrintable(QString(m.cm.textures[0].c_str()))+QString("  ");
    //, QPainter *qDebug(qPrintable(textureName));
    glLabel::render(painter,Point3f(0.0,-0.10,0.0),textureName);
    checkGLError::qDebug("DrawTexParam");
    drawQuotedLine(Point3d(0,0,0),Point3d(0,1,0),0,1,0.1,painter,qf,0,true);
    drawQuotedLine(Point3d(0,0,0),Point3d(1,0,0),0,1,0.1,painter,qf,90.0f);


    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if( rm->getBool(this->TextureStyleParam()) )
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                           else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glEnable(GL_TEXTURE_2D);
      glBindTexture( GL_TEXTURE_2D, m.glw.TMId.back() );
      glBegin(GL_TRIANGLES);
      for(size_t i=0;i<m.cm.face.size();++i)
        if(!m.cm.face[i].IsD())
        {
        glTexCoord(m.cm.face[i].WT(0).P());
        glVertex(m.cm.face[i].WT(0).P());
        glTexCoord(m.cm.face[i].WT(1).P());
        glVertex(m.cm.face[i].WT(1).P());
        glTexCoord(m.cm.face[i].WT(2).P());
        glVertex(m.cm.face[i].WT(2).P());
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Closing 2D
    glPopAttrib();

    glPopMatrix(); // restore modelview
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

}

void ExtraMeshDecoratePlugin::initGlobalParameterSet(QAction *action, RichParameterSet &parset)
{
    switch(ID(action)){
    case DP_SHOW_TEXPARAM : {
            assert(!parset.hasParameter(TextureStyleParam()));
            parset.addParam(new RichBool(TextureStyleParam(), true,"Texture Param Wire","if true the parametrization is drawn in a textured wireframe style"));
        } break;
    case DP_SHOW_VERT : {
            assert(!parset.hasParameter(VertDotSizeParam()));
            parset.addParam(new RichDynamicFloat(VertDotSizeParam(), 4,2,8,"Dot Size","if true the parametrization is drawn in a textured wireframe style"));
        } break;
    case DP_SHOW_FACE_NORMALS :
    case DP_SHOW_VERT_NORMALS :{
        if(!parset.hasParameter(NormalLength()))
        {
          parset.addParam(new RichFloat(NormalLength(),0.05,"Normal Length","The length of the normal expressed as a percentage of the bbox of the mesh"));
        }
      } break;
    case DP_SHOW_FACE_QUALITY_HISTOGRAM :
    case DP_SHOW_VERT_QUALITY_HISTOGRAM :{
        if(!parset.hasParameter(HistBinNumParam()))
        {
          parset.addParam(new RichInt(HistBinNumParam(), 256,"Histogram Bins","If true the parametrization is drawn in a textured wireframe style"));
          parset.addParam(new RichBool(AreaHistParam(), false,"Area Weighted","If true the histogram is computed according to the surface of the involved elements.<br>"
                                       "e.g. each face contribute to the histogram proportionally to its area and each vertex with 1/3 of sum of the areas of the incident triangles."));
          parset.addParam(new RichBool(UseFixedHistParam(), false,"Fixed Histogram width","if true the parametrization is drawn in a textured wireframe style"));
          parset.addParam(new RichFloat(FixedHistMinParam(), 0,"Min Hist Value","Used only if the Fixed Histogram Width Parameter is checked"));
          parset.addParam(new RichFloat(FixedHistMaxParam(), 0,"Max Hist Value","Used only if the Fixed Histogram Width Parameter is checked"));
          parset.addParam(new RichFloat(FixedHistWidthParam(), 0,"Hist Width","If not zero, this value is used to scale histogram width  so that it is the indicated value.<br>"
                                                                              "Useful only if you have to compare multiple histograms.<br>"
                                                                              "Warning, with wrong values the histogram can become excessively flat or it can overflow"));
        }

    } break;
    }
}
Q_EXPORT_PLUGIN(ExtraMeshDecoratePlugin)
