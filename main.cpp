#include "header.hpp"
#include <cstring>

using std::printf;
using autofrotz::Vm;
using autofrotz::State;
using std::strcmp;
using core::u8string;
using std::pair;
using std::get;
using std::exception;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
#define STATES 32
#define WIDTH 70
#define HEIGHT 128

DC();

int main (int argc, char *argv[]) {
  /*std::shared_ptr<core::debug::Stream> errs(new core::debug::Stream());
  DOPEN(, errs);
  autofrotz::DOPEN(, errs);
  autofrotz::vmlink::DOPEN(, errs);*/

  printf(
    "Welcome to the Simplest AutoFrotz Example I Could Think Of!\n"
    "===========================================================\n"
    "\n"
    "This proggy uses the AutoFrotz interface to do exactly\n"
    "what you could do before with the dumb build, but there's\n"
    "a new abstraction layer in place. Wow.\n"
    "\n"
    "I did warn you that it was the simplest example I could\n"
    "think of.\n"
    "\n"
    "\n"
  );

  printf("[Enter story file name:]\n");
  char zcodeFileName[PATH_MAX];
  readLine(reinterpret_cast<char8_t *>(zcodeFileName), sizeof(zcodeFileName));
  printf("\n\n");

  printf("[Initialising Z-machine:]\n");
  u8string output;
  Vm vm(zcodeFileName, WIDTH, HEIGHT, 1, true, output);

  printf("[Creating %d state slot%s:]\n", STATES, STATES == 1 ? "" : "s");
  State s[STATES];
  is currentSaveIndex = -1, currentRestoreIndex = -1;

  printOutput(output);
  output.clear();

  printf("[Checking that Z-machine did not die immediately:]\n");
  if (!vm.isAlive()) {
    printf("[Z-machine terminated]\n");
    return EXIT_FAILURE;
  }
  printf("[Z-machine still alive]\n");

  clock_t st = clock();
  do {
    char8_t inbuffer[512];
    readLine(inbuffer, sizeof(inbuffer));

    // Parse out our own internal commands for setting save and
    // restore state blocks.
    if (strcmp(inbuffer, u8("setsave")) == 0) {
      if (currentSaveIndex == -1) {
        printf("[No current save slot]\n");
      } else {
        printf("[Current save slot is %d]\n", currentSaveIndex);
      }

      printf("[Enter new save slot number:]\n");
      readLine(inbuffer, sizeof(inbuffer));
      int index = atoi(inbuffer);

      if (index == -1) {
        currentSaveIndex = -1;
        vm.setSaveState(nullptr);
        printf("[No new save slot]\n");
      } else if (index < 0 || index >= STATES) {
        printf("[Error: invalid state index]\n");
      } else {
        currentSaveIndex = index;
        vm.setSaveState(s + currentSaveIndex);
        printf("[New save slot is %d]\n", currentSaveIndex);
      }
    } else if (strcmp(inbuffer, u8("setrestore")) == 0) {
      if (currentRestoreIndex == -1) {
        printf("[No current restore slot]\n");
      } else {
        printf("[Current restore slot is %d]\n", currentRestoreIndex);
      }

      printf("[Enter new restore slot number:]\n");
      readLine(inbuffer, sizeof(inbuffer));
      int index = atoi(inbuffer);

      if (index == -1) {
        currentRestoreIndex = -1;
        vm.setRestoreState(nullptr);
        printf("[No new restore slot]\n");
      } else if (index < 0 || index >= STATES) {
        printf("[Error: invalid state index]\n");
      } else {
        currentRestoreIndex = index;
        vm.setRestoreState(s + currentRestoreIndex);
        printf("[New restore slot is %d]\n", currentRestoreIndex);
      }
    } else if (strcmp(inbuffer, u8("benchmark")) == 0) {
      vm.doAction(u8("verbose\n"), output);
      output.clear();

      const char8_t *const runInput = u8("east\ntake lamp\nexit\nwest\neast, east\ndrop lamp\nwest\n");
      pair<iu, iu> ts[] = {{2, 2}, {1, 4096}, {16, 256}, {256, 16}, {4096, 1}};
      for (auto t : ts) {
        const iu runsPerAction = get<0>(t);
        const iu actions = get<1>(t);

        printf("[Doing %d benchmarking runs for each of %d actions]\n", runsPerAction, actions);
        u8string in;
        for (iu i = 0; i < runsPerAction; ++i) {
          in.append(runInput);
        }
        clock_t st = clock();
        for (iu i = 0; i < actions; ++i) {
          vm.doAction(in, output);
          output.clear();
        }
        printf("[Run took %f secs]\n", static_cast<double>(clock() - st) / CLOCKS_PER_SEC);
      }
    } else {
      DA(vm.isAlive());

      u8string in(inbuffer);
      if (strcmp(inbuffer, u8(".")) == 0) {
        in = u8("\1\n");
      } else {
        in.push_back(u8("\n")[0]);
      }

      try {
        vm.doAction(in, output);
        printOutput(output);
        output.clear();
      } catch (exception &e) {
        printf("\n[Action failed (%s)]\n", e.what());
      }

      if (!vm.isAlive()) {
        printf("\n\n[Z-machine terminated]\n");
      }
      iu count = vm.getSaveCount();
      if (count > 0) {
        printf("\n[made %d successful save%s to current save slot]\n", count, count == 1 ? "" : "s");
      }
      count = vm.getRestoreCount();
      if (count>0) {
        printf("\n[made %d successful restorations%s from current restore slot]\n", count, count == 1 ? "" : "s");
      }
    }
  }
  while (vm.isAlive());
  printf("\nRun took %f secs\n", static_cast<double>(clock() - st) / CLOCKS_PER_SEC);

  return EXIT_SUCCESS;
}

size_t readLine (char8_t *b, size_t bSize) {
  fflush(stdout);

  // TODO convert input from native
  fgets(reinterpret_cast<char *>(b), static_cast<int>(bSize), stdin);
  size_t size = 0;
  for (; b[size] != 0; ++size) {
    if (b[size] >= 128) {
      b[size] = u8("?")[0];
    }
  }
  if (size > 0 && b[size - 1] == u8("\n")[0]) {
    b[--size] = u8("\0")[0];
  }
  return size;
}

int strcmp (const char8_t *o0, const char8_t *o1) {
  return strcmp(reinterpret_cast<const char *>(o0), reinterpret_cast<const char *>(o1));
}

int atoi (const char8_t *o) {
  return atoi(reinterpret_cast<const char *>(o));
}

void printOutput (const char8_t *begin, const char8_t *end) {
  // TODO convert output to native
  iu linelen = 0;
  printf("=* ");
  for (auto i = begin; i != end; ++i) {
    char8_t c = *i;
    if (c == u8("\n")[0]) {
      for (; linelen < WIDTH; linelen++) {
        putchar(' ');
      }
      printf(" <=\n=> ");
      linelen = 0;
    } else {
      putchar(c);
      linelen++;
    }
  }
}

void printOutput (const u8string &o) {
  const char8_t *begin = o.data();
  const char8_t *end = begin + o.size();
  printOutput(begin, end);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
