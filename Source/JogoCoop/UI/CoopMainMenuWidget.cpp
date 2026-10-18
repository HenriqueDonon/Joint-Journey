#include "CoopMainMenuWidget.h"
#include "../CoopGameUserSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "FileMediaSource.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateColorBrush.h"

void UCoopMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildUI();
	ShowMain();

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->SetShowMouseCursor(true);
	}
	if (UCoopGameUserSettings* S = UCoopGameUserSettings::Get()) S->ApplyAudio();
}

UButton* UCoopMainMenuWidget::MakeMenuButton(UVerticalBox* Parent, const FString& Label, bool bBig)
{
	UButton* Btn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	FButtonStyle St = Btn->WidgetStyle;
	St.Normal.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f));
	St.Hovered.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.10f));
	St.Pressed.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.18f));
	Btn->SetStyle(St);

	UTextBlock* Txt = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Txt->SetText(FText::FromString(Label));
	Txt->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), bBig ? 32 : 22));
	Txt->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, bBig ? 1.f : 0.85f)));
	Btn->AddChild(Txt);

	UVerticalBoxSlot* VSlot = Parent->AddChildToVerticalBox(Btn);
	VSlot->SetPadding(FMargin(0.f, 6.f, 0.f, 6.f));
	return Btn;
}

void UCoopMainMenuWidget::BuildUI()
{
	Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = Canvas;

	// video de fundo em loop
	MediaPlayer = NewObject<UMediaPlayer>(this);
	MediaPlayer->SetLooping(true);
	MediaTexture = NewObject<UMediaTexture>(this);
	MediaTexture->AutoClear = true;
	MediaTexture->SetMediaPlayer(MediaPlayer);
	MediaTexture->UpdateResource();
	MediaSource = NewObject<UFileMediaSource>(this);
	const FString VideoPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Movies/MenuLoop.mp4"));
	MediaSource->SetFilePath(VideoPath);

	BgImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	FSlateBrush BgBrush;
	BgBrush.SetResourceObject(MediaTexture);
	BgBrush.ImageSize = FVector2D(1280.f, 720.f);
	BgBrush.DrawAs = ESlateBrushDrawType::Image;
	BgImage->SetBrush(BgBrush);
	UCanvasPanelSlot* BgSlot = Canvas->AddChildToCanvas(BgImage);
	BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	BgSlot->SetOffsets(FMargin(0.f));

	MediaPlayer->OpenSource(MediaSource);
	MediaPlayer->Play();

	// sombra pro texto legivel no canto inferior esquerdo
	UImage* Overlay = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
	Overlay->SetBrush(FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 0.55f)));
	UCanvasPanelSlot* OvSlot = Canvas->AddChildToCanvas(Overlay);
	OvSlot->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));
	OvSlot->SetAlignment(FVector2D(0.f, 1.f));
	OvSlot->SetPosition(FVector2D(0.f, 0.f));
	OvSlot->SetSize(FVector2D(620.f, 460.f));

	// titulo
	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Title->SetText(FText::FromString(TEXT("JOINT JOURNEY")));
	Title->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 40));
	Title->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	UCanvasPanelSlot* TSlot = Canvas->AddChildToCanvas(Title);
	TSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
	TSlot->SetPosition(FVector2D(48.f, 40.f));
	TSlot->SetAutoSize(true);

	MainBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	CampaignBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	SettingsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

	auto AnchorBottomLeft = [&](UWidget* W)
	{
		UCanvasPanelSlot* S = Canvas->AddChildToCanvas(W);
		S->SetAnchors(FAnchors(0.f, 1.f, 0.f, 1.f));
		S->SetAlignment(FVector2D(0.f, 1.f));
		S->SetPosition(FVector2D(48.f, -48.f));
		S->SetAutoSize(true);
	};
	AnchorBottomLeft(MainBox);
	AnchorBottomLeft(CampaignBox);
	AnchorBottomLeft(SettingsBox);

	UButton* BtnCampanha = MakeMenuButton(MainBox, TEXT("INICIAR CAMPANHA"), true);
	BtnCampanha->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::OnCampanhaClicked);
	UButton* BtnConfig = MakeMenuButton(MainBox, TEXT("CONFIGURAÇÕES"), false);
	BtnConfig->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::OnConfigClicked);

	UButton* Btn2 = MakeMenuButton(CampaignBox, TEXT("2 JOGADORES"), true);
	Btn2->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::On2PlayersClicked);
	UButton* Btn3 = MakeMenuButton(CampaignBox, TEXT("3 JOGADORES"), true);
	Btn3->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::On3PlayersClicked);
	UButton* BtnBack1 = MakeMenuButton(CampaignBox, TEXT("VOLTAR"), false);
	BtnBack1->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::OnBackClicked);

	BuildSettingsPanel();
}

