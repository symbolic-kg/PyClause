from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

import sys, re
import setuptools
import pybind11
from glob import glob


# (c) Sylvain Corlay, https://github.com/pybind/python_example
def has_flag(compiler, flagname):

  import tempfile

  with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:

    f.write('int main (int argc, char **argv) { return 0; }')

    try:
      compiler.compile([f.name], extra_postargs=[flagname])
    except setuptools.distutils.errors.CompileError:
      return False

  return True


# (c) Sylvain Corlay, https://github.com/pybind/python_example
def cpp_flag(compiler):

  if   has_flag(compiler,'-std=c++14'): return '-std=c++14'
  elif has_flag(compiler,'-std=c++11'): return '-std=c++11'
  raise RuntimeError('Unsupported compiler: at least C++11 support is needed')


# (c) Sylvain Corlay, https://github.com/pybind/python_example
class BuildExt(build_ext):

  c_opts = {
    'msvc': ['/EHsc'],
    'unix': [],
  }

  def build_extensions(self):
    ct = self.compiler.compiler_type
    opts = self.c_opts.get(ct, [])
    if ct == 'unix':
      opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
      opts.append(cpp_flag(self.compiler))
      opts.append('-fopenmp') # assumes openmp is supported
    elif ct == 'msvc':
      opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())
      opts.append("/openmp")
    for ext in self.extensions:
      ext.extra_compile_args = opts
      ext.extra_link_args = ['-fopenmp'] # assumes openmp is supported
    build_ext.build_extensions(self)

ext_modules = [
  
  Extension(
    'c_clause', # needs to match module name in cpp bindings
    ['bindings.cpp'] + glob("src/cpp/core/*.cpp") + glob("src/cpp/features/*.cpp") + glob("src/cpp/*.cpp") + glob("src/cpp/api/*.cpp"),
    include_dirs=[
      pybind11.get_include(False),
      pybind11.get_include(True ),
    ],
    language='c++',
  ),
]


setup(
  name             = 'PyClause',
  ext_modules      = ext_modules,
  install_requires = ['pybind11>=2.2.0', 'numpy>=1.21.6', 'flatdict', 'pyyaml'],
  cmdclass         = {'build_ext': BuildExt},
  zip_safe         = False,
)
