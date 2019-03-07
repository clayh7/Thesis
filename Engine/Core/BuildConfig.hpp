//-------------------------------------------------------------------------------------------------
//#define RELEASE

//-------------------------------------------------------------------------------------------------

// Memory Tracking (Default = 0)
// Tracks memory allocations on the heap
// Console Commands: memorydebug

#define MEMORY_TRACKING 0

// Vebosity Levels
// Basic
// Level 0 - Counts allocations and deallocations
// Verbose
// Level 1 - Tracks amount of data being allocated

//-------------------------------------------------------------------------------------------------

// Print messages Log or Output Window
// # = Warning Level (Default = 4)

#define LOG_WARNING_LEVEL 3
#define DEBUG_WARNING_LEVEL 3

// LOG_WARNING_LEVEL = filter for printing into the log file
// DEBUG_WARNING_LEVEL = filter for printing into the output window

// Warning Level 0 - Nothing tracked (Use this to turn off tracking)
// Warning Level 1 - Severe
// Warning Level 2 - Assert Recoverable
// Warning Level 3 - Default
// Warning Level 4 - Every Frame

//-------------------------------------------------------------------------------------------------

// Determines how many logs LoggerSystem will keep in /Data/Logs/...

#define MAX_LOG_HISTORY 10

//-------------------------------------------------------------------------------------------------

// Profiler, tracks how long parts of the program takes

#define DEBUG_PROFILER

//-------------------------------------------------------------------------------------------------