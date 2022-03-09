// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeshAABBTree3.h"
#include "Object.h"
#include "SegmentTypes.h"
#include "TestMgr.generated.h"

struct FGVectex : public FVector2f
{
public:
	static FGVectex Invaild;

	FGVectex()= default;
	FGVectex(const FVector2f &vec): FVector2f(vec){}
public:
	uint32 Id;
	//邻域
	TArray<uint32> adjacencies;
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

//生成无向图,用于图形绘制
struct FGraph
{
public:
	//完美连通图
	TMap<FGVectex, TArray<FGEdge>> graph;
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
	void DetectAllIntersectPoint1();
	void SweepLine();

	static uint32 NewEdgeId();
	static uint32 NewVetexId();
	static void GetLineCoefficientFromTwoPoint(FVector2f p1, FVector2f p2, float& out_a, float& out_b, float& out_c);
protected:
	FGEdge NewLine(const FGVectex& startP, const  FGVectex& endP, const  FGEdge& originLine);
	FGEdge& FindOriginLines(const uint32 lid);
	FGVectex& FindIntersection(const uint32 pid);
	FGVectex& FindVectex(const uint32 pid);

	/// <summary>
	/// 点重叠为标准
	/// </summary>
	/// <param name="p"></param>
	/// <returns></returns>
	bool IsExistShareIntersection(const FVector2f& p);
	bool IsExistVectex(const FVector2f& p);
protected:
	TArray<FGEdge> mOriginLines;
	TArray<FGEdge> mLines;
	TArray<FGVectex> mIntersections;
	TArray<FGVectex> mVectex;
};
