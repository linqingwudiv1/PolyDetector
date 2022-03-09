// Fill out your copyright notice in the Description page of Project Settings.


#include "TestMgr.h"

#include "Kismet/KismetMathLibrary.h"
#include "Intersection/IntrSegment2Segment2.h"


FGEdge FGEdge::Invaild;
FGVectex FGVectex::Invaild;
using PointType = FGVectex;
static int iComparePointOrder(const FGVectex& p1, const FGVectex& p2)
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

static bool bComparePointOrder(const FGVectex& p1, const FGVectex& p2)
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
	SweepLine();
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
				FGVectex intersectPoint;

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
						UE_LOG(LogTemp, Log, TEXT("IntersectPoint {%s} on Line[{%s},{%s}]"),*intersectPoint.ToString(), *segment->StartPoint().ToString(), *segment->EndPoint().ToString());
						segment->intersections.Add(intersectPoint.Id);
					}
				}

				//标记访问,无向图
				took.Add(TPair<uint32, uint32>(targetLine.Id, distLine.Id));
				took.Add(TPair<uint32, uint32>(distLine.Id, targetLine.Id));
			}
		}
	}

	{
#if 0 //业务上不必要,可以忽视(基本永远不会出现叠线段)
		//处理重叠线段,并进行合并
		uint32 nCol = 0;
		TArray<uint32> pids;

		uint32 times = 0;
		bool bOk = false;
		do
		{
			//DFS标记
			TSet<TPair<uint32, uint32>> took;
			bOk = true;
			for (FGEdge& seg_a : mOriginLines)
			{
				if (seg_a.intersections.Num() < 2) { continue; }

				//一般式直线方程系数
				float a, b, c;
				GetLineCoefficientFromTwoPoint(seg_a.StartPoint(), seg_a.EndPoint(), a, b, c);

				for (FGEdge& seg_b : mOriginLines)
				{
					if (seg_a.Id == seg_b.Id) { continue; }
					if (seg_b.intersections.Num() < 2) { continue; }
					check(&seg_a != &seg_b);

					took.Add(TPair<uint32, uint32>(seg_a.Id, seg_b.Id));
					took.Add(TPair<uint32, uint32>(seg_b.Id, seg_a.Id));

					uint32_t nFound = 0;
					float maxLineDist = 0;
					pids.Empty();

					//查询两条线段间的公共交点
					for (uint32& p1Id : seg_a.intersections)
					{
						for (uint32& p2Id : seg_b.intersections)
						{
							if (p1Id == p2Id)
							{
								nFound++;
								pids.Add(p1Id);
							}

							float d = FMath::Sqrt(seg_a.DistanceSquared(this->FindIntersection(p2Id)));

							//float d = vec::lineDist(a, b, c, vec(intersectionPoints[pid2].x, intersectionPoints[pid2].y));
							if (d > maxLineDist) { maxLineDist = d; }
						}
					}

					if (nFound >= 2)
					{
						//说明重叠
						bOk = false;

						if (maxLineDist <= 1e-5f)
						{
							//do somathing :
							//两线段共线....(进行合并操作)
						}
						else
						{
						}
					}
				}
			}
		} while (!bOk);
#endif
	}

}

void UTestMgr::DetectAllIntersectPoint1()
{
	//给orginline分配 Id
	for (int32 i = 0; i < mOriginLines.Num(); i++)
	{
		auto &&p_start = mOriginLines[i].StartPoint();
		auto &&p_end   = mOriginLines[i].EndPoint();

		TArray<FVector2f*> olp;
		bool bPoint = false; // 后面也不一定是点,而是环
		if(p_start.Equals(p_end) )
		{
			olp.Add(&p_start);
			bPoint = true;
		}
		else 
		{
			olp.Add(&p_start);
			olp.Add(&p_end);
		}
		
		for (auto &lp : olp)
		{
			if (!IsExistVectex(*lp))
			{
				FGVectex vectex;
				vectex.Set(lp->X, lp->Y);
				vectex.Id = NewVetexId();
				mVectex.Emplace(vectex);
			}
		}
		// mOriginLines[i].Id = this->NewEdgeId();
		// mOriginLines[i].startPId = this->NewVetexId();
		// mOriginLines[i].endPId = this->NewVetexId();
	}
}

void UTestMgr::SweepLine()
{
	mLines.Empty();

	//从交点中生成线段
	for (FGEdge &seg : mOriginLines)
	{
		if(seg.intersections.Num() < 2 ) { continue; }
		FVector2f &&startp = seg.StartPoint();
		//对交点进行排序
		seg.intersections.Sort([this, &startp](uint32 aid, uint32 bid)
		{
			const auto &p1 = FindIntersection(aid);
			const auto &p2 = FindIntersection(bid);
			return FVector2f::DistSquared(startp, p1) < FVector2f::DistSquared(startp, p2);
		});

		//从交点生成多边形
		for (int idx = 1; idx < seg.intersections.Num(); idx++)
		{
			bool bFoundSharedSeg = false;
			uint32 StartIdx = seg.intersections[idx - 1];
			uint32 EndIdx = seg.intersections[idx];

			FGVectex &p1 = FindIntersection(StartIdx);
			FGVectex &p2 = FindIntersection(EndIdx);
			if (p1.Equals(p2))
			{
				UE_LOG(LogTemp,Log, TEXT("P%u P%u are the same for line #%u"), StartIdx, EndIdx, seg.Id);
				//assert(pointsDiffer(p1, p2));
			}

			for(FGEdge &seg_shared : mLines)
			{
				//判断线段是否存在
				//auto min_seg = FMathf::Min(seg_shared.startPId, seg_shared.endPId);
				//auto max_seg = FMathf::Max(seg_shared.startPId, seg_shared.endPId);
				//
				//auto min_vec = FMathf::Min(StartIdx, EndIdx);
				//auto max_vec = FMathf::Max(StartIdx, EndIdx);
				//
				//min_seg == min_vec;
				//max_seg == max_vec;

				bool cond1 = seg_shared.startPId == StartIdx || seg_shared.startPId == EndIdx;
				bool cond2 = seg_shared.endPId == StartIdx || seg_shared.endPId == EndIdx;
				if(cond1 && cond2)
				{
					bFoundSharedSeg = true;
					break;
				}
			}

			if(!bFoundSharedSeg)
			{
				mLines.Add(NewLine(p1, p2, seg));
			}
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

FGEdge UTestMgr::NewLine(const FGVectex& startP, const  FGVectex& endP, const  FGEdge& originLine)
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

FGVectex& UTestMgr::FindIntersection(const uint32 pid)
{
	for (FGVectex& vectex : mIntersections)
	{
		if(vectex.Id == pid)
		{
			return vectex;
		}
	}
	return FGVectex::Invaild;
}

FGVectex& UTestMgr::FindVectex(const uint32 pid)
{
	for (FGVectex& vectex : mVectex)
	{
		if (vectex.Id == pid)
		{
			return vectex;
		}
	}
	return FGVectex::Invaild;
}

bool UTestMgr::IsExistShareIntersection(const FVector2f& p)
{
	for (FGVectex vectex : mIntersections)
	{
		if (vectex.Equals(p)) 
		{ 
			return true; 
		}
	}
	return false;
}


bool UTestMgr::IsExistVectex(const FVector2f& p)
{
	for (FGVectex vectex : mVectex)
	{
		if (vectex.Equals(p))
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
