#include "Map/MapGenerator.h"
#include "Ore/OreVein.h"
#include "Game/DepotZone.h"
#include "Game/OreRushGameState.h"
#include "Game/OreRushGameMode.h"
#include "PowerUp/PowerUpBase.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
	bool IsObstacleMeshName(const FString& Name)
	{
		static const TCHAR* Keys[] = {
			TEXT("Tree"), TEXT("rock"), TEXT("bush"), TEXT("branch"),
			TEXT("log"), TEXT("Death"), TEXT("mushroom"), TEXT("plant"), TEXT("stump")
		};
		for (const TCHAR* Key : Keys)
		{
			if (Name.Contains(Key, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	bool ComponentUsesWater(const UStaticMeshComponent* Comp)
	{
		if (!Comp)
		{
			return false;
		}
		const int32 Num = Comp->GetNumMaterials();
		for (int32 i = 0; i < Num; ++i)
		{
			if (const UMaterialInterface* Mat = Comp->GetMaterial(i))
			{
				if (Mat->GetName().Contains(TEXT("Water"), ESearchCase::IgnoreCase))
				{
					return true;
				}
			}
		}
		return false;
	}
}

AMapGenerator::AMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	AreaViz = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaViz"));
	SetRootComponent(AreaViz);
	AreaViz->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AreaViz->SetBoxExtent(FVector(AreaExtent.X, AreaExtent.Y, 200.f));
	AreaViz->ShapeColor = FColor::Green;
	AreaViz->SetHiddenInGame(true);
	AreaViz->bIsEditorOnly = true;

	VeinClass = AOreVein::StaticClass();
	DepotClass = ADepotZone::StaticClass();
}

void AMapGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (AreaViz)
	{
		AreaViz->SetBoxExtent(FVector(FMath::Max(1.f, AreaExtent.X), FMath::Max(1.f, AreaExtent.Y), 200.f));
	}
}

void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	EnsureGenerated();
}

void AMapGenerator::EnsureGenerated()
{
	if (bGenerated || !HasAuthority())
	{
		return;
	}

	int32 Seed = SeedOverride;
	if (!bOverrideSeed)
	{
		if (const AOreRushGameState* GS = GetWorld()->GetGameState<AOreRushGameState>())
		{
			Seed = GS->MapSeed;
		}
	}
	if (Seed == 0)
	{
		Seed = 1;
	}

	bGenerated = true;
	Generate(Seed);
}

float AMapGenerator::ObstacleRadiusForMesh(UStaticMesh* Mesh) const
{
	if (!Mesh)
	{
		return 80.f;
	}
	const FVector Size = Mesh->GetBoundingBox().GetSize();
	const float Radius = 0.5f * FMath::Max(Size.X, Size.Y);
	return FMath::Clamp(Radius, 40.f, MaxObstacleRadius);
}

