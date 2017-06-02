#include "mesh.h"
#include <cstdio>
#include <list>
#include <util.h>
#include <qDebug.h>
using namespace std;

#define  INFINITY  999999999999
#define  DELTA 0.000000000000001

Mesh::Mesh() {
	half_edges_ = list<HalfEdge*>();
	vertices_ = list<Vertex*>();

	init();//DCEL��ʼ��
}

Mesh::~Mesh() {
	clear();
}
void Mesh ::clear(){
	
	for (HalfEdge *e : half_edges_)
		delete e;
	half_edges_.clear();

	// clear bounding box
	boundingBox.clear();

	for (Vertex *v : vertices_)
		delete v;
	vertices_.clear();

	for (list<Vertex*> l : sortedVector){
		for (Vertex* v : l){
			delete v;
		}
		l.clear();
	}
	sortedVector.clear();
	/*
	if (sortedPoint.size() > 0){
		for (LECP_Point* v : sortedPoint[sortedPoint.size() - 1]){
			delete v;
		}
		sortedPoint.clear();
	}*/

	sortedPoint.clear();
	
	init(); // init bounding box
}

/*simple version: add new vertex at the end of the vertex list*/
void Mesh::AddVertex(Vertex *v) {
	vertices_.push_back(v);
}

void Mesh::ConnectVertices(Vertex *v1, Vertex *v2) {

}

//��Ļ������һ���㣬��Ӧ�Ķ�żͼ������һ����
vector<pair<LECP_Point*, LECP_Point*>>  Mesh::AddLine(LECP_Point *point){
	//The first is intersection,and the second is corresponding original user input point.
	vector<pair<LECP_Point*, LECP_Point*>> return_intersections;

	double a = point->x;
	double b = point->y;

	//qDebug() << "AddLine:" << a << b;

	Vertex vRet;

	//�ҵ��²����ֱ�ߺ�bounding box���ĸ��߶��ཻ
	HalfEdge *firstIntersect = getIntersectBundingBox(a, b, vRet);

	//�ҵ����㣬���Ҹ���boudingBox
	Vertex* v = new Vertex();
	v->set_point(vRet.point());

	//qDebug() << "vRet: " << vRet.point().first << vRet.point().second;
	
	HalfEdge* newHalf = splitEdge(firstIntersect,v); // belongs to bounding box
	boundingBox.push_back(newHalf);

	vertices_.push_back(v);

	// end intersection with bouding box

	//���������ֱ�ߺ�ƽ��������ֱ�ߵ�һ����һ���Ľ��㣬֪�����Ľ�������bounding box�Ͻ���

	Vertex* newIntersection = new Vertex();
	HalfEdge* intersectHalfEdgeLeft;//ԭ����
	
	// right��ʱ������
	intersectHalfEdgeLeft = getIntersection(a, b, *newHalf, *newIntersection);//����ֱ�ߺ������н���İ�ߺͽ�������

	HalfEdge* intersectHalfEdgeRight = splitEdge(intersectHalfEdgeLeft,newIntersection);

	connectTwoNewVertices(newHalf,intersectHalfEdgeRight,point);

	// for polar angle sort 
	// TODO: check whether intersectHalfEdgeLeft->lecp_point is set?
	intersectHalfEdgeRight->lecp_point = intersectHalfEdgeLeft->lecp_point;

	//�жϵ�ǰ�Ľ����Ƿ���bounding box��
	bool onBB = onBoundingBox(newIntersection);

	while (!onBB){ //��ѭ��
		//---------------start add new pair--------------------------------------------------------
		// add a new intersection vertex to the return vector.
		LECP_Point* first = new LECP_Point();
		first->setX(newIntersection->point().first);
		first->setY(newIntersection->point().second);

		LECP_Point* second = intersectHalfEdgeRight->lecp_point;

		//qDebug() << "first" << newIntersection->point().first << newIntersection->point().first;
		//qDebug() << "second:" << (intersectHalfEdgeRight->lecp_point)->x << (intersectHalfEdgeRight->lecp_point)->y;

		pair<LECP_Point*, LECP_Point*> intersectionPair;
		intersectionPair.first = first;
		intersectionPair.second = second;

		return_intersections.push_back(intersectionPair);
		//-----------end add new pair--------------------------------------------------

		//sortedAngle.push_back(intersectHalfEdgeLeft->lecp_point);

		// return tmpV and left,right
		newHalf = intersectHalfEdgeLeft->twin();

		newHalf->lecp_point = intersectHalfEdgeLeft->lecp_point;

		Vertex tmpV;

		intersectHalfEdgeLeft = getIntersection(a, b, *newHalf, tmpV);

		HalfEdge *newRight = new HalfEdge();
		// TODO: here is where everything begins
		intersectHalfEdgeLeft = getIntersection(a, b, *newHalf, tmpV);

		// for polar angle sort
		// TODO: newRight->lecp_point may not be set
		newRight->lecp_point = intersectHalfEdgeLeft->lecp_point;


		Vertex *intersection = new Vertex();
		intersection->set_point(tmpV.point());

	    newRight = splitEdge(intersectHalfEdgeLeft,intersection);

		// for polar angle sort
		newRight->lecp_point = intersectHalfEdgeLeft->lecp_point;

	    connectTwoNewVertices(newHalf,newRight,point);

		//�жϵ�ǰ�Ľ����Ƿ���bounding box��
		onBB = onBoundingBox(intersection);

		//���±���
		intersectHalfEdgeRight = newRight;
		newIntersection = intersection;
	}

	//���һ������
	intersectHalfEdgeRight->set_twin(NULL);
	boundingBox.push_back(intersectHalfEdgeRight);

	// �����˳����������෴�����²����ֱ�����ĸ�bounding box�����ཻ�йأ�ÿ���²����ֱ�߶�������bounding box�ཻ��

	//postAjustIntersections(sortedAngle);// 2017-05-24���
	postAjustIntersections(return_intersections);// 2017-05-24���

	//save current kernel's polar angle sorted result
	//sortedPoint.push_back(sortedAngle);

	return return_intersections;
}

