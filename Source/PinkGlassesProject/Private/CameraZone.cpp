// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraZone.h"
#include "PinkGlassesProjectCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Public/DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

// Конструктор
ACameraZone::ACameraZone()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//Создаем триггер бокс
	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	RootComponent = Zone;

	//Создаем пустые мэши, обозначающие края триггера
	R = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("R"));
	R->SetupAttachment(Zone);
	R->SetRelativeLocation(FVector(-32.0f, 32.0f, 0.0f));

	L = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("L"));
	L->SetupAttachment(Zone);
	L->SetRelativeLocation(FVector(-32.0f, -32.0f, 0.0f));

	//Создаем сплайн
	Path = CreateDefaultSubobject<USplineComponent>(TEXT("Camera path"));
	Path->SetupAttachment(Zone);
	Path->bEditableWhenInherited = true;
}

//Инициализация
void ACameraZone::BeginPlay()
{
	Super::BeginPlay();
	
	//Находим промежуточные точки зоны
	CalculateIntermediatePoints();

	#pragma region DEBUG
	//Отображение всех промежуточных точек
	for (FVector triggerPoint : TriggetPoints)
		DrawDebugSphere(GetWorld(), triggerPoint, 2, 12, FColor(0, 255, 0), false, 1000.0f, 0, 1);
	#pragma endregion

	//Заполняем список всех контрольных точек сплайна
	SplinePoints = GetAllSplinePoints();

	//Находим компонент камеры внутри выбранной SplineCamera
	if (CameraReference != nullptr)
		Camera = CameraReference->FindComponentByClass<UCameraComponent>();
	else
		UE_LOG(LogTemp, Warning, TEXT("Камера не привязана к зоне"));
}

//Выполняется в каждом кадре
void ACameraZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentShortestPath = TNumericLimits<float>::Max();
	CurrentTriggerPoint = FVector();

	TArray<AActor*> OverlappingActors;
	Zone->GetOverlappingActors(OverlappingActors, APinkGlassesProjectCharacter::StaticClass());

	for (AActor* PlayerActor : OverlappingActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("В зону %s вошел игрок %s"), *(GetName()), *(PlayerActor->GetName()));
		PlayerCurrentLocation = PlayerActor->GetActorLocation();

		//Когда игрок заходит в зону, то меняем вектора направления в зависимости от направления зоны
		APinkGlassesProjectCharacter* PlayerCharacter = Cast<APinkGlassesProjectCharacter>(PlayerActor);
		PlayerCharacter->ChangeForwardVector(Zone->GetForwardVector());
		PlayerCharacter->ChangeRightVector(Zone->GetRightVector());

		//Смотрим все промежуточные точки, вычисляем расстояние между ними и игроком, и находим ту, которая находится ближе всего
		for (FVector TriggerPoint : TriggetPoints)
		{
			float length = FVector::Distance(TriggerPoint, PlayerCurrentLocation);

			if (CurrentShortestPath >= length)
			{
				CurrentShortestPath = length;
				CurrentTriggerPoint = TriggerPoint;
			}
		}

		//Обновим индексы точек
		GetCurrentPointsIndex();

		//Вычисляем точку сплайна для камеры
		FVector SplinePoint = GetCurrentSplinePoint();
		
		//Двигаем камеру
		if (Camera != nullptr)
		{
			FVector CameraLocation = Camera->GetComponentLocation();

			FVector NewLocation = FMath::VInterpTo(CameraLocation, SplinePoint, FApp::GetDeltaTime(), MoveSpeed);
			FRotator NewRotation = FRotationMatrix::MakeFromX(PlayerCurrentLocation - CameraLocation).Rotator();

			Camera->SetWorldLocationAndRotation(NewLocation, NewRotation);
		}
	}	
}

//Получить все промежуточные точки зоны
void ACameraZone::CalculateIntermediatePoints()
{
	TriggetPoints = TArray<FVector>();

	FVector RightPoint = R->GetComponentLocation();
	FVector LeftPoint = L->GetComponentLocation();

	//L-вектор - это минимальный промежуточный вектор между двумя соседними точками
	FVector LVector = (RightPoint - LeftPoint) / IntermediatePointCount;

	//Начинаем с левой точки
	FVector NextPoint = LeftPoint;
	TriggetPoints.Add(NextPoint);

	//Находим промежуточные
	for (int i = 1; i < IntermediatePointCount; i++)
	{
		NextPoint += LVector;
		TriggetPoints.Add(NextPoint);
	}

	//Добавляем правую точку последней
	TriggetPoints.Add(RightPoint);
}

//Получить все точки сплайна
TArray<FSplinePoint> ACameraZone::GetAllSplinePoints()
{
	//Если сплайн не привязан к зоне, то выдаем пустой список
	if (Path != nullptr)
	{
		TArray<FSplinePoint> SplinePoints = TArray<FSplinePoint>();

		//Проходим по всем точкам
		for (int i = 0; i < Path->GetNumberOfSplinePoints(); i++)
		{
			//Получим свойства текущей точки
			FRotator PointRotation = Path->GetRotationAtSplinePoint(i, ESplineCoordinateSpace::Local);
			FVector PointLocation = Path->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
			FVector PointScale = Path->GetScaleAtSplinePoint(i);
			auto PointType = Path->GetSplinePointType(i);

			//Создадим точку, использую структуру
			FSplinePoint SplinePoint = FSplinePoint();
			SplinePoint.InputKey = i;
			SplinePoint.Position = PointLocation;
			SplinePoint.Rotation = PointRotation;
			SplinePoint.Scale = PointScale;
			SplinePoint.Type = PointType;

			SplinePoints.Add(SplinePoint);
		}

		return SplinePoints;
	}
	return TArray<FSplinePoint>();
}

