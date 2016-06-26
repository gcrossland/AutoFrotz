import sys
try:
  import sconsutils
except ImportError:
  raise ImportError("Failed to import sconsutils (is buildtools on PYTHONPATH?)"), None, sys.exc_traceback

env = sconsutils.getEnv()
env.InVariantDir(env['oDir'], ".", lambda env: env.LibAndApp('autofrotz', 0, -1, (
  ('core', 0, 0),
  ('bitset', 0, 0)
), lambda env, cpppath: (
  env.StaticObject(
    env.Glob("libraries/autofrotz_vm/common/*.cpp") + env.Glob("libraries/autofrotz_vm/auto/*.cpp"),
    CPPPATH = cpppath,
    CPPDEFINES = dict(env['CPPDEFINES'], AUTOFROTZ = None),
    CXXFLAGS = env['CXXFLAGS'] + {'gcc': ["-w"]}[env['tool']]
  ),
  ()
)))
