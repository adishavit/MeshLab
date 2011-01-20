#ifndef __MESHLABDOC_XML_H
#define __MESHLABDOC_XML_H

QDomDocument MeshDocumentToXML(MeshDocument &md);
bool MeshDocumentToXMLFile(MeshDocument &md, QString filename);

bool MeshDocumentFromXML(MeshDocument &md, QString filename);
QDomElement RasterModelToXML(RasterModel *mp,QDomDocument &doc);
QDomElement PlaneToXML(Plane* pl,QDomDocument& doc);
#endif // __MESHLABDOC_XML_H
