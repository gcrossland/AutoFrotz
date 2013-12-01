#include "autofrotz_vmlink.hpp"
#include <cstdio>

namespace autofrotz { namespace vmlink {

using std::string;
using std::copy;
using std::mutex;
using std::unique_lock;
using std::basic_string;
using std::exception_ptr;
using std::rethrow_exception;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
DC();

const string VmLink::EMPTY;

VmLink::VmLink (const char *zcodeFileName, iu screenWidth, iu screenHeight, iu undoDepth, bool enableWordSet)
  : isRunning(true), isDead(false), zcodeFileName(zcodeFileName), screenWidth(screenWidth), screenHeight(screenHeight), undoDepth(undoDepth), enableWordSet(enableWordSet), memorySize(0), dynamicMemorySize(0), dynamicMemory(nullptr), initialDynamicMemory(nullptr), wordSet(nullptr), inputI(EMPTY.end()), inputEnd(inputI), output(nullptr), saveState(nullptr), saveCount(0), restoreState(nullptr), restoreCount(0)
{
  DW(, "vmlink constructed");
}

void VmLink::init (iu32 memorySize, iu16 dynamicMemorySize, const zbyte *dynamicMemory) {
  DW(, "vmlink initing with memorySize ", memorySize, ", dyn. memory size ", dynamicMemorySize);
  this->memorySize = memorySize;
  this->dynamicMemorySize = dynamicMemorySize;
  this->dynamicMemory = dynamicMemory;
  initialDynamicMemory.reset(new zbyte[dynamicMemorySize]);
  copy(dynamicMemory, dynamicMemory + dynamicMemorySize, initialDynamicMemory.get());
  DW(, "word set enabled? ", enableWordSet);
  if (enableWordSet) {
    wordSet.reset(new iu8f[(dynamicMemorySize + 7) >> 3]);
  }
}

const char *VmLink::getZcodeFileName () const noexcept {
  return zcodeFileName.c_str();
}

iu VmLink::getScreenWidth () const noexcept {
  return screenWidth;
}

iu VmLink::getScreenHeight () const noexcept {
  return screenHeight;
}

iu VmLink::getUndoDepth () const noexcept {
  return undoDepth;
}

void VmLink::markWord (zword addr) {
  iu8f *w = wordSet.get();
  if (w) {
    w[addr >> 3] |= static_cast<iu8f>(1 << (addr & 0x7));
  }
}

char VmLink::readInput () {
  DPRE(!isDead);
  DPRE(isRunning);

  while (inputI == inputEnd) {
    // There is no more input. Tell the main thread that we are done and wait
    // for more.
    DW(, "blocking for input");
    unique_lock<mutex> l(lock);
    isRunning = false;
    condVar.notify_one();
    condVar.wait(l, [this] () {
      return isRunning;
    });

    {
      char b[(inputEnd - inputI) + 1];
      std::copy(inputI, inputEnd, b);
      b[inputEnd - inputI] = '\0';
      DW(, "input got!! is **", b, "** (of length ", (inputEnd - inputI), ")");
    }
  }
  return *(inputI++);
}

void VmLink::writeOutput (char c) {
  DPRE(!!output);

  // TODO wrapper for writing whole lines?
  output->push_back(c);
}

ZbyteReader VmLink::createInitialDynamicMemoryReader () const {
  zbyte *m = initialDynamicMemory.get();
  return ZbyteReader(m, m + dynamicMemorySize);
}

bool VmLink::hasSaveState () const noexcept {
  return saveState;
}

ZbyteWriter VmLink::createSaveStateWriter () {
  DPRE(saveState);

  return ZbyteWriter(*saveState);
}

void VmLink::saveSucceeded () noexcept {
  DW(, "a save of size ",saveState->size()," succeeded");
  ++saveCount;
}

bool VmLink::hasRestoreState () const noexcept {
  return restoreState;
}

ZbyteReader VmLink::createRestoreStateReader () const {
  DPRE(restoreState);

  const zbyte *m = restoreState->data();
  return ZbyteReader(m, m + restoreState->size());
}

void VmLink::restoreSucceeded () noexcept {
  DW(, "a restore of size ",restoreState->size()," succeeded");
  ++restoreCount;
}

iu32 VmLink::getMemorySize () const noexcept {
  return memorySize;
}

iu16 VmLink::getDynamicMemorySize () const noexcept {
  return dynamicMemorySize;
}

const zbyte *VmLink::getDynamicMemory () const noexcept {
  return dynamicMemory;
}

const zbyte *VmLink::getInitialDynamicMemory () const noexcept {
  return initialDynamicMemory.get();
}

const iu8f *VmLink::getWordSet() const noexcept {
  return wordSet.get();
}

bool VmLink::isAlive() const noexcept {
  return !isDead;
}

void VmLink::checkForFailure() const {
  if (failureException) {
    rethrow_exception(failureException);
  }
}

void VmLink::waitForInputExhaustion () {
  unique_lock<mutex> l(lock);
  condVar.wait(l, [this] () {
    return !isRunning;
  });
}

void VmLink::supplyInput (string::const_iterator inputBegin, string::const_iterator inputEnd) {
  DPRE(!isRunning);

  unique_lock<mutex> l(lock);
  if (isDead) {
    return;
  }
  inputI = inputBegin;
  this->inputEnd = inputEnd;
  isRunning = true;
  condVar.notify_one();
  condVar.wait(l, [this] () {
    return !isRunning;
  });
}

void VmLink::setOutput (string *output) {
  DPRE(!!output, "output must be non-null");

  this->output = output;
}

void VmLink::setSaveState (std::basic_string<zbyte> *body) {
  saveState = body;
}

iu VmLink::getSaveCount () const noexcept {
  return saveCount;
}

void VmLink::resetSaveCount () noexcept {
  saveCount = 0;
}

void VmLink::setRestoreState (const std::basic_string<zbyte> *body) {
  restoreState = body;
  DW(, "setting up restore state of size ",restoreState->size());
}

iu VmLink::getRestoreCount () const noexcept {
  return restoreCount;
}

void VmLink::resetRestoreCount () noexcept {
  restoreCount = 0;
}

void VmLink::completed (exception_ptr failureException) {
  DPRE(isRunning);

  unique_lock<mutex> l(lock);
  isRunning = false;
  isDead = true;
  this->failureException = failureException;
  condVar.notify_one();
}

ZbyteReader::ZbyteReader (const zbyte *begin, const zbyte *end)
  : begin(begin), end(end), i(begin)
{
}

long ZbyteReader::tell () const noexcept {
  return i - begin;
}

bool ZbyteReader::seekTo (long offset) {
  const zbyte *i = begin + offset;
  if (i < begin || i > end) {
    return false;
  }

  this->i = i;
  return true;
}

bool ZbyteReader::seekBy (long offset) {
  return seekTo((i + offset) - begin);
}

iu16 ZbyteReader::getByte () noexcept {
  if (i >= end) {
    return EOF;
  }

  return *(i++);
}

iu32 ZbyteReader::getWord () noexcept {
  if (i + 1 >= end) {
    return EOF;
  }

  zbyte h = *(i++);
  zbyte l = *(i++);
  return static_cast<zword>(static_cast<zword>(h << 8) | l);
}

ZbyteWriter::ZbyteWriter (std::basic_string<zbyte> &r_b)
  : r_b(r_b), i(0), iAtEnd(true)
{
  r_b.clear();
}

long ZbyteWriter::tell () const noexcept {
  return static_cast<long>(i);
}

bool ZbyteWriter::seekTo (long offset) {
  auto size = r_b.size();
  if (offset < 0 || static_cast<unsigned long>(offset) > size) {
    return false;
  }

  i = static_cast<decltype(size)>(offset);
  iAtEnd = (i == size);
  return true;
}

bool ZbyteWriter::seekBy (long offset) {
  return seekTo(static_cast<long>(i) + offset);
}

void ZbyteWriter::setByte (zbyte b) {
  // TODO be less rubbish
  if (iAtEnd) {
    r_b.push_back(b);
    i++;
  } else {
    r_b[i++] = b;
    iAtEnd = (i == r_b.size());
  }
}

void ZbyteWriter::setWord (zword w) {
  // TODO be less rubbish
  zbyte bs[2] = {static_cast<zbyte>(w >> 8), static_cast<zbyte>(w)};
  if (iAtEnd) {
    r_b.append(bs, 2);
    i += 2;
  } else {
    setByte(bs[0]);
    setByte(bs[1]);
  }
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
const iu lowerWindowHeadroom = 32;

vmlocal VmLink *vmLink = nullptr;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
}}
