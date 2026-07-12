#pragma once

#include "Engine/DeveloperSettings.h"
#include "OnlineCoreSettings.generated.h"

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Online Core"))
class ONLINECORE_API UOnlineCoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Partitions EOS lobbies — only sessions sharing this bucket are found. */
	UPROPERTY(EditAnywhere, config, Category = "Session")
	FString BucketId;

	UPROPERTY(EditAnywhere, config, Category = "Session")
	FString DefaultReturnMap;
};