// 2017-05-24���
void Mesh::postAjustIntersections(list<LECP_Point*>  &return_intersections){
	if (return_intersections.size() <= 2)
		return;

	vector<LECP_Point*> tmpVector( return_intersections.begin(), return_intersections.end() );
	
	LECP_Point* p1 = tmpVector[1];
	LECP_Point* p2 = tmpVector[2];
	if (p1->x > p2->x){
		reverse(return_intersections.begin(), return_intersections.end());
	}
	else if (p1->x == p2->x && p1->y > p2->y){
		reverse(++return_intersections.begin(), return_intersections.end());
	}
}

// 2017-05-24���
void Mesh::postAjustIntersections(vector<pair<LECP_Point*, LECP_Point*>>  &return_intersections){
	if (return_intersections.size() < 2)
		return;

	LECP_Point* p1 = return_intersections[0].first;
	LECP_Point* p2 = return_intersections[1].first;
	if (p1->x > p2->x){
		reverse(return_intersections.begin(), return_intersections.end());
	}
	else if (p1->x == p2->x && p1->y > p2->y){
		reverse(return_intersections.begin(), return_intersections.end());
	}
}

void Mesh::init(){
	//init four vertex, which are the four window points. 

	double half_x = MAX_X;
	double half_y = MAX_Y;

	Vertex *v1 = new Vertex();
	pair<double, double> p1;
	p1.first = -half_x;
	p1.second = half_y;
	v1->set_point(p1);

	Vertex *v2 = new Vertex();
	pair<double, double> p2;
	p2.first = half_x;
	p2.second = half_y;
	v2->set_point(p2);

	Vertex *v3 = new Vertex();
	pair<double, double> p3;
	p3.first = half_x;
	p3.second = -half_y;
	v3->set_point(p3);

	Vertex *v4 = new Vertex();
	pair<double, double> p4;
	p4.first = -half_x;
	p4.second = -half_y;
	v4->set_point(p4);

	// four half_edge
	HalfEdge *h1 = new HalfEdge();
	h1->set_origin(v1);

	HalfEdge *h2 = new HalfEdge();
	h2->set_origin(v2);

	HalfEdge *h3 = new HalfEdge();
	h3->set_origin(v3);

	HalfEdge *h4 = new HalfEdge();
	h4->set_origin(v4);

	// set the half_edges of each vertex
	v1->set_half_edge(h1);
	v2->set_half_edge(h2);
	v3->set_half_edge(h3);
	v4->set_half_edge(h4);

	//relation between the four half edges
	h1->set_next(h2);
	h1->set_prev(h4);
	h1->set_twin(NULL);

	h2->set_next(h3);
	h2->set_prev(h1);
	h2->set_twin(NULL);

	h3->set_next(h4);
	h3->set_prev(h2);
	h3->set_twin(NULL);

	h4->set_next(h1);
	h4->set_prev(h3);
	h4->set_twin(NULL);

	vertices_.push_back(v1);
	vertices_.push_back(v2);
	vertices_.push_back(v3);
	vertices_.push_back(v4);

	half_edges_.push_back(h1);
	half_edges_.push_back(h2);
	half_edges_.push_back(h3);
	half_edges_.push_back(h4);

	boundingBox.push_back(h1);
	boundingBox.push_back(h2);
	boundingBox.push_back(h3);
	boundingBox.push_back(h4);
}

