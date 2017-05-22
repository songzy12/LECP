#include "Paint.h"
#include "util.h"
#include <QtGui/QPen>
#include <QtGui/QMouseEvent>
#include <QtGui\qpainter.h>
#include <QMessageBox>
#include "qdebug.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <vertex.h>
#include <QWaitCondition>
using namespace std;

PaintWidget::PaintWidget() {

}
//, Mesh *mesh_
PaintWidget::PaintWidget(int width, int height) {
	resize(width, height);
	
	scaleX = 200;
	scaleY = 0.5;
	penWidth = 0.02;

}

PaintWidget::~PaintWidget() {

}

//绘制points中所有点
void PaintWidget::paintEvent(QPaintEvent *event){
	paintAllPoints();
	
	paintAllLine();

	paintIntersectPoints();
}

void PaintWidget::timerEvent(QTimerEvent *event){
	Q_UNUSED(event);

}

void PaintWidget::paintAllPoints(){
  for (long long i = 0; i < myQPoints.size();i++){
	  MyQPoint *tmpP = myQPoints[i];
		paintPoint(tmpP);
	}
}


void PaintWidget::mousePressEvent(QMouseEvent *event){
	if (event->button() == Qt::LeftButton){
		QPoint p = event->pos();

		MyQPoint *mp = new MyQPoint(p);
		

		LECP_Point point = changeMyQPointToLECP_Point(mp);

		if (addPoint(point)){
		
			point.index = points.size();
			points.push_back(point);

			mp->setX(p.x() - 600);
			mp->setY(-400 + p.y());

			mp->setIndex(myQPoints.size());
			myQPoints.push_back(mp);

			update();
		}
	}
}

bool PaintWidget::addPoint(LECP_Point point) {
	vector<LECP_Point>::iterator it;
	it = std::find(points.begin(), points.end(), point);

	if (it == points.end())
	{
		std::cout << points.size() << " point：" << point.x << "," << point.y << endl;

		return true;
	}

	return false;
}

void PaintWidget::removeRepeatPoints() {
	//sort(vertices_.begin(), vertices_.end(), comparePoint);
	//t6vertices_.erase(unique(vertices_.begin(), vertices_.end()), vertices_.end());
}

bool PaintWidget::savePoints(char *filename){
	ofstream outFile(filename);
	if (!outFile){
		QMessageBox box;
		QString msg = "Fail to save file" + QString::fromUtf8(filename);
		box.about(this, "Failed!",msg );
		box.show();
		return false;
	}

	outFile << points.size() << endl;

	for (long long i = 0; i < points.size(); i++){
		LECP_Point point = points[i];
		outFile << point.x << " " << point.y << endl;
	}

	outFile.close();
	update();
	return true;
}

void PaintWidget::init(){
	points.clear();
	myQPoints.clear();
	lines.clear();
	intersectPoints.clear();
}

void PaintWidget::loadPoints(char *fileName){
	init();

	ifstream in(fileName);

	if (!in){
		QMessageBox box;
		QString msg = "Fail to open file" + QString::fromUtf8(fileName);
		box.about(this, "Failed!", msg);
		box.show();
		return ;
	}

	char buffer[256];
	double x, y;

	in.getline(buffer, 10);
	int size = atoi(buffer);
	// TODO: whether to clear vertices_ first?
	for (int i = 0; i < size; i++) {
		in.getline(buffer, 10);
		char *cx = strtok(buffer, " ");
		x = atof(cx);

		char *cy = strtok(NULL, " ");

		y = atof(cy);

		LECP_Point point;
		point.setX(x);
		point.setY(y);
		point.index = points.size();

		std::cout << point.index << ":" << point.x << "," << point.y << endl;

		points.push_back(point);
	}

	changeLECP_PointsToQPoints();//坐标变换

	update();
}

void PaintWidget::paintPoint(MyQPoint point){
	QPainter painter(this);
	painter.translate(WIN_WIDTH / 2, WIN_HEIGHT / 2);

	painter.setBrush(point.getColor());
	painter.drawEllipse(point.x(), point.y(), 6, 6);
	painter.drawText(point.x(), point.y(), QString::number(point.getIndex()));
	update();
}