void AMapGenerator::GatherEnvironment()
{
	Obstacles.Reset();
	bWaterKnown = false;
	WaterSurfaceZ = 0.f;
	bRiverAxisKnown = false;
	RiverCenter2D = FVector2D::ZeroVector;
	RiverSplitNormal = FVector2D::ZeroVector;

	if (bUseManualWaterZ)
	{
		bWaterKnown = true;
		WaterSurfaceZ = ManualWaterZ;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bool bWaterBoundsKnown = false;
	FVector2D WaterMin = FVector2D::ZeroVector;
	FVector2D WaterMax = FVector2D::ZeroVector;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor == this)
		{
			continue;
		}

		TInlineComponentArray<UStaticMeshComponent*> MeshComps;
		Actor->GetComponents(MeshComps);

		for (UStaticMeshComponent* Comp : MeshComps)
		{
			if (!Comp)
			{
				continue;
			}

			if (UInstancedStaticMeshComponent* Ism = Cast<UInstancedStaticMeshComponent>(Comp))
			{
				UStaticMesh* Mesh = Ism->GetStaticMesh();
				if (!Mesh || !IsObstacleMeshName(Mesh->GetName()))
				{
					continue;
				}
				const float BaseRadius = ObstacleRadiusForMesh(Mesh);
				const int32 Count = Ism->GetInstanceCount();
				for (int32 i = 0; i < Count; ++i)
				{
					FTransform T;
					if (Ism->GetInstanceTransform(i, T, true))
					{
						const float Scale = T.GetScale3D().GetAbsMax();
						const FVector Loc = T.GetLocation();
						Obstacles.Add(FVector(Loc.X, Loc.Y, FMath::Max(20.f, BaseRadius * Scale)));
					}
				}
				continue;
			}

			UStaticMesh* Mesh = Comp->GetStaticMesh();
			if (!Mesh)
			{
				continue;
			}

			if (bAutoDetectRiver && ComponentUsesWater(Comp))
			{
				const FBoxSphereBounds Bounds = Comp->Bounds;
				const float TopZ = Bounds.Origin.Z + Bounds.BoxExtent.Z;
				if (!bWaterKnown || TopZ > WaterSurfaceZ)
				{
					WaterSurfaceZ = TopZ;
				}
				bWaterKnown = true;

				const FVector2D CompMin(Bounds.Origin.X - Bounds.BoxExtent.X, Bounds.Origin.Y - Bounds.BoxExtent.Y);
				const FVector2D CompMax(Bounds.Origin.X + Bounds.BoxExtent.X, Bounds.Origin.Y + Bounds.BoxExtent.Y);
				if (!bWaterBoundsKnown)
				{
					WaterMin = CompMin;
					WaterMax = CompMax;
					bWaterBoundsKnown = true;
				}
				else
				{
					WaterMin.X = FMath::Min(WaterMin.X, CompMin.X);
					WaterMin.Y = FMath::Min(WaterMin.Y, CompMin.Y);
					WaterMax.X = FMath::Max(WaterMax.X, CompMax.X);
					WaterMax.Y = FMath::Max(WaterMax.Y, CompMax.Y);
				}
				continue;
			}

			if (IsObstacleMeshName(Mesh->GetName()))
			{
				const FVector Loc = Comp->GetComponentLocation();
				const FBoxSphereBounds Bounds = Comp->Bounds;
				const float Radius = FMath::Clamp(FMath::Max(Bounds.BoxExtent.X, Bounds.BoxExtent.Y), 40.f, MaxObstacleRadius);
				Obstacles.Add(FVector(Loc.X, Loc.Y, Radius));
			}
		}
	}

	if (bWaterBoundsKnown)
	{
		RiverCenter2D = 0.5f * (WaterMin + WaterMax);
		const FVector2D Size = WaterMax - WaterMin;
		if (Size.X >= Size.Y)
		{
			RiverSplitNormal = FVector2D(0.f, 1.f);
		}
		else
		{
			RiverSplitNormal = FVector2D(1.f, 0.f);
		}
		bRiverAxisKnown = true;
	}

	UE_LOG(LogTemp, Log, TEXT("[OreRush] Env gathered: %d obstacles, water=%s Z=%.1f riverAxis=%s center=(%.0f,%.0f) normal=(%.0f,%.0f)"),
		Obstacles.Num(), bWaterKnown ? TEXT("yes") : TEXT("no"), WaterSurfaceZ,
		bRiverAxisKnown ? TEXT("yes") : TEXT("no"),
		RiverCenter2D.X, RiverCenter2D.Y, RiverSplitNormal.X, RiverSplitNormal.Y);
}

bool AMapGenerator::FindLandscapeGround(const FVector& InLocation, FVector& OutGround, FVector& OutNormal) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FVector Start(InLocation.X, InLocation.Y, InLocation.Z + MaxTraceHeight);
	const FVector End(InLocation.X, InLocation.Y, InLocation.Z - MinTraceHeight);

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (!World->LineTraceMultiByChannel(Hits, Start, End, ECC_WorldStatic, Params))
	{
		return false;
	}

	for (const FHitResult& Hit : Hits)
	{
		if (!Hit.bBlockingHit)
		{
			continue;
		}
		const UPrimitiveComponent* Comp = Hit.GetComponent();
		if (Comp && Comp->GetClass()->GetName().Contains(TEXT("Landscape")))
		{
			OutGround = Hit.ImpactPoint;
			OutNormal = Hit.ImpactNormal;
			return true;
		}
	}

	return false;
}

