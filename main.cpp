#include "header.hpp"
#include <cstring>

using std::printf;
using autofrotz::Vm;
using autofrotz::State;
using std::strcmp;
using std::string;
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
  readLine(zcodeFileName, sizeof(zcodeFileName));
  printf("\n\n");

  printf("[Initialising Z-machine:]\n");
  Vm vm(zcodeFileName, WIDTH, HEIGHT, 1, true);

  printf("[Creating %d state slot%s:]\n", STATES, STATES == 1 ? "" : "s");
  State s[STATES];
  is currentSaveIndex = -1, currentRestoreIndex = -1;

  printOutput(vm);

  printf("[Checking that Z-machine did not die immediately:]\n");
  if (!vm.isAlive()) {
    printf("[Z-machine terminated]\n");
    return EXIT_FAILURE;
  }
  printf("[Z-machine still alive]\n");

  clock_t st = clock();
  do {
    char inbuffer[512];
    readLine(inbuffer, sizeof(inbuffer));

    // Parse out our own internal commands for setting save and
    // restore state blocks.
    if (strcmp(inbuffer, "setsave") == 0) {
      if (currentSaveIndex == -1) {
        printf("[No current save slot]\n");
      } else {
        printf("[Current save slot is %d]\n", currentSaveIndex);
      }

      printf("[Enter new save slot number:]\n");
      readLine(inbuffer, sizeof(inbuffer));
      int index = atoi(inbuffer);

      if (index < 0 || index >= STATES) {
        printf("[Error: invalid state index]\n");
      } else {
        currentSaveIndex = index;
        vm.setSaveState(s + currentSaveIndex);
        printf("[New save slot is %d]\n", currentSaveIndex);
      }
    } else if (strcmp(inbuffer, "setrestore") == 0) {
      if (currentRestoreIndex == -1) {
        printf("[No current restore slot]\n");
      } else {
        printf("[Current restore slot is %d]\n", currentRestoreIndex);
      }

      printf("[Enter new restore slot number:]\n");
      readLine(inbuffer, sizeof(inbuffer));
      int index = atoi(inbuffer);

      if (index < 0 || index >= STATES) {
        printf("[Error: invalid state index]\n");
      } else {
        currentRestoreIndex = index;
        vm.setRestoreState(s + currentRestoreIndex);
        printf("[New restore slot is %d]\n", currentRestoreIndex);
      }
    } else if (strcmp(inbuffer, "benchmark") == 0) {
      vm.doAction("verbose\n");

      const char *const runInput = "east\ntake lamp\nexit\nwest\neast, east\ndrop lamp\nwest\n";
      pair<iu, iu> ts[] = {{2, 2}, {1, 4096}, {16, 256}, {256, 16}, {4096, 1}};
      for (auto t : ts) {
        const iu runsPerAction = get<0>(t);
        const iu actions = get<1>(t);

        printf("[Doing %d benchmarking runs for each of %d actions]\n", runsPerAction, actions);
        string in;
        for (iu i = 0; i < runsPerAction; ++i) {
          in.append(runInput);
        }
        clock_t st = clock();
        for (iu i = 0; i < actions; ++i) {
          vm.doAction(in);
        }
        printf("[Run took %f secs]\n", static_cast<double>(clock() - st) / CLOCKS_PER_SEC);
      }
    } else {
      DA(vm.isAlive());

      string in(inbuffer);
      if (strcmp(inbuffer, ".") == 0) {
        in = "\1\n";
      } else {
        in.push_back('\n');
      }

      try {
        vm.doAction(in);
        printOutput(vm);
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

size_t readLine (char *b, size_t bSize) {
  fgets(b, bSize, stdin);
  size_t size = strlen(b);
  if (size > 0 && b[size - 1] == '\n') {
    b[--size] = '\0';
  }
  return size;
}

void printOutput (const char *begin, const char *end) {
  iu linelen = 0;
  printf("=* ");
  for (auto i = begin; i != end; ++i) {
    char c = *i;
    if (c == '\n') {
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

void printOutput (const Vm &vm) {
  const string &o = vm.getOutput();
  const char *begin = o.data();
  const char *end = begin + o.size();
  printOutput(begin, end);
}
