#pragma once
#include "CoreMinimal.h"
class UStaticMesh; class UObject; class UMaterialInstanceDynamic; class UProceduralMeshComponent;

// Superelipsoide: a,b,c = semi-eixos (cm); e1/e2 = arredondamento (1 = elipsoide, ~0.4 = caixa/cilindro arredondado);
// taper = escala do topo (+Z) em relação à base; nu/nv = segmentos (baixo poligonal, normais suaves).
struct FJJShape
{
	float a = 10.f, b = 10.f, c = 10.f, e1 = 1.f, e2 = 1.f, taper = 1.f;
	int32 nu = 16, nv = 10;
};
inline FJJShape JJShape(float a, float b, float c, float e1 = 1.f, float e2 = 1.f, float taper = 1.f, int32 nu = 16, int32 nv = 10)
{
	FJJShape S; S.a = a; S.b = b; S.c = c; S.e1 = e1; S.e2 = e2; S.taper = taper; S.nu = nu; S.nv = nv; return S;
}

// Ajudantes visuais: malhas básicas da engine e material com cor/emissivo/rugosidade.
namespace JJ
{
	void BuildSuper(UProceduralMeshComponent* C, const FJJShape& S); // gera malha arredondada com normais suaves
	UStaticMesh* Mesh(const TCHAR* Name); // "Cube", "Cylinder", "Cone", "Sphere"
	UMaterialInstanceDynamic* Mat(UObject* Outer, FLinearColor Base, FLinearColor Emissive = FLinearColor::Black, float Rough = 0.8f, float Metal = 0.f);
}