bool AMapGenerator::IsBlockedByObstacle(const FVector2D& Point, float ExtraRadius) const
{
	for (const FVector& Ob : Obstacles)
	{
		const float Radius = Ob.Z + ExtraRadius;
		const float Dx = Point.X - Ob.X;
		const float Dy = Point.Y - Ob.Y;
		if (Dx * Dx + Dy * Dy < Radius * Radius)
		{
			return true;
		}
	}
	return false;
}

bool AMapGenerator::IsAboveWater(float GroundZ) const
{
	if (!bWaterKnown)
	{
		return true;
	}
	return GroundZ >= WaterSurfaceZ + RiverZMargin;
}

bool AMapGenerator::TryPickRaw(FRandomStream& Rng, FVector& OutPoint, float Clearance, bool bAllowObstacles, bool bAllowWater) const
{
	const FVector Origin = GetActorLocation();
	const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(MaxSlopeAngle));

	for (int32 Attempt = 0; Attempt < PickAttempts; ++Attempt)
	{
		const float X = Origin.X + Rng.FRandRange(-AreaExtent.X, AreaExtent.X);
		const float Y = Origin.Y + Rng.FRandRange(-AreaExtent.Y, AreaExtent.Y);
		const FVector2D P2(X, Y);

		if (!bAllowObstacles && IsBlockedByObstacle(P2, Clearance))
		{
			continue;
		}

		FVector Ground;
		FVector Normal;
		if (!FindLandscapeGround(FVector(X, Y, Origin.Z), Ground, Normal))
		{
			continue;
		}

		if (Normal.Z < CosThreshold)
		{
			continue;
		}

		if (!bAllowWater && !IsAboveWater(Ground.Z))
		{
			continue;
		}

		bool bOk = true;
		for (const FVector& Used : UsedPoints)
		{
			if (FVector::DistSquared2D(Ground, Used) < MinDistance * MinDistance)
			{
				bOk = false;
				break;
			}
		}
		if (!bOk)
		{
			continue;
		}

		OutPoint = Ground;
		return true;
	}

	return false;
}

bool AMapGenerator::PickPoint(FRandomStream& Rng, FVector& OutPoint, float Clearance)
{
	if (TryPickRaw(Rng, OutPoint, Clearance))
	{
		UsedPoints.Add(OutPoint);
		return true;
	}
	return false;
}

bool AMapGenerator::PickFarthestPair(FRandomStream& Rng, float Clearance, bool bAllowObstacles, bool bAllowWater, FVector& OutA, FVector& OutB)
{
	TArray<FVector> Candidates;
	const int32 Wanted = 20;

	for (int32 i = 0; i < Wanted; ++i)
	{
		FVector P;
		if (!TryPickRaw(Rng, P, Clearance, bAllowObstacles, bAllowWater))
		{
			continue;
		}

		bool bDup = false;
		for (const FVector& C : Candidates)
		{
			if (FVector::DistSquared2D(C, P) < MinDistance * MinDistance)
			{
				bDup = true;
				break;
			}
		}
		if (!bDup)
		{
			Candidates.Add(P);
		}
	}

	if (Candidates.Num() < 2)
	{
		return false;
	}

	float BestSq = -1.f;
	int32 BestA = 0;
	int32 BestB = 1;
	for (int32 a = 0; a < Candidates.Num(); ++a)
	{
		for (int32 b = a + 1; b < Candidates.Num(); ++b)
		{
			const float DSq = FVector::DistSquared2D(Candidates[a], Candidates[b]);
			if (DSq > BestSq)
			{
				BestSq = DSq;
				BestA = a;
				BestB = b;
			}
		}
	}

	if (Rng.FRand() < 0.5f)
	{
		OutA = Candidates[BestA];
		OutB = Candidates[BestB];
	}
	else
	{
		OutA = Candidates[BestB];
		OutB = Candidates[BestA];
	}

	return true;
}

