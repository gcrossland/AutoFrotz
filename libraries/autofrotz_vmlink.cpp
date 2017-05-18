#include "autofrotz_vmlink.hpp"
#include <cstdio>
#include <cstring>

namespace autofrotz { namespace vmlink {

using core::u8string;
using std::copy;
using std::mutex;
using std::unique_lock;
using core::string;
using std::exception_ptr;
using std::rethrow_exception;
using bitset::Bitset;
using std::move;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
DC();

const u8string VmLink::EMPTY;

VmLink::VmLink (const char *zcodeFileName, iu screenWidth, iu screenHeight, iu undoDepth, bool enableWordSet) :
  isRunning(true), isDead(false), zcodeFileName(zcodeFileName), screenWidth(screenWidth), screenHeight(screenHeight), undoDepth(undoDepth), memorySize(0), dynamicMemorySize(0), dynamicMemory(nullptr), initialDynamicMemory(nullptr), wordSet(nullptr), inputI(EMPTY.end()), inputEnd(inputI), output(nullptr), saveState(nullptr), saveCount(0), restoreState(nullptr), restoreCount(0)
{
  DW(, "vmlink constructed");
  if (enableWordSet) {
    wordSet.reset(new Bitset());
  }
}

void VmLink::init (iu32 memorySize, iu16 dynamicMemorySize, const zbyte *dynamicMemory) {
  DW(, "vmlink initing with memorySize ", memorySize, ", dyn. memory size ", dynamicMemorySize);
  this->memorySize = memorySize;
  this->dynamicMemorySize = dynamicMemorySize;
  this->dynamicMemory = dynamicMemory;
  initialDynamicMemory.reset(new zbyte[dynamicMemorySize]);
  memcpy(initialDynamicMemory.get(), dynamicMemory, dynamicMemorySize);
  DW(, "word set enabled? ", !!wordSet.get());
  if (wordSet.get()) {
    wordSet->ensureWidth(dynamicMemorySize);
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
  Bitset *w = wordSet.get();
  if (w) {
    w->setExistingBit(addr);
  }
}

uchar VmLink::readInput () {
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

    if (isDead) {
      // We're supposed to be dead, so oblige.
      DW(, "input got... except we've been told to die");
      // DODGY
      throw core::PlainException(u8("VM has been killed"));
    }

    {
      char8_t b[(inputEnd - inputI) + 1];
      copy(inputI, inputEnd, b);
      b[inputEnd - inputI] = u8("\0")[0];
      DW(, "input got!! is **", b, "** (of length ", (inputEnd - inputI), ")");
    }
  }

  // TODO make inputI be a uchar iterator
  DPRE(*inputI < 128);
  return *(inputI++);
}

void VmLink::writeOutput (uchar c) {
  DPRE(!!output);

  // TODO wrapper for writing whole lines?
  DA(c < 256);
  // TODO make output be a uchar iterator
  if (c < 128) {
    output->push_back(static_cast<char8_t>(c));
  } else {
    output->push_back(static_cast<char8_t>(((c >> 6) & 0b00011111) | 0b11000000));
    output->push_back(static_cast<char8_t>((c & 0b00111111) | 0b10000000));
  }
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

void VmLink::completed (exception_ptr failureException) {
  DPRE(isRunning);

  unique_lock<mutex> l(lock);
  isRunning = false;
  isDead = true;
  this->failureException = failureException;
  condVar.notify_one();
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

const Bitset *VmLink::getWordSet () const noexcept {
  return wordSet.get();
}

void VmLink::setWordSet (Bitset &&initialWordSet) {
  wordSet.reset(new Bitset(move(initialWordSet)));
  wordSet->ensureWidth(dynamicMemorySize);
}

void VmLink::disableWordSet () noexcept {
  wordSet.reset();
}

bool VmLink::isAlive () const noexcept {
  return !isDead;
}

void VmLink::checkForFailure () const {
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

void VmLink::supplyInput (u8string::const_iterator inputBegin, u8string::const_iterator inputEnd) {
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

void VmLink::setOutput (u8string *output) {
  DPRE(!!output, "output must be non-null");

  this->output = output;
}

void VmLink::setSaveState (string<zbyte> *body) noexcept {
  saveState = body;
}

iu VmLink::getSaveCount () const noexcept {
  return saveCount;
}

void VmLink::resetSaveCount () noexcept {
  saveCount = 0;
}

void VmLink::setRestoreState (const string<zbyte> *body) noexcept {
  restoreState = body;
  DW(, "setting up restore state of size ",restoreState ? static_cast<is64>(restoreState->size()) : -1);
}

iu VmLink::getRestoreCount () const noexcept {
  return restoreCount;
}

void VmLink::resetRestoreCount () noexcept {
  restoreCount = 0;
}

void VmLink::kill () {
  DPRE(!isRunning);

  unique_lock<mutex> l(lock);
  if (isDead) {
    return;
  }
  isRunning = true;
  isDead = true;
  condVar.notify_one();
}

ZbyteReader::ZbyteReader (const zbyte *begin, const zbyte *end) :
  begin(begin), end(end), i(begin)
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

zbyte ZbyteReader::getByte () noexcept {
  DPRE(i < end);
  return *(i++);
}

zword ZbyteReader::getWord () noexcept {
  DPRE(i + 1 < end);
  zbyte h = *(i++);
  zbyte l = *(i++);
  return static_cast<zword>(static_cast<zword>(h << 8) | l);
}

void ZbyteReader::copy (zbyte *out, size_t s) noexcept {
  DPRE(i + s <= end);
  memcpy(out, i, s);
  i += s;
}

bool ZbyteReader::atEnd () const noexcept {
  DA(i <= end);
  return i == end;
}

ZbyteWriter::ZbyteWriter (string<zbyte> &r_b) :
  r_b(r_b), i(0), iAtEnd(true)
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

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
}}