//Получить точку на сплайне для камеры
FVector ACameraZone::GetCurrentSplinePoint()
{
	//Найдем мировые координаты нужных нам контрольных точек сплайна
	FVector CurrentPoint = Path->GetLocationAtSplinePoint(CurrentPointIndex, ESplineCoordinateSpace::World);
	FVector OtherPoint = Path->GetLocationAtSplinePoint(OtherPointIndex, ESplineCoordinateSpace::World);

	//Точка пересечения прямой между текущей и следующей точками, и плоскостью в позиции игрока
	FVector PlaneIntersection = FMath::LinePlaneIntersection(CurrentPoint, OtherPoint, PlayerCurrentLocation, Zone->GetRightVector());

	FVector ResultPoint = Path->FindLocationClosestToWorldLocation(PlaneIntersection, ESplineCoordinateSpace::World);

	#pragma region DEBUG
	//Отображаем линии до контрольных точек
	DrawDebugLine(GetWorld(), CurrentTriggerPoint, CurrentPoint, FColor(0, 255, 0, 255), false, -1.0f, 0, 4.0f);
	DrawDebugLine(GetWorld(), CurrentTriggerPoint, OtherPoint, FColor(255, 0, 255, 255), false, -1.0f, 0, 2.0f);
	DrawDebugLine(GetWorld(), CurrentPoint, OtherPoint, FColor::Black, false, -1.0f, 0, 2.0f);

	DrawDebugSphere(GetWorld(), PlaneIntersection, 10, 12, FColor::Blue, false, -1.0f, 0, 1);
	DrawDebugSphere(GetWorld(), ResultPoint, 10, 12, FColor::Orange, false, -1.0f, 0, 1);
	#pragma endregion

	return ResultPoint;
}

//Вычислить индекс текущей контрольной точки сплайна
void ACameraZone::GetCurrentPointsIndex()
{
	FVector RightVector = Zone->GetRightVector();
	FVector ForwardVector = Zone->GetForwardVector() * -1;
	float MinAngle = 180.0f;
	FVector2D DDCurrentTriggerPoint = FVector2D(CurrentTriggerPoint.X, CurrentTriggerPoint.Y);
	FVector2D DDForwardVector = FVector2D(ForwardVector.X, ForwardVector.Y);
	FVector2D DDRightVector = FVector2D(RightVector.X, RightVector.Y);

	//Смотрим каждую контрольную точку сплайна
	for (FSplinePoint Point : SplinePoints)
	{
		//Мировая позиция контрольной точки сплайна
		FVector PointLocation = Path->GetLocationAtSplinePoint(Point.InputKey, ESplineCoordinateSpace::World);

		//Для правильных вычислений, все точки должны находится в одной плоскости
		
		FVector2D DDPointLocation = FVector2D(PointLocation.X, PointLocation.Y);
		FVector2D DDFromTriggerToPoint = DDPointLocation - DDCurrentTriggerPoint;

		//Угол между точкой и перпендикуляром
		float angle = acosf(FVector2D::DotProduct(DDFromTriggerToPoint.GetSafeNormal(), DDForwardVector.GetSafeNormal())) * (180 / 3.1415926);

		#pragma region DEBUG
		DrawDebugLine(GetWorld(), CurrentTriggerPoint, PointLocation, FColor(255, 255, 255, 255), false, -1.0f, 0, 1.0f);
		DrawDebugLine(GetWorld(), CurrentTriggerPoint, CurrentTriggerPoint + ForwardVector * 200, FColor(0, 0, 0, 255), false, -1.0f, 0, 2.0f);

		TArray<FStringFormatArg> args;
		args.Add(FStringFormatArg(angle));
		FString res = FString::Format(TEXT("{0}"), args);

		DrawDebugString(GetWorld(), PointLocation, res, 0, FColor::White, 1.0f);
		#pragma endregion

		//Находим точку с минимальным углом
		if (angle < MinAngle)
		{
			MinAngle = angle;
			CurrentPointIndex = Point.InputKey;
		}
	}

	//Если мы в начале сплайна
	if (CurrentPointIndex == 0)
		OtherPointIndex = CurrentPointIndex + 1;
	//Если мы в конце сплайна
	else if (CurrentPointIndex == SplinePoints.Num() - 1)
		OtherPointIndex = CurrentPointIndex - 1;
	//Если мы внутри сплайна
	else
	{
		//Мировая позиция контрольной точки сплайна
		FVector PointLocation = Path->GetLocationAtSplinePoint(CurrentPointIndex, ESplineCoordinateSpace::World);

		//Для правильных вычислений, все точки должны находится в одной плоскости
		FVector2D DDPointLocation = FVector2D(PointLocation.X, PointLocation.Y);
		FVector2D DDFromTriggerToPoint = DDPointLocation - DDCurrentTriggerPoint;

		//Угол между точкой и перпендикуляром
		float angle = acosf(FVector2D::DotProduct(DDFromTriggerToPoint.GetSafeNormal(), DDRightVector.GetSafeNormal())) * (180 / 3.1415926);

		//Проверяем, находимся ли мы левее или правее текущей контрольной точки
		if (angle < 90)
			OtherPointIndex = CurrentPointIndex - 1;
		else
			OtherPointIndex = CurrentPointIndex + 1;
	}
}

