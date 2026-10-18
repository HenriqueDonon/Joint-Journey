#include "CoopSkelMesh.h"
#include "Engine/SkeletalMesh.h"
#include "ReferenceSkeleton.h"

void UCoopSkelMesh::FinalizeBoneTransform()
{
	USkeletalMesh* SM = GetSkeletalMeshAsset();
	if (AimAlpha > 0.01f && SM)
	{
		const FReferenceSkeleton& RS = SM->GetRefSkeleton();
		const int32 Up = RS.FindBoneIndex(TEXT("upperarm_r")), Low = RS.FindBoneIndex(TEXT("lowerarm_r")), Hand = RS.FindBoneIndex(TEXT("hand_r"));
		TArray<FTransform>& CS = GetEditableComponentSpaceTransforms();
		if (Up != INDEX_NONE && Low != INDEX_NONE && Hand != INDEX_NONE && CS.IsValidIndex(Hand) && CS.Num() == RS.GetNum())
		{
			const FVector Tgt = GetComponentTransform().InverseTransformVectorNoScale(AimDir).GetSafeNormal();
			const float A = FMath::Clamp(AimAlpha, 0.f, 1.f);
			// gira o osso Start e todos os filhos em torno de Pivot (corpo rigido, sem torcer a malha)
			auto Rot = [&](int32 Start, const FVector& Pivot, const FQuat& Q)
			{
				TArray<bool> Desc; Desc.Init(false, CS.Num());
				for (int32 i = Start; i < CS.Num(); ++i)
				{
					const int32 P = RS.GetParentIndex(i);
					if (i == Start || (P != INDEX_NONE && Desc[P]))
					{
						Desc[i] = true;
						CS[i].SetLocation(Pivot + Q.RotateVector(CS[i].GetLocation() - Pivot));
						CS[i].SetRotation((Q * CS[i].GetRotation()).GetNormalized());
					}
				}
			};
			if (!Tgt.IsNearlyZero())
			{
				// 1) braco superior avanca so um pouco (ombro quase parado); 2) antebraco dobra no cotovelo e aponta exatamente para o alvo
				const FVector Sh = CS[Up].GetLocation();
				const FVector U0 = (CS[Low].GetLocation() - Sh).GetSafeNormal();
				const FVector U1 = FMath::Lerp(U0, Tgt, 0.3f).GetSafeNormal();
				if (!U0.IsNearlyZero() && !U1.IsNearlyZero()) Rot(Up, Sh, FQuat::Slerp(FQuat::Identity, FQuat::FindBetweenNormals(U0, U1), A));
				const FVector El = CS[Low].GetLocation();
				const FVector F0 = (CS[Hand].GetLocation() - El).GetSafeNormal();
				if (!F0.IsNearlyZero()) Rot(Low, El, FQuat::Slerp(FQuat::Identity, FQuat::FindBetweenNormals(F0, Tgt), A));
			}
		}
	}
	Super::FinalizeBoneTransform();
}
