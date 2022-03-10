// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeshAABBTree3.h"
#include "Object.h"
#include "SegmentTypes.h"
#include "TestMgr.generated.h"

struct FGVertex : public FVector2f
{
public:
	static FGVertex Invaild;

	FGVertex()= default;
	FGVertex(const FVector2f &vec): FVector2f(vec){}
public:
	uint32 Id;
	//邻域
	//TArray<uint32> adjacencies;
};

struct FGEdge : public UE::Geometry::FSegment2f
{
public:
	FGEdge() = default;
	FGEdge(const FVector2f& Point0, const FVector2f& Point1):
		UE::Geometry::FSegment2f(Point0,Point1)
	{
	}
public:
	static FGEdge Invaild;
	void Swap();
	uint32 startPId;
	uint32 endPId;

	uint32 Id;
	//邻域(相邻的线段)
	TArray<uint32> adjacencies;
	TArray<uint32> intersections;
};

//生成无向图拓扑关系,用于图形绘制
struct FGraph
{
public:
	bool AddVertex(FGVertex & vertex);
public:
	FGVertex* GetLeftMostVertex();
	FGVertex* GetClockwiseVertex(FGVertex* prev, FGVertex* cur);
	FGVertex* GetCounterClockwiseVertex(FGVertex* prev, FGVertex* cur);
public:
	//连通图
	TMap<uint32, TArray<uint32>> topology;
	TMap<uint32, FGVertex> mVertex;
};

/**
 * 
 */
UCLASS()
class UIEXTRA_API UTestMgr : public UObject
{
	GENERATED_BODY()
public:
	void UpdateLines(TArray<FGEdge>& mOriginLines);
	void GenerateGraph();
protected:
	
	void DetectAllIntersectPoint();
	//void SweepLine();
	void CreateVertex();
	void GenerateAdj();

	static uint32 NewEdgeId();
	static uint32 NewVetexId();

	static void GetLineCoefficientFromTwoPoint(FVector2f p1, FVector2f p2, float& out_a, float& out_b, float& out_c);
protected:
	FGEdge NewLine(const FGVertex& startP, const  FGVertex& endP, const  FGEdge& originLine);
	FGEdge& FindOriginLines(const uint32 lid);
	FGVertex& FindIntersection(const uint32 pid);
	FGVertex& FindVertex(const uint32 pid);

	/// <summary>
	/// 点重叠为标准
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	bool IsExistShareIntersection(const FVector2f& p);
	bool IsExistVertex(const FVector2f& p);
protected:
	FGraph mG;
	TArray<FGEdge> mOriginLines;
	//TArray<FGEdge> mLines;
	TArray<FGVertex> mIntersections;
	//TArray<FGVertex> mVertex;
};
