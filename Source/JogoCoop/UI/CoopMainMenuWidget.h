#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CoopMainMenuWidget.generated.h"

class UCanvasPanel;
class UVerticalBox;
class UImage;
class UButton;
class UTextBlock;
class USlider;
class UPanelWidget;
class UMediaPlayer;
class UMediaTexture;
class UFileMediaSource;

// Menu principal estilo "It Takes Two": video de fundo em loop + opcoes de texto no canto inferior esquerdo.
UCLASS()
class JOGOCOOP_API UCoopMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	void BuildUI();
	void BuildSettingsPanel();
	UButton* MakeMenuButton(UVerticalBox* Parent, const FString& Label, bool bBig);
	void ShowMain();
	void ShowCampaign();
	void ShowSettings();
	void StartCampaign(int32 NumPlayers);

	UFUNCTION() void OnCampanhaClicked();
	UFUNCTION() void OnConfigClicked();
	UFUNCTION() void OnBackClicked();
	UFUNCTION() void On2PlayersClicked();
	UFUNCTION() void On3PlayersClicked();
	UFUNCTION() void OnFullscreenClicked();
	UFUNCTION() void OnWindowedClicked();
	UFUNCTION() void OnQualityLowClicked();
	UFUNCTION() void OnQualityMedClicked();
	UFUNCTION() void OnQualityHighClicked();
	UFUNCTION() void OnVolumeChanged(float Value);
	UFUNCTION() void OnSensitivityChanged(float Value);

	UPROPERTY() UCanvasPanel* Canvas;
	UPROPERTY() UImage* BgImage;
	UPROPERTY() UVerticalBox* MainBox;
	UPROPERTY() UVerticalBox* CampaignBox;
	UPROPERTY() UVerticalBox* SettingsBox;
	UPROPERTY() UMediaPlayer* MediaPlayer;
	UPROPERTY() UMediaTexture* MediaTexture;
	UPROPERTY() UFileMediaSource* MediaSource;
};
