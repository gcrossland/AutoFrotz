/** @file */
/* -----------------------------------------------------------------------------
   AutoFrotz Library
   © Geoff Crossland 2003, 2006, 2013
----------------------------------------------------------------------------- */
#ifndef AUTOFROTZ_ALREADYINCLUDED
#define AUTOFROTZ_ALREADYINCLUDED

#include "autofrotz_vmlink.hpp"
#include <thread>

namespace autofrotz {

extern const core::Version VERSION;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
extern DC();

using vmlink::zbyte;
using vmlink::zword;

class State;

class Vm {
  prv vmlink::VmLink vmLink;
  prv std::unique_ptr<std::thread> vmThread;

  /**
    Starts a new Z-machine.

    @param zcodeFileName the Z-code file (giving the VM's initial memory) to
    run.
    @param enableWordSet whether ot not the VM will track which addresses are
    accessed as words.
  */
  pub Vm (char *zcodeFileName, iu screenWidth, iu screenHeight, iu undoDepth, bool enableWordSet);
  Vm (const Vm &) = delete;
  Vm &operator= (const Vm &) = delete;
  Vm (Vm &&) = delete;
  Vm &operator= (Vm &&) = delete;
  pub ~Vm () noexcept;

  /**
    Gets the Z-machine's memory size.
   */
  pub iu32 getMemorySize () const noexcept;
  /**
    Gets the Z-machine's dynamic memory size.
   */
  pub iu16 getDynamicMemorySize () const noexcept;
  /**
    Gets the Z-machine's dynamic memory (valid until ::isAlive() is {@c false}).
   */
  pub const zbyte *getDynamicMemory () const noexcept;
  /**
    Gets the Z-machine's dynamic memory as it looked immediately after loading
    the Z-code file (valid until destruction).
   */
  pub const zbyte *getInitialDynamicMemory () const noexcept;
  /**
    Gets the word set (valid until destruction) or {@c nullptr}, if not enabled.
    If a dynamic memory address {@c a} is used as the first byte of a word,
    {@c (getWordSet()[a >> 3] >> (a & 0x7)) & 0x1} is set.
   */
  pub const iu8f *getWordSet() const noexcept;
  /**
    Returns whether or not the Z-machine is alive.
   */
  pub bool isAlive () const noexcept;
  /**
    Passes input to the Z-machine and waits until it next requests input.

    @param input the input to pass to the Z-machine.
    @throw if the Z-machine failed while performing the action.
  */
  pub void doAction (const std::string &input);
  /**
    Gets the output from the Z-machine during the last action or before the
    first action.

    @return the output (valid until the next call to ::doAction()).
  */
  pub const std::string &getOutput () const noexcept;
  /**
    Gets the number of successful saves into the current save state during the
    last action.
   */
  pub iu getSaveCount() const noexcept;
  /**
    Gets the number of successful restorations into the current restore state
    during the last action.
   */
  pub iu getRestoreCount() const noexcept;
  /**
    Sets the State (valid until the next call to ::setSaveState() or
    destruction) into which the Z-machine will save when given the filename of
    character U+0001.
   */
  pub void setSaveState(State *state);
  /**
    Sets the State (valid until the next call to ::setRestoreState() or
    destruction) from which the Z-machine will restore when given the filename
    of character U+0001.
   */
  pub void setRestoreState(const State *state);
};

/**
  Stores the result of saving the state of the Z-machine.
 */
class State {
  prv std::basic_string<zbyte> body;

  /**
    Clears the state.
   */
  pub void clear () noexcept;
  /**
    Minimises the memory usage.
   */
  pub void compact ();

  friend class Vm;
};

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
}

#endif
