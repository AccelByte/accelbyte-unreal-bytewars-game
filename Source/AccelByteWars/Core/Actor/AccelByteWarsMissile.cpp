// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/Actor/AccelByteWarsMissile.h"

// Sets default values
AAccelByteWarsMissile::AAccelByteWarsMissile()
{
	// Setup missile mesh component
	MissileMeshComponenet = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MissileMeshComponenet"));
	RootComponent = MissileMeshComponenet;

	// Setup missile audio component
	MissileAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("MissileAudioComponent"));
	MissileAudioComponent->SetupAttachment(RootComponent);

	// Setup thrust sparks
	ThrustSparks = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ThrustSparks"));
	ThrustSparks->SetupAttachment(RootComponent);

	// Setup expiring sparks
	ExpiringSparks = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExpiringSparks"));
	ExpiringSparks->SetupAttachment(RootComponent);

	// Setup procedural mesh component
	AccelByteWarsProceduralMesh = CreateDefaultSubobject<UAccelByteWarsProceduralMeshComponent>(TEXT("AccelByteWarsProceduralMesh"));
	AccelByteWarsProceduralMesh->SetupAttachment(RootComponent);

	// Setup Gameplay Object Component
	AccelByteWarsGameplayObjectComponent = CreateDefaultSubobject<UAccelByteWarsGameplayObjectComponent>(TEXT("AccelByteWarsGameplayObjectComponent"));

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAccelByteWarsMissile::BeginPlay()
{
	Super::BeginPlay();

	// Ensure near hit ship list is empty on start
	NearHitShips.Empty();

	// Get Game State
	AAccelByteWarsInGameGameState* ABGameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (ABGameState == nullptr)
		return;

	// Initialize Score and ScoreIncrement
	Score = ABGameState->GameSetup.BaseScoreForKill;
	ScoreIncrement = ABGameState->GameSetup.SkimInitialScore;
	
	// Setup event for OnOwnerDestroyed (in blueprint)
	DestroyActorOnOwnerDestroyed();
}

// Called every frame
void AAccelByteWarsMissile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyGravityToThisGameObjects();
	ApplyOverallGravityForceToChangeTheVelocity(DeltaTime);
	ExpiryWindowBeforeTimeoutDestruction();
	DestroyOnTimeout();
	SkimmingAndScoreUpdate(DeltaTime);
	OnDestroyObject();
}

void AAccelByteWarsMissile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAccelByteWarsMissile, Color);
	DOREPLIFETIME(AAccelByteWarsMissile, Velocity);
	DOREPLIFETIME(AAccelByteWarsMissile, GravityForce);
}

bool AAccelByteWarsMissile::IsNearHitShip(UAccelByteWarsGameplayObjectComponent* ABObjectComponent)
{
	if (ABObjectComponent == nullptr)
		return false;

	if (ABObjectComponent->GetOwner() == nullptr)
		return false;

	float NearHitDistance = 200.0f;
	double distance = FVector::Distance(ABObjectComponent->GetOwner()->GetActorLocation(), GetActorLocation());

	if (ABObjectComponent->ObjectType == EGameplayObjectType::SHIP &&
		ABObjectComponent->GetOwner() != GetInstigator() &&
		distance <= NearHitDistance)
	{
		return true;
	}

	return false;
}

bool AAccelByteWarsMissile::IsSkimmingPlanet(TArray<UAccelByteWarsGameplayObjectComponent*> ABObjectComponent, UAccelByteWarsGameplayObjectComponent* ABGameplayObject)
{
	if (ABObjectComponent.Num() == 0)
		return false;

	if (ABGameplayObject == nullptr)
		return false;

	for(int i = 0; i < ABObjectComponent.Num(); i++)
	{
		if (ABObjectComponent[i] == nullptr)
			continue;

		float distance = GetSurfanceDistanceBetweenObjects(ABObjectComponent[i], ABGameplayObject);
		if (distance < 100.0f)
		{
			return true;
		}
	}

	return false;
}

float AAccelByteWarsMissile::GetSurfanceDistanceBetweenObjects(UAccelByteWarsGameplayObjectComponent* OtherObject, UAccelByteWarsGameplayObjectComponent* ThisObject)
{
	if (OtherObject == nullptr || ThisObject == nullptr)
		return 0.0f;

	if (OtherObject->GetOwner() == nullptr)
		return 0.0f;

	double distance = FVector::Distance(OtherObject->GetOwner()->GetActorLocation(), GetActorLocation());

	return (distance - ((OtherObject->Radius + ThisObject->Radius) * 100.0f));
}

