Installation
============

PyClause only uses a few lightweight Python dependencies and runs under Windows and Linux systems. Linux users need to have a C++ (14) compiler installed. Windows users need to have C++ build tools installed. PyClause requires Microsoft Visual C++ 14.0 or newer.

We recommend using a fresh Conda environment with Python 3.7+.

Install from codebase
~~~~~~~~~~~~~~~~~~~~~

For running our examples and using our data directories or working with the code:

.. code-block:: bash

    git clone https://github.com/symbolic-kg/PyClause
    cd PyClause
    pip install -e .

Install as a package
~~~~~~~~~~~~~~~~~~~~

For using PyClause in your own Python project and as a dependency. When running our examples, you have to set your own correct paths to data and rule files.

install:

.. code-block:: bash

    pip install git+https://github.com/symbolic-kg/PyClause.git

as dependency:

Add the following to your `setup.py` file:

.. code-block:: python

    setup(
      name             = "MyProject",
      install_requires = ["PyClause @ git+https://github.com/symbolic-kg/PyClause.git#egg=PyClause"],
      python_requires  = ">=3.7"
    )
