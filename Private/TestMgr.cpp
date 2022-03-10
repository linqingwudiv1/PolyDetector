// Fill out your copyright notice in the Description page of Project Settings.


#include "TestMgr.h"

#include "Kismet/KismetMathLibrary.h"
#include "Intersection/IntrSegment2Segment2.h"


FGEdge FGEdge::Invaild;
FGVertex FGVertex::Invaild;
using PointType = FGVertex;
static int iComparePointOrder(const FGVertex& p1, const FGVertex& p2)
{
	if (p1.Y < p2.Y)
		return -1;
	else if (p1.Y == p2.Y)
	{
		if (p1.X < p2.X)
			return -1;
		else if (p1.X == p2.X)
			return 0;
	}
	// p1 is greater than p2
	return 1;
}

static bool bComparePointOrder(const FGVertex& p1, const FGVertex& p2)
{
	return iComparePointOrder(p1, p2) < 0;
}

static int iCompareLineOrder(const FGEdge& l1, const FGEdge& l2)
{
	int result = iComparePointOrder(l1.StartPoint(), l2.EndPoint());

	if (result == 0)
	{
		// in case lines share first point
		// we must order the lines by its slope
		auto&& p1StartP = l1.StartPoint();
		auto&& p1EndP = l1.EndPoint();

		auto&& p2StartP = l1.StartPoint();
		auto&& p2EndP = l1.EndPoint();

		float dx1 = p1EndP.X - p1StartP.X;
		float dy1 = p1EndP.Y - p1StartP.Y;

		float dx2 = p2EndP.X - p2StartP.X;
		float dy2 = p2EndP.Y - p2StartP.Y;

		// by definition of first and last point we are sure that dy > 0

		if (dx1 > 0 && dx2 < 0)
			// line 1 in 1st quadrant, line 2 in 2nd quadrant
			// this means line 2 cames first
			return 1;

		if (dx1 < 0 && dx2>0)
			// line 1 in 2nd quadrant, line 2 in 1st quadrant
			// this means line 1 cames first
			return -1;

		if (dx1 == 0) {
			// first line is vertical
			if (dx2 > 0)
				// second line in 1st quadrant
				// first line is previous
				return -1;

			if (dx2 < 0)
				// second line in 2nd quadrant
				// second line is previous
				return 1;
			// this should no happen
			return 0;
		}

		if (dx2 == 0) {
			// second line is vertical
			if (dx1 > 0)
				// first line in 1st quadrant
				// second line is previous
				return 1;

			if (dx1 < 0)
				// first line in 2nd quadrant
				// first line is previous
				return -1;

			// this should not happen
			return 0;
		}


		// calculate the slopes
		double m1 = dy1 / dx1;
		double m2 = dy2 / dx2;
		// line 1 and line 2 in 2nd quadrant
		if (m1 > m2)
			return -1;
		if (m1 < m2)
			return 1;

		// in this case we have the same slope in both lines,
		// which means that both lines are coincident.
		return 0;
	}

	return result;
}

static bool bCompareLineOrder(const FGEdge& l1,const FGEdge& l2)
{
	return iCompareLineOrder(l1, l2) < 0;
}


void UTestMgr::UpdateLines(TArray<FGEdge>& OriginLines)
{
	mOriginLines = OriginLines;
	//
	mOriginLines.Sort([](const FGEdge& l1, const FGEdge& l2)
	{
		return bCompareLineOrder(l1, l2);
	});
	
	DetectAllIntersectPoint();
	//SweepLine();
}

void UTestMgr::GenerateGraph()
{
	//生成无向图
	
	//确保线段和交点均排过序
	for (auto &seg : mOriginLines)
	{
		auto &&seg_startP = seg.StartPoint();
		auto &&seg_endP = seg.EndPoint();
		for (auto& p:seg.intersections)
		{

		}
	}
	
}