HalfEdge*  Mesh::getIntersectHalfEdge(double a, double b){
	list<HalfEdge*>::iterator it = half_edges_.begin();

	while (it != half_edges_.end()){
		HalfEdge* tmp = *it++;

		//�жϰ����ֱ���Ƿ��н���
		Vertex* intersect = intersectWithBoundingBox(tmp,a,b);
		if (intersect!=NULL){
			return tmp;
		}
	}

	return NULL;
}

//�ж�ֱ��y=ax-b����tmp�Ƿ��н���,����У��򷵻ؽ��㣬����ΪNULL
Vertex*  Mesh::intersectWithBoundingBox(HalfEdge* tmp, double a, double b){
	double half_x = MAX_X;
	double half_y = MAX_Y;

	//�жϵ�ǰhalf_edge���ڵ�boudingBox�ı�
	double x1 = tmp->origin()->point().first;
	double y1 = tmp->origin()->point().second;

	double x2 = tmp->next()->origin()->point().first;
	double y2 = tmp->next()->origin()->point().second;

	Vertex* re = new Vertex();


	//4�����
	if (x1 == x2 && x1 == -half_x){//���߽� �߶�x=-half_x

		double tmpY = a* x1 - b;

		if ((tmpY >= y1 && tmpY <= y2) || (tmpY <= y1 && tmpY >= y2))
		{
			pair<double, double> p;
			p.first = -half_x;
			p.second = a*x1 - b;
			re->set_point(p);


			return re;
		}

	}
	else if (x1 == x2 && x1 == half_x){ // x=half_x
		double y = a*x1 - b;
		if ((y >= y1 && y <= y2) || (y <= y1 && y >= y2))
		{
			pair<double, double> p;
			p.first = x1;
			p.second = y;
			re->set_point(p);

			return re;
		}

	}
	else if (y1 == y2 && y1 == -half_y){// y = -half_y
		if (a != 0){
			double x = (b - half_y) / a;
			if ((x >= x1 && x <= x2) || (x <= x1 && x >= x2))
			{
				pair<double, double> p;
				p.first = x;
				p.second = -half_y;
				re->set_point(p);

				return re;
			}
		}
	}
	else if (y1 == y2 && y1 == half_y) // y=maxHeight
	{
		if (a != 0){
			double x = (b + y1) / a;
			if ((x >= x1 && x <= x2) || (x <= x1 && x >= x2))
			{
				pair<double, double> p;
				p.first = x;
				p.second = y1;
				re->set_point(p);


				return re;
			}
		}
	}
	delete re;
	return NULL;
}

