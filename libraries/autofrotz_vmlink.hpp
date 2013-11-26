/** @file */
/* -----------------------------------------------------------------------------
   AutoFrotz VmLink Library
   © Geoff Crossland 2003, 2006, 2013
----------------------------------------------------------------------------- */
#ifndef AUTOFROTZ_VMLINK_ALREADYINCLUDED
#define AUTOFROTZ_VMLINK_ALREADYINCLUDED

#include <core.hpp>
#include <condition_variable>
#include <mutex>

namespace autofrotz { namespace vmlink {

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
// from Frotz
typedef unsigned char zbyte;
typedef unsigned short zword;

class ZbyteReader;
class ZbyteWriter;

class VmLink {
  // TODO replace std::basic_string
  prv static const std::string EMPTY;

  // VM state + synchronisation stuff
  prv std::mutex lock;
  prv std::condition_variable condVar;
  prv volatile bool isRunning;
  prv bool isDead;
  prv std::exception_ptr failureException;
  // VM config
  prv std::string zcodeFileName;
  prv iu screenWidth;
  prv iu screenHeight;
  prv iu undoDepth;
  prv bool enableWordSet;
  // VM properties
  prv iu32f memorySize;
  prv iu16f dynamicMemorySize;
  prv const zbyte *dynamicMemory;
  prv std::unique_ptr<zbyte []> initialDynamicMemory;
  prv std::unique_ptr<iu8f []> wordSet;
  // I/O
  prv std::string::const_iterator inputI;
  prv std::string::const_iterator inputEnd;
  prv std::string output;
  // Save and restore states
  prv std::basic_string<zbyte> *saveState;
  iu saveCount;
  prv const std::basic_string<zbyte> *restoreState;
  iu restoreCount;

  pub VmLink (char *zcodeFileName, iu screenWidth, iu screenHeight, iu undoDepth, bool enableWordSet);
  VmLink (const VmLink &) = delete;
  VmLink &operator= (const VmLink &) = delete;
  VmLink (VmLink &&) = delete;
  VmLink &operator= (VmLink &&) = delete;
  pub void init (iu32 memorySize, iu16 dynamicMemorySize, const zbyte *dynamicMemory);

  pub const char *getZcodeFileName () const noexcept;
  pub iu getScreenWidth () const noexcept;
  pub iu getScreenHeight () const noexcept;
  pub iu getUndoDepth () const noexcept;
  pub void markWord (zword addr);
  pub char readInput ();
  pub void writeOutput (char c);
  pub ZbyteReader createInitialDynamicMemoryReader () const;
  pub bool hasSaveState () const noexcept;
  pub ZbyteWriter createSaveStateWriter ();
  pub void saveSucceeded () noexcept;
  pub bool hasRestoreState () const noexcept;
  pub ZbyteReader createRestoreStateReader () const;
  pub void restoreSucceeded () noexcept;

  pub iu32 getMemorySize () const noexcept;
  pub iu16 getDynamicMemorySize () const noexcept;
  pub const zbyte *getDynamicMemory () const noexcept;
  pub const zbyte *getInitialDynamicMemory () const noexcept;
  pub const iu8f *getWordSet () const noexcept;
  pub bool isAlive () const noexcept;
  pub void checkForFailure () const;
  pub void waitForInputExhaustion ();
  pub void supplyInput (std::string::const_iterator inputBegin, std::string::const_iterator inputEnd);
  pub void resetOutput () noexcept;
  pub const std::string &getOutput () const noexcept;
  pub void setSaveState (std::basic_string<zbyte> *body);
  pub iu getSaveCount () const noexcept;
  pub void resetSaveCount () noexcept;
  pub void setRestoreState (const std::basic_string<zbyte> *body);
  pub iu getRestoreCount () const noexcept;
  pub void resetRestoreCount () noexcept;
  pub void completed (std::exception_ptr failureException);
};

class ZbyteReader {
  prv const zbyte *begin;
  prv const zbyte *end;
  prv const zbyte *i;

  pub ZbyteReader (const zbyte *begin, const zbyte *end);

  pub long tell () const noexcept;
  pub bool seekTo (long offset);
  pub bool seekBy (long offset);
  pub iu16 getByte () noexcept;
  pub iu32 getWord () noexcept;
};

class ZbyteWriter {
  prv std::basic_string<zbyte> &r_b;
  prv std::basic_string<zbyte>::size_type i;
  prv bool iAtEnd;

  pub ZbyteWriter (std::basic_string<zbyte> &r_b);
  ZbyteWriter (const ZbyteWriter &) = delete;
  ZbyteWriter &operator= (const ZbyteWriter &) = delete;
  pub ZbyteWriter (ZbyteWriter &&) = default;
  pub ZbyteWriter &operator= (ZbyteWriter &&) = default;

  pub long tell () const noexcept;
  pub bool seekTo (long offset);
  pub bool seekBy (long offset);
  pub void setByte (zbyte b);
  pub void setWord (zword w);
};

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
#define vmlocal /* thread_local */

extern DC();
extern const iu lowerWindowHeadroom;
extern vmlocal VmLink *vmLink;

}}

#endif