float AMapGenerator::SideOfRiver(const FVector& Point) const
{
	const FVector2D Rel(Point.X - RiverCenter2D.X, Point.Y - RiverCenter2D.Y);
	return FVector2D::DotProduct(Rel, RiverSplitNormal);
}

bool AMapGenerator::PickSplitPair(FRandomStream& Rng, float Clearance, bool bAllowObstacles, bool bAllowWater, FVector& OutRed, FVector& OutBlue)
{
	TArray<FVector> PosSide;
	TArray<FVector> NegSide;
	const int32 Wanted = 40;

	for (int32 i = 0; i < Wanted; ++i)
	{
		FVector P;
		if (!TryPickRaw(Rng, P, Clearance, bAllowObstacles, bAllowWater))
		{
			continue;
		}

		TArray<FVector>& Bucket = (SideOfRiver(P) >= 0.f) ? PosSide : NegSide;

		bool bDup = false;
		for (const FVector& C : Bucket)
		{
			if (FVector::DistSquared2D(C, P) < MinDistance * MinDistance)
			{
				bDup = true;
				break;
			}
		}
		if (!bDup)
		{
			Bucket.Add(P);
		}
	}

	if (PosSide.Num() == 0 || NegSide.Num() == 0)
	{
		return false;
	}

	const float MinSepSq = DepotMinSeparation * DepotMinSeparation;
	float BestSq = -1.f;
	int32 BestP = -1;
	int32 BestN = -1;
	for (int32 p = 0; p < PosSide.Num(); ++p)
	{
		for (int32 n = 0; n < NegSide.Num(); ++n)
		{
			const float DSq = FVector::DistSquared2D(PosSide[p], NegSide[n]);
			if (DSq > BestSq)
			{
				BestSq = DSq;
				BestP = p;
				BestN = n;
			}
		}
	}

	if (BestP < 0 || BestN < 0)
	{
		return false;
	}

	if (BestSq < MinSepSq)
	{
		return false;
	}

	OutRed = PosSide[BestP];
	OutBlue = NegSide[BestN];
	return true;
}

bool AMapGenerator::PickDepotPair(FRandomStream& Rng, FVector& OutRed, FVector& OutBlue)
{
	bool bFound = false;

	if (bSplitDepotsByRiver && bRiverAxisKnown)
	{
		bFound = PickSplitPair(Rng, DepotClearance, false, false, OutRed, OutBlue);
		if (!bFound)
		{
			bFound = PickSplitPair(Rng, 0.f, false, false, OutRed, OutBlue);
		}
		if (!bFound)
		{
			bFound = PickSplitPair(Rng, 0.f, true, true, OutRed, OutBlue);
		}
	}

	if (!bFound)
	{
		bFound = PickFarthestPair(Rng, DepotClearance, false, false, OutRed, OutBlue);
	}
	if (!bFound)
	{
		bFound = PickFarthestPair(Rng, 0.f, false, false, OutRed, OutBlue);
	}
	if (!bFound)
	{
		bFound = PickFarthestPair(Rng, 0.f, true, true, OutRed, OutBlue);
	}

	if (bFound)
	{
		UsedPoints.Add(OutRed);
		UsedPoints.Add(OutBlue);
	}
	return bFound;
}

AOreVein* AMapGenerator::SpawnVein(EOreType Type, bool bUnlimited, int32 Units, const FVector& Location)
{
	TSubclassOf<AOreVein> Cls = VeinClass;
	if (!Cls)
	{
		Cls = AOreVein::StaticClass();
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, Location);
	AOreVein* Vein = GetWorld()->SpawnActorDeferred<AOreVein>(Cls, SpawnTransform, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Vein)
	{
		Vein->OreType = Type;
		Vein->bUnlimited = bUnlimited;
		Vein->RemainingUnits = Units;
		Vein->FinishSpawning(SpawnTransform);
	}
	return Vein;
}