//�ж�half_edge��ֱ��y=ax-b�Ƿ��н��㣬if intersect,return the Vertex,else return null
// half_edgeû��next
Vertex* intersaction(HalfEdge *half_edge, double a, double b){
	//�߶ε������յ�
	Vertex* start = half_edge->origin();

	HalfEdge* tmpHalf = half_edge->next();
	Vertex* end = tmpHalf->origin();

	double x1 = start->point().first;
	double y1 = start->point().second;
	double x2 = end->point().first;
	double y2 = end->point().second;

	//�ж��Ƿ�ƽ��
	double k1;
	if ((end->point().first - start->point().first) == 0){ //��ֱ�ں������������б������Ϊ�������INFINITY����
		k1 = INFINITY;
	}
	else{
		k1 = (end->point().second - start->point().second) / (end->point().first - start->point().first);
	}
	double k2 = a;
	if (abs(k1 - k2) <= DELTA)//б����ͬ
		return NULL;

	double resultX;
	if (k1 != INFINITY){
		resultX = (b + y1 - k1*x1) / (a - k1);
	}
	else{
		resultX = x1;
	}

	double resultY;
	if (k1 == 0){//ƽ��������
		resultY = y1;
	}
	else{
		resultY = a*resultX - b;
	}

	Vertex* v = new Vertex();
	pair<double, double> p;
	p.first = resultX;
	p.second = resultY;
	v->set_point(p);

	if (k1 == INFINITY){
		if ((resultY >= y1 && resultY <= y2) || (resultY <= y1 && resultY >= y2))

			return v;
		else
			return NULL;
	}

	if (abs(resultY)> MAX_Y || abs(resultX) >MAX_X){ //�������������ֵ������Ϊ����ֱ��ƽ��
		return NULL;
	}

	if ((resultX >= x1 && resultX <= x2) || (resultX <= x1 && resultX >= x2))
		return v;


	return NULL;
}

//newIntersection,intersectHalfEdge,intersectHalfEdgeRight��Ϊ���ؽ��
HalfEdge* Mesh::getIntersection(double a, double b, HalfEdge newHalf, Vertex &newIntersection){
	//�����²�����half_edge�ҵ���ǰface����ֱ��y=ax-b�н����half_edge
	HalfEdge *newInectHalfEdge = newHalf.next();

	Vertex* inter = intersaction(newInectHalfEdge, a, b);

	while (inter == NULL){
		newInectHalfEdge = newInectHalfEdge->next();

		inter = intersaction(newInectHalfEdge, a, b);

		// TODO: how can newInectHalfEdge->next() not set?
		inter = intersaction(newInectHalfEdge, a, b);//

	}

	newIntersection = *inter;

	delete inter;

	return newInectHalfEdge;
}

bool Mesh::onBoundingBox(Vertex* newIntersection){
	double half_x = MAX_X;
	double half_y = MAX_Y;

	if (newIntersection->point().first == -half_x || newIntersection->point().first == half_x ||
		newIntersection->point().second == -half_y || newIntersection->point().second == half_y)
		return true;
	return false;
}
// store the intersection point of line: ax-b with bounding box to vertex, then return the intersected bounding edge
HalfEdge*  Mesh::getIntersectBundingBox(double a, double b, Vertex &vertex){
	list<HalfEdge*>::iterator it = boundingBox.begin();
	HalfEdge *firstHalfEdge = nullptr; 
	while (it != boundingBox.end()){
		firstHalfEdge = *it++;

		//�жϰ����ֱ���Ƿ��н���
		Vertex *re = intersectWithBoundingBox(firstHalfEdge, a, b);

		if (re != NULL){
			vertex = *re;

			return firstHalfEdge;
		}
	}

	return NULL;
}

//�����м�Ľ���
//����newPoH��newNeH
// Not used yet
void Mesh::dealWithNormalIntersection(Vertex* newIntersection, HalfEdge* intersectHalfEdgeLeft, HalfEdge* intersectHalfEdgeRight, HalfEdge* newPoH, HalfEdge* newNeH){

	//���������µ�HalfEdge
	HalfEdge *alHL = new HalfEdge();
	HalfEdge *alHR = intersectHalfEdgeRight->twin();

	newPoH->set_origin(newIntersection);
	newPoH->set_twin(newNeH);
	newPoH->set_prev(alHR);

	newNeH->set_next(alHL);
	newNeH->set_twin(newPoH);

	alHL->set_origin(newIntersection);
	alHL->set_prev(newNeH);
	if (alHR != NULL){
		alHL->set_next(alHR->next());
	}

	alHL->set_twin(intersectHalfEdgeLeft);

	intersectHalfEdgeLeft->set_twin(alHL);


	alHR->set_next(newPoH);
	alHR->set_origin(intersectHalfEdgeRight->next()->origin());
	alHR->set_twin(intersectHalfEdgeRight);
	//newPoH->set_next(alHR->prev());

}

