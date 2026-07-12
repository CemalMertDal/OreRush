#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/OreRushTypes.h"
#include "MapGenerator.generated.h"

class AOreVein;
class ADepotZone;
class APowerUpBase;
class UBoxComponent;

UCLASS()
class ORERUSH_API AMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AMapGenerator();

	void EnsureGenerated();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map|Area")
	TObjectPtr<UBoxComponent> AreaViz;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Classes")
	TSubclassOf<AOreVein> VeinClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Classes")
	TSubclassOf<ADepotZone> DepotClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Classes")
	TArray<TSubclassOf<APowerUpBase>> PowerUpClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Classes")
	TArray<TSubclassOf<AActor>> DecorClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 IronCount = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 GoldCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 DiamondCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "1"))
	int32 GoldUnits = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "1"))
	int32 DiamondUnits = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 PowerUpCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts", meta = (ClampMin = "0"))
	int32 DecorCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts")
	bool bScaleWithArea = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Counts")
	FVector2D ReferenceArea = FVector2D(2500.f, 2500.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Area")
	FVector2D AreaExtent = FVector2D(2500.f, 2500.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Area", meta = (ClampMin = "0.0"))
	float MinDistance = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Area")
	float SpawnHeight = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Spawn", meta = (ClampMin = "1"))
	int32 PickAttempts = 250;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Spawn", meta = (ClampMin = "0.0"))
	float ObstacleClearance = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Spawn", meta = (ClampMin = "0.0"))
	float MaxObstacleRadius = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Depot", meta = (ClampMin = "0.0"))
	float DepotMinSeparation = 3800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Depot", meta = (ClampMin = "0.0"))
	float DepotClearance = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Depot")
	bool bSplitDepotsByRiver = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|River")
	bool bAutoDetectRiver = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|River")
	float RiverZMargin = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|River")
	bool bUseManualWaterZ = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|River")
	float ManualWaterZ = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Seed")
	bool bOverrideSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Seed")
	int32 SeedOverride = 12345;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Trace")
	float MaxTraceHeight = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Trace")
	float MinTraceHeight = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map|Trace", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxSlopeAngle = 30.f;

private:
	void Generate(int32 Seed);

	AOreVein* SpawnVein(EOreType Type, bool bUnlimited, int32 Units, const FVector& Location);

	int32 ScaledCount(int32 Base) const;

	void GatherEnvironment();

	bool TryPickRaw(FRandomStream& Rng, FVector& OutPoint, float Clearance, bool bAllowObstacles = false, bool bAllowWater = false) const;

	bool PickPoint(FRandomStream& Rng, FVector& OutPoint, float Clearance);

	bool PickFarthestPair(FRandomStream& Rng, float Clearance, bool bAllowObstacles, bool bAllowWater, FVector& OutA, FVector& OutB);

	bool PickSplitPair(FRandomStream& Rng, float Clearance, bool bAllowObstacles, bool bAllowWater, FVector& OutRed, FVector& OutBlue);

	bool PickDepotPair(FRandomStream& Rng, FVector& OutRed, FVector& OutBlue);

	float SideOfRiver(const FVector& Point) const;

	bool FindLandscapeGround(const FVector& InLocation, FVector& OutGround, FVector& OutNormal) const;

	bool IsBlockedByObstacle(const FVector2D& Point, float ExtraRadius) const;

	bool IsAboveWater(float GroundZ) const;

	float ObstacleRadiusForMesh(class UStaticMesh* Mesh) const;

	TArray<FVector> UsedPoints;

	TArray<FVector> Obstacles;

	bool bWaterKnown = false;

	float WaterSurfaceZ = 0.f;

	bool bRiverAxisKnown = false;

	FVector2D RiverCenter2D = FVector2D::ZeroVector;

	FVector2D RiverSplitNormal = FVector2D::ZeroVector;

	bool bGenerated = false;
};
