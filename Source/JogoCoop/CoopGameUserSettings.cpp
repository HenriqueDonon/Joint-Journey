#include "CoopGameUserSettings.h"
#include "Engine/Engine.h"
#include "AudioDevice.h"

UCoopGameUserSettings* UCoopGameUserSettings::Get()
{
	return Cast<UCoopGameUserSettings>(GEngine->GetGameUserSettings());
}

void UCoopGameUserSettings::ApplyAudio()
{
	if (GEngine)
	{
		if (FAudioDeviceHandle AD = GEngine->GetMainAudioDevice())
			AD->SetTransientPrimaryVolume(FMath::Clamp(MasterVolume, 0.f, 1.f));
	}
}