void PaintWidget::paintPoint(MyQPoint *point){
	QPainter painter(this);
	painter.translate(WIN_WIDTH / 2, WIN_HEIGHT / 2);
	painter.setBrush(point->getColor());
	painter.drawEllipse(point->x(), point->y(), 6, 6);

	int index = point->getIndex();
	if (index != -1){
		painter.drawText(point->x(), point->y(), QString::number(index));
	}
	
	update();
}

void PaintWidget::paintEdge(HalfEdge* edge_) {
	QPainter painter(this);
	painter.translate(WIN_WIDTH / 2, WIN_HEIGHT / 2);
	painter.setBrush(Qt::blue);
	painter.drawLine(edge_->origin()->point().first, edge_->origin()->point().second, 
					 edge_->target()->point().first, edge_->target()->point().second);
	update();
}

//采用index 的 method
LECP_Point PaintWidget::changeMyQPointToLECP_Point(MyQPoint *qPoint){
	vector<MyQPoint*>::iterator it;
	it = std::find(myQPoints.begin(), myQPoints.end(), qPoint);

	long long index = it - myQPoints.begin();

	LECP_Point point;
	if (index >= points.size()){
		double half_x = WIN_WIDTH / 2.0;
		double half_y = WIN_HEIGHT / 2.0;

		point.x = qPoint->x() - half_x;
		point.y = -qPoint->y() + half_y;
		//point.x = qPoint->x();
		//point.y = -qPoint->y();
		
	}
	else{
		point  = points[index];
	}
	
	return point;
}

//直接计算会存在误差，因此按照index返回对应的MyQPoint
//vector<LECP_Point> points;和vector<MyQPoint*> myQPoints;存储顺序一样
//Attention: the order of points and myQPoints can not be changed!!!!

// y坐标相反
MyQPoint* PaintWidget::changeLECP_PointToMyQPoint(LECP_Point p){
	
	MyQPoint *mp;


	vector<LECP_Point>::iterator it;
	it = std::find(points.begin(), points.end(), p);
	long long index = it - points.begin();

	if (myQPoints.size() <= index){
       double half_x = WIN_WIDTH / 2.0;
	   double half_y = WIN_HEIGHT / 2.0;

	   QPoint qp;
	   qp.setX(p.x);
	   qp.setY(-p.y); 
	   //qp.setX(half_x + p.x);
	   //qp.setY(half_y - p.y);
	  mp = new MyQPoint(qp);
	}
	else{
		mp = myQPoints[index];
	}

	return mp;
}

void PaintWidget::changeLECP_PointsToQPoints(){
	//double half_x = WIN_WIDTH / 2.0;
	//double half_y = WIN_HEIGHT/ 2.0;

	for (long long i = 0; i < points.size(); i++)
	{
		LECP_Point p = points[i];
		//QPoint qp;
		//qp.setX(half_x + p.x);
		//qp.setY(half_y - p.y);

		//MyQPoint *mp = new MyQPoint(qp);

		MyQPoint *mp = changeLECP_PointToMyQPoint(p);

		mp->setIndex(myQPoints.size());
		myQPoints.push_back(mp);


	}

	update();
}

void PaintWidget::initDisplayDCEL(){
	displayDCEL = new DisplayDCEL(this->width(), this->height());
	displayDCEL->show();

}

long long PaintWidget::getOriginPointIndex(MyQPoint* qPoint){
	vector<MyQPoint*>::iterator it;
	it = std::find(myQPoints.begin(), myQPoints.end(), qPoint);

	long long index = it - myQPoints.begin();

	return index;
}

//-----------for animation-----------------------------------------------------------------------
void PaintWidget::animationPoint(MyQPoint  *qPoint, vector<pair<LECP_Point*, LECP_Point*>> lecp_points){
	// current user input point change to green
	qPoint->setColor(Qt::green);
	update();

	// draw the corresponding 
	addLine(qPoint);

	displayIntersectionPoint(lecp_points);//挨个显示交点以及对应的用户输入的point

	//draw dual graph
	//displayDCEL->addLine(qPoint);
}

