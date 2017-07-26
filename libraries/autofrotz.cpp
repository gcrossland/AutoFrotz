#include "autofrotz.hpp"
// from Frotz
extern int common_main (autofrotz::vmlink::VmLink *vmLink);

LIB_DEPENDENCIES

namespace autofrotz {

using std::exception_ptr;
using std::exception;
using std::current_exception;
using std::thread;
using core::u8string;
using bitset::Bitset;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
DC();

Vm::Vm (const char *zcodeFileName, iu screenWidth, iu screenHeight, iu undoDepth, bool enableWordSet, u8string &r_output) :
  vmLink(zcodeFileName, screenWidth, screenHeight, undoDepth, enableWordSet), vmThread(new thread([this, &r_output] () {
    exception_ptr failureException;
    try {
      DW(, "started thread");
      vmLink.setOutput(&r_output);
      common_main(&vmLink);
    } catch (exception &e) {
      DW(, "exception with msg **", e.what(), "** came out of thread");
      failureException = current_exception();
    } catch (...) {
      DW(, "unknown exception came out of thread");
      failureException = current_exception();
    }
    DW(, "vm thread over - failed? ", static_cast<bool>(failureException));

    vmLink.completed(failureException);
  }))
{
  vmLink.waitForInputExhaustion();
}

Vm::~Vm () noexcept {
  try {
    DW(, "destructing VM, so asking it do die");
    vmLink.kill();
    DW(, "VM told to die; waiting for thread to complete");
    vmThread->join();
    DW(, "joining completed");
  } catch (...) {
    DW(, "joining failed");
  }
}

iu32 Vm::getMemorySize () const noexcept {
  return vmLink.getMemorySize();
}

iu16 Vm::getDynamicMemorySize () const noexcept {
  return vmLink.getDynamicMemorySize();
}

const zbyte *Vm::getDynamicMemory () const noexcept {
  DPRE(isAlive(), "VM must be alive");

  return vmLink.getDynamicMemory();
}

const zbyte *Vm::getInitialDynamicMemory () const noexcept {
  return vmLink.getInitialDynamicMemory();
}

Bitset *Vm::getWordSet () noexcept {
  return vmLink.getWordSet();
}

bool Vm::isAlive () const noexcept {
  return vmLink.isAlive();
}

void Vm::doAction (u8string::const_iterator inputBegin, u8string::const_iterator inputEnd, u8string &r_output) {
  DW(, "doing action **", u8string(inputBegin, inputEnd).c_str(), "**");
  vmLink.setOutput(&r_output);
  vmLink.resetSaveCount();
  vmLink.resetRestoreCount();

  DW(, "giving input to VM...");
  vmLink.supplyInput(inputBegin, inputEnd);
  DW(, "... VM has consumed input");
  DW(, "output was **", r_output.c_str(), "**");

  vmLink.checkForFailure();
}

void Vm::doAction (const u8string &input, u8string &r_output) {
  doAction(input.begin(), input.end(), r_output);
}

iu Vm::getSaveCount () const noexcept {
  return vmLink.getSaveCount();
}

iu Vm::getRestoreCount () const noexcept {
  return vmLink.getRestoreCount();
}

void Vm::setSaveState (State *state) noexcept {
  vmLink.setSaveState(state ? &state->body : nullptr);
}

void Vm::setRestoreState (const State *state) noexcept {
  vmLink.setRestoreState(state ? &state->body : nullptr);
}

void State::clear () noexcept {
  body.clear();
}

bool State::isEmpty () noexcept {
  return body.empty();
}

void State::compact () {
  body.shrink_to_fit();
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
}
