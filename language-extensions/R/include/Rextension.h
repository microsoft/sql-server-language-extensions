#pragma once

// Define EXPORTED for any platform
#ifdef _WIN64
    #define EXPORTED  __declspec( dllexport )
#else
  #define EXPORTED
#endif