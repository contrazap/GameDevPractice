# 05 — The Replication Test Cube

**Goal:** Add an actor that visibly proves replication works, and place it in the map. This is your "did the network actually sync?" litmus test for every multiplayer step after this.

---

## Concept — what this cube proves

Multiplayer bugs are easy to miss because two players *look* like they're in sync even when they aren't. This cube removes all doubt by exercising **both kinds of replication** at once:

1. **Movement replication** — the server rotates and bobs it every tick. Clients don't run that logic; they receive the transform. (`SetReplicatingMovement`.)
2. **Property replication** — a `ColorIndex` marked `ReplicatedUsing = OnRep_ColorIndex` cycles every 2 s. When the server changes it, each client's `OnRep_ColorIndex()` fires and recolors the cube.

Key detail: **only the server drives state** (`if (!HasAuthority()) return;`). If the cube moves/colors identically on two machines, replication is genuinely working. If it drifts or freezes on the client, it isn't.

`OnRep` does **not** fire on the authority, so the host applies the color locally itself — that's why `OnRep_ColorIndex()` is also called directly in `Tick` and `BeginPlay`.

---

## Steps

### 5.1 — Create `ReplicatedTestCube.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReplicatedTestCube.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;

UCLASS()
class GTFOCLONE_API AReplicatedTestCube : public AActor
{
	GENERATED_BODY()

public:
	AReplicatedTestCube();
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_ColorIndex();

	UPROPERTY(VisibleAnywhere, Category = "Test")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(ReplicatedUsing = OnRep_ColorIndex)
	uint8 ColorIndex = 0;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Mid;

	float SpawnZ = 0.f;
	float ColorTimer = 0.f;
};
```

### 5.2 — Create `ReplicatedTestCube.cpp`

```cpp
#include "ReplicatedTestCube.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor CubeColors[] =
	{
		FLinearColor::Red,
		FLinearColor::Green,
		FLinearColor::Blue,
		FLinearColor::Yellow,
	};
}

AReplicatedTestCube::AReplicatedTestCube()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetMobility(EComponentMobility::Movable);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AReplicatedTestCube::BeginPlay()
{
	Super::BeginPlay();
	SpawnZ = GetActorLocation().Z;
	Mid = Mesh->CreateAndSetMaterialInstanceDynamic(0);
	OnRep_ColorIndex();
}

void AReplicatedTestCube::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Server drives all state; clients only receive replication.
	if (!HasAuthority())
	{
		return;
	}

	AddActorWorldRotation(FRotator(0.f, 60.f * DeltaSeconds, 0.f));

	FVector Location = GetActorLocation();
	Location.Z = SpawnZ + 60.f * FMath::Sin(GetWorld()->GetTimeSeconds() * 1.5f);
	SetActorLocation(Location);

	ColorTimer += DeltaSeconds;
	if (ColorTimer >= 2.f)
	{
		ColorTimer = 0.f;
		ColorIndex = (ColorIndex + 1) % UE_ARRAY_COUNT(CubeColors);
		OnRep_ColorIndex(); // OnRep doesn't fire on the authority; apply locally for the listen server
	}
}

void AReplicatedTestCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AReplicatedTestCube, ColorIndex);
}

void AReplicatedTestCube::OnRep_ColorIndex()
{
	if (Mid)
	{
		Mid->SetVectorParameterValue(TEXT("Color"), CubeColors[ColorIndex]);
	}
}
```

> The material param is named `Color`. The engine's default `BasicShapeMaterial` may not expose a `Color` parameter — if the cube stays grey, assign the mesh a simple material that has a `Color` vector parameter, or the color-cycle half of the test won't show (rotation/bob will still work).

### 5.3 — Place one in the map

Put a single `AReplicatedTestCube` in `/Game/ThirdPerson/Maps/ThirdPersonMap`, around `(300, 0, 300)`. Pick one:

- **Editor drag-drop (simplest, ~30 s):** in the Place Actors panel search `ReplicatedTestCube`, drag it in, set location, **save the level**.
- **Unreal MCP (you have it running):** ask the agent to spawn `/Script/GTFOClone.ReplicatedTestCube` at `(300,0,300)` in that map and save.
- **Headless Python** (needs PythonScriptPlugin enabled):
  ```python
  import unreal
  les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
  eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
  les.load_level("/Game/ThirdPerson/Maps/ThirdPersonMap")
  cube_cls = unreal.load_class(None, "/Script/GTFOClone.ReplicatedTestCube")
  cube = eas.spawn_actor_from_class(cube_cls, unreal.Vector(300, 0, 300))
  cube.set_actor_label("ReplicatedTestCube")
  les.save_current_level()
  ```

---

## ✅ Verify before continuing

1. **Rebuild succeeds** and `ReplicatedTestCube` appears in the Place Actors / C++ classes list.
2. The cube is placed in the map and the level is **saved**.
3. Press **Play (single-player PIE)**:
   - [ ] The cube **rotates** smoothly.
   - [ ] The cube **bobs** up and down.
   - [ ] The cube **cycles color** every ~2 s (if you gave it a `Color`-parameter material — see note above).

> Single-player is authoritative, so all three behaviors run locally here. This gate confirms the actor *works*; whether it stays **in sync across the network** is what files 06–08 prove. From here on, "the cube looks identical on both screens" is your quick health check.

Cube animates in PIE → open **`06_Local_MP_Test.md`** — the first real multiplayer test.
