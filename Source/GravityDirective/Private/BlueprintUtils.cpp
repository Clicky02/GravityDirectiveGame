// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintUtils.h"
#include "BaseVRPawn.h"

void UBlueprintUtils::SortByDistance(FVector Origin, TArray<AActor*> inArray, TArray<AActor*>& outArray)
{
	inArray.Sort([Origin](const AActor& A, const AActor& B) {
		float DistA = FVector::Dist(Origin, A.GetActorLocation());
		float DistB = FVector::Dist(Origin, B.GetActorLocation());
		return DistA < DistB;
	});
}

FVector UBlueprintUtils::GetLocation(AActor* Actor, bool bUseCamera=false)
{
	ABaseVRPawn* PawnRef = Cast<ABaseVRPawn>(Actor);

	if (PawnRef)
	{
		FVector ReturnVec = PawnRef->GetCameraPosition();

		if (bUseCamera)
		{
			return ReturnVec;
		}

		FVector ActorVec = PawnRef->GetActorLocation();

		ReturnVec.SetComponentForAxis(EAxis::Y, ActorVec.Y);

		return ReturnVec;
	}

	return Actor->GetActorLocation();
}

FVector UBlueprintUtils::GetVRPawnCenter(ABaseVRPawn* Pawn)
{

	FVector ReturnVec = Pawn->GetCameraPosition();

	FVector ActorVec = Pawn->GetActorLocation();

	ReturnVec.SetComponentForAxis(EAxis::Y, (ActorVec.Y + ReturnVec.Y) / 2);

	return ReturnVec;

}