void PaintWidget::displayIntersectionPoint(vector<pair<LECP_Point*, LECP_Point*>> lecp_points){
	for (long long i = 0; i < lecp_points.size(); i++){
		pair<LECP_Point*, LECP_Point*> p = lecp_points[i];
		LECP_Point* intersectPoint = p.first;

		LECP_Point* originPoint = p.second;
		
		//MyQPoint *qPoint = changeLECP_PointToMyQPoint(*intersectPoint);
		MyQPoint *origQPoint = changeLECP_PointToMyQPoint(*originPoint);
		long long index = getOriginPointIndex(origQPoint);

		//qPoint->setColor(Qt::yellow);
		//qPoint->setIndex(index);
		intersectPoint->index = index;
		intersectPoints.push_back(intersectPoint);

		repaint();
		update();

		_sleep(2*1000);
	}

	intersectPoints.clear();

}

void PaintWidget::paintAllLine(){
	QPainter painter(this);
	painter.translate(WIN_WIDTH / 2, WIN_HEIGHT / 2);
	
	painter.scale(scaleX, scaleY); //放大两倍
	
	for (long long i = 0; i < lines.size(); i++){
		MyQline *line = lines[i];
		//paintLine(line);

		painter.setPen(QPen(line->getColor(),penWidth));

		painter.drawLine(*line);
	}
	update();
}

//斜率普遍偏大
void PaintWidget::getSuitCoordinate(double a, double  b, double &x1, double &y1, double &x2, double &y2){
	double maxY = WIN_HEIGHT / 2.0;

	x1 = 500;
	y1 = x1*a - b;
	x2 = -500;
	y2 = x2*a - b;
}

QLine PaintWidget::getPaintLine(MyQPoint *qPoint){
	LECP_Point point = changeMyQPointToLECP_Point(qPoint);
	double a, b;
	a = point.x;
	b = point.y;

	//以QPoint坐标为主

	// y坐标超出范围
	
    double x1, x2, y1, y2;
	getSuitCoordinate(a, b,x1,y1,x2,y2);
	//x1 = -300;
	//y1 = x1*a - b;
	//x2 = 500;
	//y2 = x2*a - b;

	LECP_Point startP, endP;
	startP.setX(x1);
	startP.setY(y1);
	endP.setX(x2);
	endP.setY(y2);

	QPoint *start, *end;
	start = changeLECP_PointToMyQPoint(startP);
	end = changeLECP_PointToMyQPoint(endP);

	QLine line(*start, *end);
	return line;
}

void PaintWidget::paintLine(MyQline *line){
	QPainter painter(this);
	painter.translate(WIN_WIDTH / 2, WIN_HEIGHT / 2);
	painter.setBrush(line->getColor());
	
	painter.setPen(QPen(line->getColor(),0.2));

	painter.drawLine(*line);
	update();
}

void PaintWidget::addLine(MyQPoint *qPoint){
	QLine line = getPaintLine(qPoint);// calculate the dual line according to qPoint
	MyQline *qLine = new MyQline(line);//change QLine to self defined structure MyQline
	qLine->setColor(Qt::green);
	lines.push_back(qLine); // add the new line to the list

	repaint();
	update();

	_sleep(2 * 1000);

	qLine->setColor(Qt::blue);
}

// draw the intersect points
void PaintWidget::paintIntersectPoints(){
	QPainter painter(this);
	painter.translate(WIN_WIDTH / 2, WIN_HEIGHT / 2);
	//painter.scale(scaleX, scaleY); //放大两倍

	for (long long i = 0; i < intersectPoints.size(); i++){
		LECP_Point* point = intersectPoints[i];
		//paintPoint(qPoint);
		painter.setPen(QPen(Qt::cyan, 2));
		painter.setBrush(QBrush(Qt::cyan, Qt::SolidPattern)); //设置画刷形式 
		//painter.drawPoint(qPoint->x(), qPoint->y());
		//painter.drawRect()
		double x = point->x*scaleX;
		double y =- point->y*scaleY;
		painter.drawEllipse(x, y, 8,8);
		
		painter.drawText(x+10,y+10 , QString::number(point->index));
	}

	update();
}