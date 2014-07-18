/** @file */
/* -----------------------------------------------------------------------------
   AutoFrotz VmLink Library
   © Geoff Crossland 2003, 2006, 2013, 2014
----------------------------------------------------------------------------- */
#ifndef AUTOFROTZ_VMLINK_ALREADYINCLUDED
#define AUTOFROTZ_VMLINK_ALREADYINCLUDED

#include <core.hpp>
#include <condition_variable>
#include <mutex>
#include <bitset.hpp>

namespace autofrotz { namespace vmlink {

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
#define vmlocal /* thread_local */

extern DC();

// from Frotz
typedef unsigned char zbyte;
typedef unsigned short zword;

class ZbyteReader;
class ZbyteWriter;

class VmLink {
  prv static const core::u8string EMPTY;

  // VM state + synchronisation stuff
  prv std::mutex lock;
  prv std::condition_variable condVar;
  prv volatile bool isRunning;
  prv bool isDead;
  prv std::exception_ptr failureException;
  // VM config
  prv core::string<char> zcodeFileName;
  prv iu screenWidth;
  prv iu screenHeight;
  prv iu undoDepth;
  prv bool enableWordSet;
  // VM properties
  prv iu32f memorySize;
  prv iu16f dynamicMemorySize;
  prv const zbyte *dynamicMemory;
  prv std::unique_ptr<zbyte []> initialDynamicMemory;
  prv std::unique_ptr<bitset::Bitset> wordSet;
  // I/O
  prv core::u8string::const_iterator inputI;
  prv core::u8string::const_iterator inputEnd;
  prv core::u8string *output;
  // Save and restore states
  prv core::string<zbyte> *saveState;
  iu saveCount;
  prv const core::string<zbyte> *restoreState;
  iu restoreCount;

  pub VmLink (const char *zcodeFileName, iu screenWidth, iu screenHeight, iu undoDepth, bool enableWordSet);
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
  pub uchar readInput ();
  pub void writeOutput (uchar c);
  pub ZbyteReader createInitialDynamicMemoryReader () const;
  pub bool hasSaveState () const noexcept;
  pub ZbyteWriter createSaveStateWriter ();
  pub void saveSucceeded () noexcept;
  pub bool hasRestoreState () const noexcept;
  pub ZbyteReader createRestoreStateReader () const;
  pub void restoreSucceeded () noexcept;
  pub void completed (std::exception_ptr failureException);

  pub iu32 getMemorySize () const noexcept;
  pub iu16 getDynamicMemorySize () const noexcept;
  pub const zbyte *getDynamicMemory () const noexcept;
  pub const zbyte *getInitialDynamicMemory () const noexcept;
  pub const bitset::Bitset *getWordSet () const noexcept;
  pub bool isAlive () const noexcept;
  pub void checkForFailure () const;
  pub void waitForInputExhaustion ();
  pub void supplyInput (core::u8string::const_iterator inputBegin, core::u8string::const_iterator inputEnd);
  pub void setOutput (core::u8string *output);
  pub void setSaveState (core::string<zbyte> *body) noexcept;
  pub iu getSaveCount () const noexcept;
  pub void resetSaveCount () noexcept;
  pub void setRestoreState (const core::string<zbyte> *body) noexcept;
  pub iu getRestoreCount () const noexcept;
  pub void resetRestoreCount () noexcept;
  pub void kill ();
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
  prv core::string<zbyte> &r_b;
  prv core::string<zbyte>::size_type i;
  prv bool iAtEnd;

  pub ZbyteWriter (core::string<zbyte> &r_b);
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
extern const iu lowerWindowHeadroom;

extern vmlocal VmLink *vmLink;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
}}

#endif
