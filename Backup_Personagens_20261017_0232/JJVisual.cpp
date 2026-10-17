#include "JJVisual.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"

UStaticMesh* JJ::Mesh(const TCHAR* Name)
{
	const FString P = FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"), Name, Name);
	return LoadObject<UStaticMesh>(nullptr, *P);
}

UMaterialInstanceDynamic* JJ::Mat(UObject* Outer, FLinearColor Base, FLinearColor Emissive, float Rough, float Metal)
{
	UMaterialInterface* Parent = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_JJ.M_JJ"));
	if (!Parent) Parent = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(Parent, Outer);
	if (M)
	{
		M->SetVectorParameterValue(TEXT("Color"), Base);
		M->SetVectorParameterValue(TEXT("Emissive"), Emissive);
		M->SetScalarParameterValue(TEXT("Roughness"), Rough);
		M->SetScalarParameterValue(TEXT("Metallic"), Metal);
	}
	return M;
}

static float SPow(float x, float e) { return FMath::Sign(x) * FMath::Pow(FMath::Abs(x), e); }

void JJ::BuildSuper(UProceduralMeshComponent* C, const FJJShape& S)
{
	if (!C) return;
	const int32 NU = FMath::Max(S.nu, 6), NV = FMath::Max(S.nv, 4);
	TArray<FVector> P; P.SetNum(NU * (NV + 1));
	TArray<FVector2D> UV; UV.SetNum(P.Num());
	for (int32 j = 0; j <= NV; j++)
	{
		const float v = -PI * 0.5f + PI * j / NV;
		const float cv = SPow(FMath::Cos(v), S.e1), sv = SPow(FMath::Sin(v), S.e1);
		const float k = 1.f + (S.taper - 1.f) * (sv * 0.5f + 0.5f);
		for (int32 i = 0; i < NU; i++)
		{
			const float u = 2.f * PI * i / NU;
			P[j * NU + i] = FVector(S.a * k * cv * SPow(FMath::Cos(u), S.e2), S.b * k * cv * SPow(FMath::Sin(u), S.e2), S.c * sv);
			UV[j * NU + i] = FVector2D((float)i / NU, (float)j / NV);
		}
	}
	TArray<int32> T;
	TArray<FVector> N; N.Init(FVector::ZeroVector, P.Num());
	auto AddTri = [&](int32 A, int32 B, int32 Cc) {
		FVector Cr = FVector::CrossProduct(P[B] - P[A], P[Cc] - P[A]);
		if (Cr.SizeSquared() < 1e-6f) return;
		if (FVector::DotProduct(Cr, P[A] + P[B] + P[Cc]) < 0.f) { Swap(B, Cc); Cr = -Cr; }
		T.Add(A); T.Add(B); T.Add(Cc);
		N[A] += Cr; N[B] += Cr; N[Cc] += Cr;
	};
	for (int32 j = 0; j < NV; j++)
		for (int32 i = 0; i < NU; i++)
		{
			const int32 i1 = (i + 1) % NU;
			const int32 a = j * NU + i, b = j * NU + i1, c = (j + 1) * NU + i, d = (j + 1) * NU + i1;
			AddTri(a, b, d); AddTri(a, d, c);
		}
	for (int32 i = 0; i < P.Num(); i++) N[i] = N[i].GetSafeNormal();
	for (int32 i = 0; i < NU; i++) { N[i] = FVector(0, 0, -1); N[NV * NU + i] = FVector(0, 0, 1); }
	C->CreateMeshSection_LinearColor(0, P, T, N, UV, TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
}
