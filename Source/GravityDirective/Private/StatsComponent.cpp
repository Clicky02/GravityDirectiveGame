// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsComponent.h"
#include <Runtime/AIModule/Classes/AIController.h>
#include "BrainComponent.h"

TArray<AActor*> UStatsComponent::StatActors = TArray<AActor*>();


// Sets default values for this component's properties
UStatsComponent::UStatsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Health = CreateDefaultSubobject<UStat>("Health");
	Health->Type = EStatVariant::Health;
	Health->bIsActive = true;

	Armor = CreateDefaultSubobject<UStat>("Armor");
	Armor->Type = EStatVariant::Armor;

	Mana = CreateDefaultSubobject<UStat>("Mana");
	Mana->Type = EStatVariant::Mana;

	OutgoingDamageModifier = CreateDefaultSubobject<UStat>("OutgoingDamageModifier");
	OutgoingDamageModifier->Type = EStatVariant::OutgoingDamageModifier;
	OutgoingDamageModifier->BaseValue = 1;

	IncomingDamageModifier = CreateDefaultSubobject<UStat>("IncomingDamageModifier");
	IncomingDamageModifier->Type = EStatVariant::IncomingDamageModifier;
	IncomingDamageModifier->BaseValue = 1;
	// ...
}


// Called when the game starts
void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	Health->RefreshValues();
	Armor->RefreshValues();
	Mana->RefreshValues();
	OutgoingDamageModifier->RefreshValues();
	IncomingDamageModifier->RefreshValues();

	StatActors.Add(GetOwner());
	
}

void UStatsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsAlive)
	{
		Kill();
	}
}

UStat* UStatsComponent::GetStat(EStatVariant StatVariant)
{
	switch (StatVariant)
	{
	case EStatVariant::Health:
		return Health;
		break;
	case EStatVariant::Mana:
		return Mana;
		break;
	case EStatVariant::Armor:
		return Armor;
		break;
	case EStatVariant::OutgoingDamageModifier:
		return OutgoingDamageModifier;
		break;
	case EStatVariant::IncomingDamageModifier:
		return IncomingDamageModifier;
		break;
	default:
		return nullptr;
		break;
	};
}

// Called every frame
void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsAlive || bShouldTickWhenDead)
	{
		for (int32 i = 0; i < ActiveStatusEffects.Num(); i++)
		{
			ActiveStatusEffects[i]->Tick(DeltaTime);

			if (!ActiveStatusEffects[i]->IsActive())
			{
				ActiveStatusEffects.RemoveAt(i);
				i--;
			}
		}
	}
}

TArray<AActor*> UStatsComponent::GetStatActors()
{
	return StatActors;
}

int32 UStatsComponent::GetTeam()
{
	return Team;
}

void UStatsComponent::SetTeam(int32 NewTeam)
{
	Team = NewTeam;
}

void UStatsComponent::Damage(float Damage)
{
	float DamageDealt = Damage;

	if (Armor->bIsActive)
	{
		DamageDealt -= Armor->Value;
	}

	OutgoingDamageModifier[1];

	if (IncomingDamageModifier->bIsActive)
	{
		DamageDealt *= IncomingDamageModifier->Value;
	}

	if (DamageDealt < 0)
	{
		DamageDealt = 0;
	}


	Health->Add(-DamageDealt);
}

bool UStatsComponent::IsAlive()
{
	return bIsAlive;
}

bool UStatsComponent::Kill()
{
	bIsAlive = false;

	FOnDeathPayload Payload;
	Payload.StatsComponent = this;

	OnDeath.Broadcast(Payload);

	APawn* Pawn = Cast<APawn>(GetOwner());

	if (IsValid(Pawn))
	{
		AAIController* Controller = Cast<AAIController>(Pawn->GetController());

		if (IsValid(Controller))
		{
			Controller->GetBrainComponent()->StopLogic("Dead");
		}
	}

	StatActors.Remove(GetOwner());

	return true;
}

void UStatsComponent::ApplyStatusEffect(TSubclassOf<UStatusEffect> StatusEffect, float StartStacks, float StartDuration)
{
	UStatusEffect* CurrentEffect = GetStatusEffect(StatusEffect);

	if (CurrentEffect)
	{
		CurrentEffect->AddStacks(StartStacks);
	}
	else
	{
		CurrentEffect = NewObject<UStatusEffect>(this, StatusEffect);
		CurrentEffect->ApplyStatusEffect(this->GetOwner(), this, StartStacks, StartDuration);
		ActiveStatusEffects.Add(CurrentEffect);
	}
}

void UStatsComponent::RemoveStatusEffect(TSubclassOf<UStatusEffect> StatusEffect, float NumStacks)
{
	UStatusEffect* CurrentEffect = GetStatusEffect(StatusEffect);

	if (CurrentEffect)
	{
		if (NumStacks > 0)
		{
			CurrentEffect->RemoveStacks(NumStacks);
		}
		else
		{
			ActiveStatusEffects.Remove(CurrentEffect);
		}
	}
}

UStatusEffect* UStatsComponent::GetStatusEffect(TSubclassOf<UStatusEffect> StatusEffect)
{
	auto result = ActiveStatusEffects.FindByPredicate(
		[StatusEffect](const UStatusEffect* Object)
		{
			return Object->GetClass() == StatusEffect;
		}
	);

	if (result) 
	{
		return *result;
	}

	return nullptr;
}

int32 UStatsComponent::ApplyStatModifier(const FStatModifier& Modifier)
{
	UStat* Stat = GetStat(Modifier.StatType);
	if (Stat)
	{
		return Stat->ApplyStatModifier(Modifier);
	}
	
	return -1;
}

UPARAM(ref) FStatModifier& UStatsComponent::GetStatModifier(EStatVariant StatVariant, int32 Id)
{

	return GetStat(StatVariant)->GetStatModifier(Id);
}