void AAccelByteWarsMissile::GetGravityForceToObject(UAccelByteWarsGameplayObjectComponent* OtherObject, UAccelByteWarsGameplayObjectComponent* ThisObject, float& RetValue1, FVector& RetValue2)
{
	if (OtherObject == nullptr)
		return;

	if (OtherObject->GetOwner() == nullptr)
		return;

	if (ThisObject == nullptr)
		return;

	float a = OtherObject->Mass * 50.0f;
	float b = GravitationalConstant * a * ThisObject->Mass;
	float c = FVector::Distance(OtherObject->GetOwner()->GetActorLocation(), GetActorLocation());
	float d = FMath::Pow(c, 1.5f);
	float e = b / d;

	FVector f = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), OtherObject->GetOwner()->GetActorLocation());

	FVector g = f * e;

	RetValue1 = c;
	RetValue2 = g;
}

void AAccelByteWarsMissile::AlignWithVelocityDirection(FVector InVector)
{
	FVector forward_vector = UKismetMathLibrary::Cross_VectorVector(InVector, FVector(0.0f, 0.0f, 1.0f));

	FVector right_vector = GetActorTransform().GetRotation().GetRightVector();
	FVector up_vector = GetActorTransform().GetRotation().GetUpVector();

	FRotator new_rotator = UKismetMathLibrary::MakeRotationFromAxes(forward_vector, right_vector, up_vector);

	SetActorRotation(new_rotator);
}

void AAccelByteWarsMissile::Server_SetColor_Implementation(FLinearColor InColor)
{
	Color = InColor;
	OnRepNotify_Color();
}

void AAccelByteWarsMissile::OnRepNotify_Color()
{
	if (AccelByteWarsProceduralMesh == nullptr)
		return;

	AccelByteWarsProceduralMesh->UpdateColor(Color);
}

void AAccelByteWarsMissile::OnRepNotify_Velocity()
{

}

void AAccelByteWarsMissile::SetVelocity()
{
	Velocity = InitialSpeed * GetActorTransform().GetRotation().GetRightVector();
}

void AAccelByteWarsMissile::ApplyGravityToThisGameObjects()
{
	AAccelByteWarsInGameGameState* ABGameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (ABGameState == nullptr)
		return;

	if (AccelByteWarsGameplayObjectComponent == nullptr)
		return;

	GravityForce = FVector::ZeroVector;

	for (int i = 0; i < ABGameState->ActiveGameObjects.Num(); i++)
	{
		if (ABGameState->ActiveGameObjects[i] == nullptr)
			continue;

		UAccelByteWarsGameplayObjectComponent* GameObject = ABGameState->ActiveGameObjects[i];
		if (GameObject == nullptr)
			continue;

		if (IsNearHitShip(GameObject))
		{
			if (NearHitShips.Contains(Cast<AActor>(GameObject)) == false)
			{
				if (GameObject->GetOwner() == nullptr)
					continue;

				NearHitShips.Add(GameObject->GetOwner());

				APawn* Pawn = Cast<APawn>(GameObject->GetOwner());
				if (Pawn == nullptr)
					continue;

				APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
				if (PlayerController == nullptr)
					return;

				AAccelByteWarsInGameGameMode* ABInGameMode = Cast<AAccelByteWarsInGameGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
				if (ABInGameMode == nullptr)
					return;

				ABInGameMode->IncreasePlayerKilledAttempt(PlayerController);
			}
		}

		float ret_1 = 0.0f;
		FVector ret_2 = FVector::ZeroVector;
		GetGravityForceToObject(GameObject, AccelByteWarsGameplayObjectComponent, ret_1, ret_2);

		GravityForce = ret_2 + GravityForce;

		float a = ret_1 / 100.0f;
		float b = AccelByteWarsGameplayObjectComponent->Radius + GameObject->Radius;

		if (a < b)
		{
			HitObject = GameObject;
			KillActorThisFrame = true;
		}
	}
}

void AAccelByteWarsMissile::ApplyOverallGravityForceToChangeTheVelocity(float DeltaTime)
{
	if (AccelByteWarsGameplayObjectComponent == nullptr)
		return;

	FVector va = GravityForce / AccelByteWarsGameplayObjectComponent->Mass;
	FVector vb = va * DeltaTime;
	FVector vc = Velocity + vb;

	Velocity = vc;
	FVector vd = DeltaTime * Velocity;
	Server_SetMissileForwardVector_Implementation(vd, Velocity);
}

void AAccelByteWarsMissile::Server_SetMissileForwardVector_Implementation(FVector DeltaAdjustedVelocity, FVector NewVelocity)
{
	Velocity = NewVelocity;
	AddActorWorldOffset(DeltaAdjustedVelocity);
	AlignWithVelocityDirection(NewVelocity);

	OnRepNotify_Velocity();
}