void UTestMgr::DetectAllIntersectPoint()
{
	//给orginline分配 Id
	for (int32 i = 0; i < mOriginLines.Num(); i++)
	{
		mOriginLines[i].Id = this->NewEdgeId();
		mOriginLines[i].startPId = this->NewVetexId();
		mOriginLines[i].endPId = this->NewVetexId();
	}

	//计算线线的邻域关系
	for (int32 idx_target = 0; idx_target < mOriginLines.Num(); idx_target++)
	{
		FGEdge& targetLine = mOriginLines[idx_target];
		for (int32 idx_dist = idx_target + 1; idx_dist < mOriginLines.Num(); idx_dist++)
		{
			if(idx_dist == idx_target)
			{
				continue; 
			}
			FGEdge& distLine = mOriginLines[idx_dist];

			if(targetLine.Intersects(distLine))
			{
				//邻域(相邻的原始Edge)
				targetLine.adjacencies.Add(distLine.Id);
				distLine.adjacencies.Add(targetLine.Id);
			}
		}
	}

	{
		//计算线线间的交点
		//DFS标记
		TSet<TPair<uint32, uint32>> took;
		for (int32 idx_target = 0; idx_target < mOriginLines.Num(); idx_target++)
		{
			FGEdge& targetLine = mOriginLines[idx_target];
			for (uint32 otherIntesectLineId : targetLine.adjacencies)
			{
				//计算相交点
				FGEdge& distLine = FindOriginLines(otherIntesectLineId);
				FGVertex intersectPoint;

				if (took.Find(TPair<uint32, uint32>(targetLine.Id, distLine.Id)) != nullptr)
				{
					continue;
				}

				UE::Geometry::FIntrSegment2Segment2f intr(targetLine, distLine);
				if (intr.Compute().Result == EIntersectionResult::Intersects)
				{
					if (!IsExistShareIntersection(intr.Point0))
					{
						intersectPoint.Set(intr.Point0.X, intr.Point0.Y);
						intersectPoint.Id = NewVetexId();
						mIntersections.Emplace(intersectPoint);
					}
				}
				//添加线段上的交点关系
				for (auto& segment :{&targetLine, &distLine})
				{
					if(segment->intersections.Find(intersectPoint.Id) == INDEX_NONE)
					{
						UE_LOG(LogTemp, Log, TEXT("IntersectPoint {%s} on Line[{%s},{%s}]"), *intersectPoint.ToString(), *segment->StartPoint().ToString(), *segment->EndPoint().ToString());
						segment->intersections.Add(intersectPoint.Id);
						
						//segment的端点即为交点
						if( segment->StartPoint().Equals(intersectPoint) )
						{
							segment->startPId = intersectPoint.Id;
						}
						else if( segment->EndPoint().Equals(intersectPoint) )
						{
							segment->endPId = intersectPoint.Id;
						}
					}
				}

				//DFS标记访问,无向图
				took.Add(TPair<uint32, uint32>(targetLine.Id, distLine.Id));
				took.Add(TPair<uint32, uint32>(distLine.Id, targetLine.Id));
			}
		}
	}

	{
		//生成Vertex与邻域拓扑关系
		
		{
			//写入所有顶点
			CreateVertex();
			//}
		}

		{
			//建立顶点的邻域
			GenerateAdj();
		}
	}
}

void UTestMgr::CreateVertex()
{
	//写入所有顶点
	mG.mVertex.Empty();
	for (auto& p : mIntersections)
	{
		mG.AddVertex(p);
	}

	for (int32 idx = 0; idx < mOriginLines.Num(); idx++)
	{
		FGEdge& segment = mOriginLines[idx];

		for (auto& vectexWrapper : { TPair<uint32, FVector2f>(segment.startPId, segment.StartPoint()),TPair<uint32, FVector2f>(segment.endPId ,segment.EndPoint()) })
		{
			if(this->mG.mVertex.Find(vectexWrapper.Key) == nullptr)
			{
				FGVertex v(vectexWrapper.Value);
				v.Id = vectexWrapper.Key;
				mG.AddVertex(v);
			}
		}
	}
}

void UTestMgr::GenerateAdj()
{
	//建立顶点的邻域
	for (int32 idx = 0; idx < mOriginLines.Num(); idx++)
	{
		auto& seg = mOriginLines[idx];
		auto&& startp = seg.StartPoint();
		//对交点进行排序(按到startP的距离升序)
		seg.intersections.Sort([this, &startp](uint32 aid, uint32 bid)
			{
				const auto& p1 = FindIntersection(aid);
				const auto& p2 = FindIntersection(bid);
				//note:交点为起始端点时, DistSquared == 0, infine
				return FVector2f::DistSquared(startp, p1) < FVector2f::DistSquared(startp, p2);
			});
		//test:情况:
		// e.g:
		//1.intersections0 == startpid ||  intersectionsMax == endPid
		// start---------------1-------------------2--------------------3-------------------------end
		// start==1------------2-------------------3--------------------4-------------------------end

		TArray<uint32> vertices;
		if (seg.intersections.Num() == 0)
		{
			//无交点则前后端点互为邻居
			vertices.Append({ seg.startPId, seg.endPId });
		}
		else
		{
			if (seg.startPId != seg.intersections[0])
			{
				//第一个交点非起始点
				vertices.Add(seg.startPId);
			}

			vertices.Append(seg.intersections);

			if (seg.endPId != seg.intersections[seg.intersections.Num() - 1])
			{
				//最后的交点非结尾点
				vertices.Add(seg.endPId);
			}
		}

		//compute of vertex
		TArray<uint32>* pre = nullptr;
		TArray<uint32>* cur = nullptr;
		for (int32 idx = 1; idx < vertices.Num(); idx++)
		{
			uint32 startPid = vertices[idx - 1];
			uint32 endPid = vertices[idx];
			//endPid->
			if (pre == nullptr)
			{
				pre = mG.topology. Find(startPid);
			}
			cur = mG.topology.Find(endPid);
			pre->Add(endPid);
			cur->Add(startPid);
			pre = cur;
		}
	}
}

