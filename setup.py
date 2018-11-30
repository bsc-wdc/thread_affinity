from distutils.core import setup, Extension

thread_affinity = \
Extension(
  "thread_affinity",
  include_dirs = ["ext/"],
  sources = ["ext/thread_affinity.cc"]
)

setup(
  name = "thread_affinity",
  version = "1.0.0",
  description = "A wrapper to call set & get affinity from python (linux only)",
  author = "The COMPSs Team",
  author_email = "sergio.rodriguez@bsc.es",
  url = "https://github.com/srgrr/thread_affinity"
)