void AAccelByteWarsMissile::ExpiryWindowBeforeTimeoutDestruction()
{
	float a = GetGameTimeSinceCreation();
	float b = MaxTimeAlive - ExpiryTime;

	if (a > b && Expiring == false)
	{
		if (ThrustSparks != nullptr)
			ThrustSparks->Deactivate();

		if (ExpiringSparks != nullptr)
			ExpiringSparks->Activate();

		Expiring = true;
	}
}

void AAccelByteWarsMissile::DestroyOnTimeout()
{
	if (HasAuthority() == false)
		return;

	if (GetGameTimeSinceCreation() > MaxTimeAlive)
	{
		KillActorThisFrame = true;
	}
}

void AAccelByteWarsMissile::SkimmingAndScoreUpdate(float DeltaTime)
{
	if (GetGameTimeSinceCreation() <= 1.0f)
		return;

	if (AccelByteWarsGameplayObjectComponent == nullptr)
		return;

	AAccelByteWarsInGameGameState* ABGameState = Cast<AAccelByteWarsInGameGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (ABGameState == nullptr)
		return;

	// Skimming Planet detection
	if (IsSkimmingPlanet(ABGameState->ActiveGameObjects, AccelByteWarsGameplayObjectComponent))
	{
		TimeSkimmingPlanet += DeltaTime;
		TimeSkimmingPlanetReward += DeltaTime;
	}
	else
	{
		TimeSkimmingPlanet = 0.0f;
		TimeSkimmingPlanetReward = 0.0f;
	}

	// Skimming planet reward
	if (TimeSkimmingPlanetReward > ABGameState->GameSetup.SkimScoreDeltaTime)
	{
		TimeSkimmingPlanetReward = 0.0f;
		Score += ScoreIncrement;

		int a = UKismetMathLibrary::FTrunc(ScoreIncrement);
		SpawnScorePopupHud(a);

		ScoreIncrement *= ABGameState->GameSetup.SkimScoreAdditionalMultiplier;
	}

	// Set TimeAlive
	TimeAlive += DeltaTime;

	// Update score based on alive time
	float fa = TimeAlive + DeltaTime;
	float fb = fa / ABGameState->GameSetup.TimeScoreDeltaTime;
	float fc = TimeAlive / ABGameState->GameSetup.TimeScoreDeltaTime;
	
	int ia = UKismetMathLibrary::FFloor(fb);
	int ib = UKismetMathLibrary::FFloor(fc);
	int ic = ia - ib;
	int id = ic * ABGameState->GameSetup.TimeScoreIncrement;

	float fd = id + Score;

	if (ic > 0)
	{
		Score = fd;
	}
}

void AAccelByteWarsMissile::OnDestroyObject()
{
	if (HasAuthority() == false)
		return;

	if (KillActorThisFrame == false)
		return;

	if (GetOwner() == nullptr)
		return;

	if (GetOwner()->GetInstigatorController() == nullptr)
		return;

	AAccelByteWarsPlayerController* ABPlayerController = Cast<AAccelByteWarsPlayerController>(GetOwner()->GetInstigatorController());
	if (ABPlayerController == nullptr)
		return;

	DecrementPlayerMissileCount();

	AAccelByteWarsInGameGameMode* ABInGameMode = Cast<AAccelByteWarsInGameGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (ABInGameMode == nullptr)
		return;

	if (HitObject != nullptr)
	{
		ABInGameMode->OnMissileDestroyed(GetActorLocation(), HitObject, Color, GetOwner());

		NearHitShips.Empty();
		Destroy();

		if (HitObject->ObjectType == EGameplayObjectType::SHIP)
		{
			float a = UKismetMathLibrary::FTrunc(Score);
			SpawnScorePopupHud(a);

			ABInGameMode->OnShipDestroyed(HitObject, Score, ABPlayerController);

			HitObject = nullptr;
		}
	}
	else
	{
		TreatMissileAsExpired(ABInGameMode);
	}
}

void AAccelByteWarsMissile::DecrementPlayerMissileCount()
{
	if (HasAuthority() == false)
		return;

	AAccelByteWarsPlayerController* ABPlayerController = Cast<AAccelByteWarsPlayerController>(GetOwner()->GetInstigatorController());
	if (ABPlayerController == nullptr)
		return;

	if (ABPlayerController->PlayerState == nullptr)
		return;

	AAccelByteWarsPlayerState* ABPlayerState = Cast<AAccelByteWarsPlayerState>(ABPlayerController->PlayerState);
	if (ABPlayerState == nullptr)
		return;

	ABPlayerState->MissilesFired--;
}