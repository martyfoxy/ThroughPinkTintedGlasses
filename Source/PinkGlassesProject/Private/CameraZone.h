// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Classes/Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraZone.generated.h"

UCLASS()
class ACameraZone : public AActor
{
	GENERATED_BODY()
	
public:	
	ACameraZone();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	//Количество промежуточных точек в зоне
	UPROPERTY(EditAnywhere)
		int IntermediatePointCount = 10;

	//Камера, которую будем двигать
	UPROPERTY(EditAnywhere)
		AActor* CameraReference = nullptr;

	//Скорость движения камеры
	UPROPERTY(EditAnywhere)
		float MoveSpeed = 1.0f;

	//Путь камеры для зоны
	UPROPERTY(EditAnywhere)
		USplineComponent* Path = nullptr;
private:	
	//Компонент камеры
	UCameraComponent* Camera = nullptr;

	//Компонент триггера
	UBoxComponent* Zone = nullptr;

	//Невидимые меши, позволяющие знать края зоны
	UStaticMeshComponent* R;
	UStaticMeshComponent* L;

	//Массив всех промежуточных точек зоны
	TArray<FVector> TriggetPoints;

	//Массив всех контрольных точек сплайна
	TArray<FSplinePoint> SplinePoints;

	//Индекс текущей, следующей и предыдущей точек сплайна
	int CurrentPointIndex = -1;
	int OtherPointIndex = -1;

	//Расстояние до ближайшей точки пути
	float CurrentShortestPath;

	//Позиция текущей промежуточной точки триггера
	FVector CurrentTriggerPoint;

	//Текущая позиция игрока
	FVector PlayerCurrentLocation;
private:
	//Вычислить все промежуточные точки триггера
	void CalculateIntermediatePoints();

	//Получить индексы текущей, предыдущей и следующей точек сплайна
	void GetCurrentPointsIndex();

	//Вспомогательный метод. Получить все точки сплайна
	TArray<FSplinePoint> GetAllSplinePoints();
	
	//Получить реальную точку на сплайне, где будет находится камера
	FVector GetCurrentSplinePoint();
};
