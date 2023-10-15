#pragma once

#define BLACKBOX_UE_WINDOWS 0
#define BLACKBOX_UE_LINUX 0
#define BLACKBOX_UE_XBOXONE 0
#define BLACKBOX_UE_XBOXONEGDK 0
#define BLACKBOX_UE_XSX 0
#define BLACKBOX_UE_PS4 0
#define BLACKBOX_UE_PS5 0
#define BLACKBOX_UE_SONY 0
#define BLACKBOX_UE_MAC 0

#if defined(PLATFORM_WINDOWS) && PLATFORM_WINDOWS
#    if defined(BLACKBOX_UE_WINDOWS)
#        undef BLACKBOX_UE_WINDOWS
#    endif
#    define BLACKBOX_UE_WINDOWS 1
#elif defined(PLATFORM_LINUX) && PLATFORM_LINUX
#    if defined(BLACKBOX_UE_LINUX)
#        undef BLACKBOX_UE_LINUX
#    endif
#    define BLACKBOX_UE_LINUX 1
#elif defined(PLATFORM_MAC) && PLATFORM_MAC
#    if defined(BLACKBOX_UE_MAC)
#        undef BLACKBOX_UE_MAC
#    endif
#    define BLACKBOX_UE_MAC 1
#elif (defined(PLATFORM_XBOXONEGDK) && PLATFORM_XBOXONEGDK) || (defined(PLATFORM_XB1) && PLATFORM_XB1)
#    if defined(BLACKBOX_UE_XBOXONEGDK)
#        undef BLACKBOX_UE_XBOXONEGDK
#    endif
#    define BLACKBOX_UE_XBOXONEGDK 1
#elif (defined(PLATFORM_XBOXONE) && PLATFORM_XBOXONE) && !BLACKBOX_UE_XBOXONEGDK
#    if defined(BLACKBOX_UE_XBOXONE)
#        undef BLACKBOX_UE_XBOXONE
#    endif
#    define BLACKBOX_UE_XBOXONE 1
#elif defined(PLATFORM_XSX) && PLATFORM_XSX
#    if defined(BLACKBOX_UE_XSX)
#        undef BLACKBOX_UE_XSX
#    endif
#    define BLACKBOX_UE_XSX 1
#elif defined(PLATFORM_PS4) && PLATFORM_PS4
#    if defined(BLACKBOX_UE_PS4)
#        undef BLACKBOX_UE_PS4
#    endif
#    define BLACKBOX_UE_PS4 1
#elif defined(PLATFORM_PS5) && PLATFORM_PS5
#    if defined(BLACKBOX_UE_PS5)
#        undef BLACKBOX_UE_PS5
#    endif
#    define BLACKBOX_UE_PS5 1
#endif

#if BLACKBOX_UE_PS4 || BLACKBOX_UE_PS5
#    if defined(BLACKBOX_UE_SONY)
#        undef BLACKBOX_UE_SONY
#    endif
#    define BLACKBOX_UE_SONY 1
#endif