int32 AMapGenerator::ScaledCount(int32 Base) const
{
	if (!bScaleWithArea || Base <= 0)
	{
		return Base;
	}

	const float RefSize = ReferenceArea.X * ReferenceArea.Y;
	if (RefSize <= 0.f)
	{
		return Base;
	}

	const float Scale = (AreaExtent.X * AreaExtent.Y) / RefSize;
	return FMath::Max(1, FMath::RoundToInt(Base * Scale));
}

void AMapGenerator::Generate(int32 Seed)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	UsedPoints.Reset();
	GatherEnvironment();

	FRandomStream Rng(Seed);

	const int32 NumIron = ScaledCount(IronCount);
	const int32 NumGold = ScaledCount(GoldCount);
	const int32 NumDiamond = ScaledCount(DiamondCount);
	const int32 NumPowerUp = ScaledCount(PowerUpCount);
	const int32 NumDecor = ScaledCount(DecorCount);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (DepotClass)
	{
		FVector RedLoc;
		FVector BlueLoc;
		if (PickDepotPair(Rng, RedLoc, BlueLoc))
		{
			UE_LOG(LogTemp, Log, TEXT("[OreRush] Depots Red=%s Blue=%s dist=%.0f (seed=%d)"),
				*RedLoc.ToString(), *BlueLoc.ToString(), FVector::Dist2D(RedLoc, BlueLoc), Seed);

			const FTransform RedT(FRotator::ZeroRotator, RedLoc);
			if (ADepotZone* Red = World->SpawnActorDeferred<ADepotZone>(DepotClass, RedT, this, nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				Red->Team = ETeam::Red;
				Red->FinishSpawning(RedT);
			}

			const FTransform BlueT(FRotator::ZeroRotator, BlueLoc);
			if (ADepotZone* Blue = World->SpawnActorDeferred<ADepotZone>(DepotClass, BlueT, this, nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				Blue->Team = ETeam::Blue;
				Blue->FinishSpawning(BlueT);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[OreRush] Depot placement failed (seed=%d) - not enough valid ground"), Seed);
		}
	}

	for (int32 i = 0; i < NumIron; ++i)
	{
		FVector P;
		if (PickPoint(Rng, P, ObstacleClearance))
		{
			SpawnVein(EOreType::Iron, true, 0, P);
		}
	}

	for (int32 i = 0; i < NumGold; ++i)
	{
		FVector P;
		if (PickPoint(Rng, P, ObstacleClearance))
		{
			SpawnVein(EOreType::Gold, false, GoldUnits, P);
		}
	}

	for (int32 i = 0; i < NumDiamond; ++i)
	{
		FVector P;
		if (PickPoint(Rng, P, ObstacleClearance))
		{
			SpawnVein(EOreType::Diamond, false, DiamondUnits, P);
		}
	}

	if (PowerUpClasses.Num() > 0)
	{
		for (int32 i = 0; i < NumPowerUp; ++i)
		{
			FVector P;
			if (!PickPoint(Rng, P, ObstacleClearance))
			{
				continue;
			}
			const int32 Idx = Rng.RandRange(0, PowerUpClasses.Num() - 1);
			if (TSubclassOf<APowerUpBase> Cls = PowerUpClasses[Idx])
			{
				World->SpawnActor<APowerUpBase>(Cls, P, FRotator::ZeroRotator, SpawnParams);
			}
		}
	}

	if (DecorClasses.Num() > 0)
	{
		for (int32 i = 0; i < NumDecor; ++i)
		{
			FVector P;
			if (!PickPoint(Rng, P, ObstacleClearance))
			{
				continue;
			}
			const int32 Idx = Rng.RandRange(0, DecorClasses.Num() - 1);
			if (TSubclassOf<AActor> Cls = DecorClasses[Idx])
			{
				const FRotator Yaw(0.f, Rng.FRandRange(0.f, 360.f), 0.f);
				World->SpawnActor<AActor>(Cls, P, Yaw, SpawnParams);
			}
		}
	}

	if (AOreRushGameMode* GM = World->GetAuthGameMode<AOreRushGameMode>())
	{
		GM->EnsurePlayersAtDepots();
	}
}
