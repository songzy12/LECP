#ifndef MESH_H
#define MESH_H
#include "vertex.h"
#include "half_edge.h"
#include "polygon.h"
#include "paint.h"
#include <list>

using namespace std;

class Mesh
{
public:
	Mesh();
	~Mesh();

	list<Vertex*> vertices() { return vertices_; }
	list<HalfEdge*> edges() { return half_edges_; }

	//the first is the intersection point, and the second is the corresponding point user input.
	vector<pair<LECP_Point*,LECP_Point*>> AddLine(LECP_Point *point);// return the intersections

	void init();//��ʼ����bounding box, one face,4 half_edge,4 vertex
	void clear();
	HalfEdge*  getIntersectHalfEdge(double a,double b);
	Vertex* intersectWithBoundingBox(HalfEdge* tmp,double a,double b);

	//�õ��²����ֱ�ߺ͵�ǰ��Ľ���.
	//��newHalfEdge��next��ʼ�ж��Ƿ��н���
	//���ص�newIntersection�б��潻��Vertex
	//���ص�intersectHalfEdge���浱ǰface������half_edge��ֱ��y=ax-b�ཻ
	HalfEdge* getIntersection(double a, double b, HalfEdge newHalf, Vertex &newIntersection);
	bool onBoundingBox(Vertex* newIntersection);

	HalfEdge* getIntersectBoundingBox(double a, double b, Vertex &vertex);

	//poH��neH����һ���������±߶�Ӧ������half_edge,����һ����Ҫ����ĳЩ�ֶ�
	//newPoH��new neH�Ǹò��²����ı߶�Ӧ������half_edge
	void dealWithNormalIntersection(Vertex* newIntersection, HalfEdge* intersectHalfEdgeLeft, HalfEdge* intersectHalfEdgeRight, HalfEdge* newPoH, HalfEdge* newNeH);//�����м�Ľ���

	//void  preprocessingPolarAngleSort();
	void  postCalcPolarAngle();

	void postAjustIntersections(list<LECP_Point*>  &return_intersections);// 2017-05-24���
	void postAjustIntersections(vector<pair<LECP_Point*, LECP_Point*>>  &return_intersections);// 2017-05-24���

	// Added by zyx,2017-05-25
	// return the new halfEdge
	HalfEdge* splitEdge(HalfEdge* half_edge,Vertex* vertex);
	void connectTwoNewVertices(HalfEdge* h1,HalfEdge* h2,LECP_Point *point);

	//���²����ֱ�߹��ɵĽ���list��ŵ�sortedPoint�У����������㹲x�����
	void addCurrentAngleSortedResultToVector(LECP_Point *point, vector<pair<LECP_Point*, LECP_Point*>> lecp_points, vector<LECP_Point*> points);

private:
	list<Vertex*> vertices_;
	list<HalfEdge*> half_edges_;

	list<HalfEdge*> boundingBox;//����߽��HalfEdge,����ȷ���²����ֱ�����ȴ����ĸ�face

public:
	vector<list<Vertex*>> sortedVector;
	vector<list<LECP_Point*>> sortedPoint;
	vector<LECP_Point*> points;
};

#endif