void UCoopMainMenuWidget::BuildSettingsPanel()
{
	UTextBlock* Header = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Header->SetText(FText::FromString(TEXT("CONFIGURAÇÕES")));
	Header->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 28));
	Header->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	SettingsBox->AddChildToVerticalBox(Header);

	UCoopGameUserSettings* Settings = UCoopGameUserSettings::Get();

	auto AddLabel = [&](const FString& Text)
	{
		UTextBlock* L = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		L->SetText(FText::FromString(Text));
		L->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 18));
		L->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.85f)));
		UVerticalBoxSlot* VS = SettingsBox->AddChildToVerticalBox(L);
		VS->SetPadding(FMargin(0.f, 10.f, 0.f, 2.f));
	};

	AddLabel(TEXT("Volume"));
	USlider* VolSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass());
	VolSlider->SetMinValue(0.f);
	VolSlider->SetMaxValue(1.f);
	VolSlider->SetValue(Settings ? Settings->MasterVolume : 1.f);
	VolSlider->OnValueChanged.AddDynamic(this, &UCoopMainMenuWidget::OnVolumeChanged);
	UVerticalBoxSlot* VolSlot = SettingsBox->AddChildToVerticalBox(VolSlider);
	VolSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	VolSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

	AddLabel(TEXT("Sensibilidade do mouse"));
	USlider* SensSlider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass());
	SensSlider->SetMinValue(0.2f);
	SensSlider->SetMaxValue(3.f);
	SensSlider->SetValue(Settings ? Settings->MouseSensitivity : 1.f);
	SensSlider->OnValueChanged.AddDynamic(this, &UCoopMainMenuWidget::OnSensitivityChanged);
	UVerticalBoxSlot* SensSlot = SettingsBox->AddChildToVerticalBox(SensSlider);
	SensSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	SensSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

	AddLabel(TEXT("Tela"));
	UButton* BtnWin = MakeMenuButton(SettingsBox, TEXT("Janela"), false);
	BtnWin->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::OnWindowedClicked);
	UButton* BtnFull = MakeMenuButton(SettingsBox, TEXT("Tela cheia"), false);
	BtnFull->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::OnFullscreenClicked);

	AddLabel(TEXT("Qualidade gráfica"));
	UButton* BtnLow = MakeMenuButton(SettingsBox, TEXT("Baixa"), false);
	BtnLow->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::OnQualityLowClicked);
	UButton* BtnMed = MakeMenuButton(SettingsBox, TEXT("Média"), false);
	BtnMed->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::OnQualityMedClicked);
	UButton* BtnHigh = MakeMenuButton(SettingsBox, TEXT("Alta"), false);
	BtnHigh->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::OnQualityHighClicked);

	UButton* BtnBack2 = MakeMenuButton(SettingsBox, TEXT("VOLTAR"), false);
	BtnBack2->OnClicked.AddDynamic(this, &UCoopMainMenuWidget::OnBackClicked);
}

void UCoopMainMenuWidget::ShowMain()
{
	MainBox->SetVisibility(ESlateVisibility::Visible);
	CampaignBox->SetVisibility(ESlateVisibility::Collapsed);
	SettingsBox->SetVisibility(ESlateVisibility::Collapsed);
}
void UCoopMainMenuWidget::ShowCampaign()
{
	MainBox->SetVisibility(ESlateVisibility::Collapsed);
	CampaignBox->SetVisibility(ESlateVisibility::Visible);
	SettingsBox->SetVisibility(ESlateVisibility::Collapsed);
}
void UCoopMainMenuWidget::ShowSettings()
{
	MainBox->SetVisibility(ESlateVisibility::Collapsed);
	CampaignBox->SetVisibility(ESlateVisibility::Collapsed);
	SettingsBox->SetVisibility(ESlateVisibility::Visible);
}

void UCoopMainMenuWidget::StartCampaign(int32 NumPlayers)
{
	UGameplayStatics::OpenLevel(this, FName(TEXT("Sala1")), true, FString::Printf(TEXT("listen?Players=%d"), NumPlayers));
}

void UCoopMainMenuWidget::OnCampanhaClicked() { ShowCampaign(); }
void UCoopMainMenuWidget::OnConfigClicked() { ShowSettings(); }
void UCoopMainMenuWidget::OnBackClicked() { ShowMain(); }
void UCoopMainMenuWidget::On2PlayersClicked() { StartCampaign(2); }
void UCoopMainMenuWidget::On3PlayersClicked() { StartCampaign(3); }

void UCoopMainMenuWidget::OnFullscreenClicked()
{
	if (UGameUserSettings* S = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		S->SetFullscreenMode(EWindowMode::Fullscreen);
		S->ApplySettings(false);
	}
}
void UCoopMainMenuWidget::OnWindowedClicked()
{
	if (UGameUserSettings* S = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		S->SetFullscreenMode(EWindowMode::Windowed);
		S->ApplySettings(false);
	}
}
static void ApplyQuality(int32 Level)
{
	if (UGameUserSettings* S = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		S->SetOverallScalabilityLevel(Level);
		S->ApplySettings(false);
	}
}
void UCoopMainMenuWidget::OnQualityLowClicked() { ApplyQuality(0); }
void UCoopMainMenuWidget::OnQualityMedClicked() { ApplyQuality(2); }
void UCoopMainMenuWidget::OnQualityHighClicked() { ApplyQuality(3); }

void UCoopMainMenuWidget::OnVolumeChanged(float Value)
{
	if (UCoopGameUserSettings* S = UCoopGameUserSettings::Get())
	{
		S->MasterVolume = Value;
		S->ApplyAudio();
		S->SaveSettings();
	}
}
void UCoopMainMenuWidget::OnSensitivityChanged(float Value)
{
	if (UCoopGameUserSettings* S = UCoopGameUserSettings::Get())
	{
		S->MouseSensitivity = Value;
		S->SaveSettings();
	}
}
