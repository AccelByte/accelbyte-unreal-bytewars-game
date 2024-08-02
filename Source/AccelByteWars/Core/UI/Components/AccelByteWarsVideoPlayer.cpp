// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsVideoPlayer.h"

#include "MediaPlayerFacade.h"

void UAccelByteWarsVideoPlayer::HandleMediaPlayerEvent(EMediaEvent EventType)
{
	Super::HandleMediaPlayerEvent(EventType);

	switch (EventType)
	{
	case EMediaEvent::MediaOpened:
		PlayFromStart();
		break;
	default: ;
	}
}
