// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/InputChord.h"
#include "GenericPlatform/GenericApplication.h"
#include "InputCoreTypes.h"
#include "UObject/ObjectMacros.h"

// generated.h
#include "BlackboxFInputChord.generated.h"

/** A Blackbox Input Chord is a custom of FInputChord without cmd key (current Blackbox limitation). */
USTRUCT(BlueprintType)
struct FBlackboxInputChord {
    GENERATED_USTRUCT_BODY()

    /** The Key is the core of the chord. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Key)
    FKey Key;

    /** Whether the shift key is part of the chord.  */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modifier, Meta = (DisplayName = "Shift - 1st modifier"))
    uint32 bShift : 1;

    /** Whether the control key is part of the chord.  */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modifier, Meta = (DisplayName = "Ctrl - 2nd modifier"))
    uint32 bCtrl : 1;

    /** Whether the alt key is part of the chord.  */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Modifier, Meta = (DisplayName = "Alt - 3rd modifier"))
    uint32 bAlt : 1;

#if PLATFORM_MAC
    bool HideCmdToggle = false;
#else
    bool HideCmdToggle = true;
#endif

    UPROPERTY(GlobalConfig)
    bool HideCmdToggleConfig = HideCmdToggle; // UPROPERTY must not be inside preprocessor blocks

    /** Whether the command key is part of the chord.  */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = Modifier,
        Meta = (DisplayName = "Cmd - 4th modifier"),
        Meta = (EditCondition = "!HideCmdToggleConfig", EditConditionHides, HideEditConditionToggle))
    uint32 bCmd : 1;

    FBlackboxInputChord()
        : Key(EKeys::Invalid)
        , bShift(false)
        , bCtrl(false)
        , bAlt(false)
        , bCmd(false)
    {
    }

    FBlackboxInputChord(const FKey InKey)
        : Key(InKey)
        , bShift(false)
        , bCtrl(false)
        , bAlt(false)
        , bCmd(false)
    {
    }

    FBlackboxInputChord(const FKey InKey, const bool bInShift, const bool bInCtrl, const bool bInAlt, const bool bInCmd)
        : Key(InKey)
        , bShift(bInShift)
        , bCtrl(bInCtrl)
        , bAlt(bInAlt)
        , bCmd(bInCmd)
    {
    }

    /**
     * Creates and initializes a new instance.
     *
     * @param InKey
     * @param InModifierKeys
     */
    FBlackboxInputChord(const EModifierKey::Type InModifierKeys, const FKey InKey)
        : Key(InKey)
        , bShift((InModifierKeys & EModifierKey::Shift) != 0)
        , bCtrl((InModifierKeys & EModifierKey::Control) != 0)
        , bAlt((InModifierKeys & EModifierKey::Alt) != 0)
        , bCmd((InModifierKeys & EModifierKey::Command) != 0)
    {
    }

    /**
     * Creates and initializes a new instance.
     *
     * @param InKey
     * @param InModifierKeys
     */
    FBlackboxInputChord(const FKey InKey, const EModifierKey::Type InModifierKeys)
        : Key(InKey)
        , bShift((InModifierKeys & EModifierKey::Shift) != 0)
        , bCtrl((InModifierKeys & EModifierKey::Control) != 0)
        , bAlt((InModifierKeys & EModifierKey::Alt) != 0)
        , bCmd((InModifierKeys & EModifierKey::Command) != 0)
    {
    }

    /**
     * Copy constructor.
     *
     * @param Other
     */
    FBlackboxInputChord(const FBlackboxInputChord& Other)
        : Key(Other.Key)
        , bShift(Other.bShift)
        , bCtrl(Other.bCtrl)
        , bAlt(Other.bAlt)
        , bCmd(Other.bCmd)
    {
    }

    /**
     * Compares this input chord with another for equality.
     *
     * @param Other The other chord to compare with.
     * @return true if the chords are equal, false otherwise.
     */
    bool operator==(const FBlackboxInputChord& Other) const
    {
        return (
            Key == Other.Key && bShift == Other.bShift && bCtrl == Other.bCtrl && bAlt == Other.bAlt &&
            bCmd == Other.bCmd);
    }

    /**
     * Compares this input chord with another for inequality.
     *
     * @param Other The other chord to compare with.
     * @return true if the chords are equal, false otherwise.
     */
    bool operator!=(const FBlackboxInputChord& Other) const { return !(*this == Other); }

    /**
     * @brief Assignment operator from a FInputChord object.
     *
     * @param FInputChord A FInputChord object to assign from.
     * @return  reference to the assigned FBlackboxInputChord object.
     */
    FBlackboxInputChord& operator=(const FInputChord& InputChord)
    {
        Key = InputChord.Key;
        bShift = InputChord.bShift;
        bAlt = InputChord.bAlt;
        bCtrl = InputChord.bCtrl;
        bCmd = InputChord.bCmd;

        return *this;
    }

    /**
     * @brief Conversion operator to FInputChord.
     *
     * @return A FInputChord.
     */
    operator FInputChord() const { return FInputChord(Key, bShift, bCtrl, bAlt, bCmd); }

#if PLATFORM_MAC
    bool NeedsControl() const { return bCmd; }
    bool NeedsCommand() const { return bCtrl; }
#else
    bool NeedsControl() const { return bCtrl; }
    bool NeedsCommand() const { return bCmd; }
#endif
    bool NeedsAlt() const { return bAlt; }
    bool NeedsShift() const { return bShift; }

    /**
     * Checks whether this chord requires an modifier keys to be pressed.
     *
     * @return true if modifier keys must be pressed, false otherwise.
     */
    bool HasAnyModifierKeys() const { return (bAlt || bCtrl || bShift || bCmd); }

    /**
     * Determines if this chord is valid.  A chord is valid if it has a non modifier key that must be pressed
     * and zero or more modifier keys that must be pressed
     *
     * @return true if the chord is valid
     */
    bool IsValidChord() const { return (Key.IsValid() && !Key.IsModifierKey()); }

    /**
     * Sets this chord to a new key and modifier state based on the provided template
     * Should not be called directly.  Only used by the key binding editor to set user defined keys
     */
    void Set(const FBlackboxInputChord& InTemplate) { *this = InTemplate; }

public:
    /**
     * Gets a type hash value for the specified chord.
     *
     * @param Chord The input chord to get the hash value for.
     * @return The hash value.
     */
    friend uint32 GetTypeHash(const FBlackboxInputChord& Chord)
    {
        return GetTypeHash(Chord.Key) ^ (Chord.bShift | Chord.bCtrl << 1 | Chord.bAlt << 2 | Chord.bCmd << 3);
    }
};