void Mesh::postCalcPolarAngle(){
	for (long long i = 0; i < sortedPoint.size(); i++){
		vector<LECP_Point*> tmpList;

		list<LECP_Point*> tmpPoints = sortedPoint[i];
		list<LECP_Point*>::iterator it = tmpPoints.begin();
		LECP_Point *pole = *it++;//��˵�

		tmpList.push_back(pole);

		while (it != tmpPoints.end()){//������ÿ������д���
			LECP_Point* tmpPoint = *it++;
			tmpList.push_back(tmpPoint);
		}

		//LECP_Point to Vertex*
		list<Vertex*> polarVextex = changeLECO_PointToVertex(tmpList);// no index
		sortedVector.push_back(polarVextex);
	}

	reverse(sortedVector.begin(), sortedVector.end());//reverse kernel
}

// add a new vertex to a half_edge
HalfEdge* Mesh::splitEdge(HalfEdge* half_edge,Vertex* vertex){
	HalfEdge* newLeft = new HalfEdge();
	HalfEdge* newRight = new HalfEdge();

	//for animation
	newLeft->lecp_point = half_edge->lecp_point;
	newRight->lecp_point = half_edge->lecp_point;

	HalfEdge* half_twin = half_edge->twin();
	bool bb= (half_twin == NULL);

	half_edges_.push_back(newLeft);

	if (!bb){
		half_edges_.push_back(newRight);
	}
	else{
		newRight = NULL;
	}
	
	

	newLeft->set_next (half_edge->next());
	newLeft->set_origin(vertex);
	newLeft->set_prev(half_edge);
	newLeft->set_twin(half_edge->twin());

	if(!bb){
		newRight->set_next(half_twin->next());
	}
	
	if (!bb){
		newRight->set_origin(vertex);
		newRight->set_prev(half_twin);
		newRight->set_twin(half_edge);
	}
	
	//��©
	half_edge->next()->set_prev(newLeft);

	half_edge->set_next(newLeft);
	half_edge->set_twin(newRight);

	if(!bb){

		//��©
		half_twin->next()->set_prev(newRight);

		half_twin->set_next(newRight);
		half_twin->set_twin(newLeft);
	}
	

	return newLeft;
}

// Added by zyx,2015-05-25
void Mesh::connectTwoNewVertices(HalfEdge* h1,HalfEdge* h2,LECP_Point *point){
	HalfEdge* half_up = new HalfEdge();
	HalfEdge* half_down = new HalfEdge();

	half_up->lecp_point = point;
	half_down->lecp_point = point;

	half_edges_.push_back(half_up);
	half_edges_.push_back(half_down);

	half_up->set_next(h2);
	half_up->set_origin(h1->origin());
	half_up->set_prev(h1->prev());
	half_up->set_twin(half_down);

	half_down->set_next(h1);
	half_down->set_origin(h2->origin());
	half_down->set_prev(h2->prev());
	half_down->set_twin(half_up);

	//��©
	h1->prev()->set_next(half_up);
	h2->prev()->set_next(half_down);

	h1->set_prev(half_down);

	h2->set_prev(half_up);
}

//���²����ֱ�߹��ɵĽ���list��ŵ�sortedPoint�У����������㹲x�����
void Mesh::addCurrentAngleSortedResultToVector(LECP_Point *point, vector<pair<LECP_Point*, LECP_Point*>> lecp_points, vector<LECP_Point*> points){

	list<LECP_Point*> sortedAngle;// kernel point's sorted result.
	sortedAngle.push_back(point);// kernel point

	// add all intersect line to the list
	for (long long i = 0; i < lecp_points.size(); i++){
		LECP_Point* line = lecp_points[i].second;//origin user input point
		sortedAngle.push_back(line);
	}

	// three points which xs are the same
	vector<LECP_Point*>::iterator it;
	it = find(points.begin(), points.end(), point);
	long long index = it - points.begin();
	index++;
	// yԽС������ԽС
	while (index<points.size() && points[index]->x == point->x){
		sortedAngle.push_back(points[index]);
		index++;
	}

	// add the sorted list to the vector
	sortedPoint.push_back(sortedAngle);
}