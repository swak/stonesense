#pragma once

#ifdef LINUX_BUILD
  #define SUSPEND_DF DF.Suspend()
  #define  RESUME_DF DF.Resume()
#else
  #define SUSPEND_DF ;
  #define  RESUME_DF ;
#endif


inline bool IDisWall(int in);
inline bool IDisFloor(int in);
void reloadDisplayedSegment();
void beautify_Segment(WorldSegment * segment);

void DisconnectFromDF();
bool IsConnectedToDF();

extern const VersionInfo *dfMemoryInfo;
