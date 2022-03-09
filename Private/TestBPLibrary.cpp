// Fill out your copyright notice in the Description page of Project Settings.


#include "TestBPLibrary.h"
#include "TestMgr.h"

void UTestBPLibrary::Test()
{
	UTestMgr* c1 = NewObject<UTestMgr>();

	TArray<FGEdge> arr;
	
	arr.Emplace(FVector2f(-15, 0), FVector2f(15, 0));
	arr.Emplace(FVector2f(-15, -15), FVector2f(15, -15));
	arr.Emplace(FVector2f(-10, -20), FVector2f(-10, 20));
	arr.Emplace(FVector2f(10, -20), FVector2f(10, -5));
	arr.Emplace(FVector2f(5, -10), FVector2f(15, -10));
	arr.Emplace(FVector2f(8, -12), FVector2f(8, 20));

	c1->UpdateLines(arr);
	c1->GenerateGraph();

}