uint32 UTestMgr::NewEdgeId()
{
	static TAtomic<uint32> id = 0;
	return id++;
}

uint32 UTestMgr::NewVetexId()
{
	static TAtomic<uint32> id = 0;
	return id++;
}

FGEdge UTestMgr::NewLine(const FGVertex& startP, const  FGVertex& endP, const  FGEdge& originLine)
{
	FGEdge edge;
	edge.Id = NewEdgeId();
	edge.startPId = startP.Id;
	edge.endPId = endP.Id;
	//
	edge.SetStartPoint(startP);
	edge.SetEndPoint(endP);
	return edge;
}

void UTestMgr::GetLineCoefficientFromTwoPoint(FVector2f p1, FVector2f p2, float& out_a, float& out_b, float& out_c)
{
	out_a = p1.X - p2.X;
	out_b = p2.Y - p1.Y;
	out_c = out_a * p1.X + out_b * p2.Y;
}

FGEdge& UTestMgr::FindOriginLines(const uint32 lid)
{
	//TODO:排序后二分查找...
	for (FGEdge& edge : mOriginLines)
	{
		if (edge.Id == lid)
		{
			return edge;
		}
	}

	return FGEdge::Invaild;//FGEdge();
}

FGVertex& UTestMgr::FindIntersection(const uint32 pid)
{
	for (FGVertex& vectex : mIntersections)
	{
		if(vectex.Id == pid)
		{
			return vectex;
		}
	}
	return FGVertex::Invaild;
}

FGVertex& UTestMgr::FindVertex(const uint32 pid)
{
	auto result = mG.mVertex.Find(pid);
	
	if(result == nullptr)
	{
		return *result;
	}
	else 
	{
		return FGVertex::Invaild;
	}
}

bool UTestMgr::IsExistShareIntersection(const FVector2f& p)
{
	for (FGVertex vectex : mIntersections)
	{
		if (vectex.Equals(p)) 
		{ 
			return true; 
		}
	}
	return false;
}


bool UTestMgr::IsExistVertex(const FVector2f& p)
{
	for (auto &vectex : mG.mVertex)
	{
		if (vectex.Value.Equals(p))
		{
			return true;
		}
	}
	return false;
}



void FGEdge::Swap()
{
	uint32 tmp = this->startPId;
	this->startPId = this->endPId;
	this->endPId = tmp;
	this->Reverse();
}

bool FGraph::AddVertex(FGVertex& vertex)
{
	#if WITH_EDITOR
	check(this->mVertex.Find(vertex.Id) == nullptr);
	check(this->topology.Find(vertex.Id) == nullptr);
	#endif
	this->mVertex.Add(vertex.Id, vertex);
	this->topology.Add(vertex.Id, TArray<uint32>() );
	return true;
}

FGVertex* FGraph::GetClockwiseVertex(FGVertex* prev, FGVertex* cur)
{
	FVector2f d;
	if(prev == nullptr)
	{
		d = FVector2f{0,-1}; 
	}
	else
	{
		d = (*cur) - (*prev); 
	}

	TArray<uint32> *adjs = this->topology.Find(cur->Id);

	for (auto pid : *adjs)
	{
	}
}

FGVertex* FGraph::GetCounterClockwiseVertex(FGVertex* prev, FGVertex* cur)
{
	return nullptr;
}

FGVertex* FGraph::GetLeftMostVertex()
{
	//MinEle
	FGVertex* Min = nullptr;

	for(auto &c :this->mVertex)
	{
		if(Min == nullptr){Min = &c.Value; continue;}
		bool cond1 = Min->X < c.Value.X;
		bool cond2 = Min->X == c.Value.X && Min->Y < c.Value.Y;
		if( cond1 || cond2)
		{
			Min = &c.Value;
		}
	}
	return Min;
}
