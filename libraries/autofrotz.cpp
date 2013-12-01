#include "autofrotz.hpp"

// from Frotz
extern int common_main (int argc, char *argv[]);

namespace autofrotz {

using std::exception_ptr;
using std::exception;
using std::current_exception;
using std::thread;
using std::string;

const core::Version VERSION{LIB_MAJ, LIB_MIN}; DEPENDENCIES;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
DC();

Vm::Vm (const char *zcodeFileName, iu screenWidth, iu screenHeight, iu undoDepth, bool enableWordSet, string &r_output)
  : vmLink(zcodeFileName, screenWidth, screenHeight, undoDepth, enableWordSet), vmThread(new thread([this, &r_output] () {
    DPRE(!vmlink::vmLink, "a VM has already been created (and more than one is not supported)");

    vmlink::vmLink = &vmLink;

    exception_ptr failureException;
    try {
      DW(, "started thread");
      vmLink.setOutput(&r_output);
      common_main(0, nullptr);
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
    vmThread->join();
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

const iu8f *Vm::getWordSet() const noexcept {
  return vmLink.getWordSet();
}

bool Vm::isAlive () const noexcept {
  return vmLink.isAlive();
}

void Vm::doAction (string::const_iterator inputBegin, string::const_iterator inputEnd, string &r_output) {
  DW(, "doing action **", string(inputBegin, inputEnd).c_str(), "**");
  vmLink.setOutput(&r_output);
  vmLink.resetSaveCount();
  vmLink.resetRestoreCount();

  DW(, "giving input to VM...");
  vmLink.supplyInput(inputBegin, inputEnd);
  DW(, "... VM has consumed input");
  DW(, "output was **", r_output.c_str(), "**");

  vmLink.checkForFailure();
}

void Vm::doAction (const string &input, string &r_output) {
  doAction(input.begin(), input.end(), r_output);
}

iu Vm::getSaveCount() const noexcept {
  return vmLink.getSaveCount();
}

iu Vm::getRestoreCount() const noexcept {
  return vmLink.getRestoreCount();
}

void Vm::setSaveState(State *state) {
  vmLink.setSaveState(&(state->body));
}

void Vm::setRestoreState(const State *state) {
  vmLink.setRestoreState(&(state->body));
}

void State::clear () noexcept {
  body.clear();
}

void State::compact () {
  body.shrink_to_fit();
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
}
