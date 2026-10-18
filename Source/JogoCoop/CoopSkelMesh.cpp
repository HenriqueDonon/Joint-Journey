#include "CoopSkelMesh.h"
#include "Engine/SkeletalMesh.h"
#include "ReferenceSkeleton.h"

void UCoopSkelMesh::FinalizeBoneTransform()
{
	USkeletalMesh* SM = GetSkeletalMeshAsset();
	if (AimAlpha > 0.01f && SM)
	{
		const FReferenceSkeleton& RS = SM->GetRefSkeleton();
		const int32 Up = RS.FindBoneIndex(TEXT("upperarm_r")), Hand = RS.FindBoneIndex(TEXT("hand_r"));
		TArray<FTransform>& CS = GetEditableComponentSpaceTransforms();
		if (Up != INDEX_NONE && Hand != INDEX_NONE && CS.IsValidIndex(Hand) && CS.Num() == RS.GetNum())
		{
			const FVector Pivot = CS[Up].GetLocation();
			const FVector Cur = (CS[Hand].GetLocation() - Pivot).GetSafeNormal();
			const FVector Tgt = GetComponentTransform().InverseTransformVectorNoScale(AimDir).GetSafeNormal();
			if (!Cur.IsNearlyZero() && !Tgt.IsNearlyZero())
			{
				const FQuat Q = FQuat::Slerp(FQuat::Identity, FQuat::FindBetweenNormals(Cur, Tgt), FMath::Clamp(AimAlpha, 0.f, 1.f));
				TArray<bool> Desc; Desc.Init(false, CS.Num());
				for (int32 i = Up; i < CS.Num(); ++i)
				{
					const int32 P = RS.GetParentIndex(i);
					if (i == Up || (P != INDEX_NONE && Desc[P]))
					{
						Desc[i] = true;
						CS[i].SetLocation(Pivot + Q.RotateVector(CS[i].GetLocation() - Pivot));
						CS[i].SetRotation((Q * CS[i].GetRotation()).GetNormalized());
					}
				}
			}
		}
	}
	Super::FinalizeBoneTransform();
}
