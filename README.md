
# <img src="https://raw.githubusercontent.com/symbolic-kg/PyClause/master/docs/logo/PyClause/proc/PyClause.png" width="200" height="65" style="margin-right: 10px;" alt="PyClause">


PyClause is a library for easy and efficient usage and learning of symbolic rules for knowledge graphs.

## Documentation
Documentation of library and features can be found [here](https://pyclause.readthedocs.io/en/latest/index.html). <br>
All usable default options for the configuration files can be found [here](clause/config-default.yaml) 


## Installation
PyClause only uses a few lightweight Python dependencies. We recommend using a fresh conda environment with Python 3.7+.
You also need to have a c++(14) compiler installed. Installation is tested under Ubuntu and Windows.

### 1) Install from codebase
For running our examples and using our data directories or working with the code
```
git clone https://github.com/symbolic-kg/PyClause
cd PyClause
pip install -e .
```

### 2) Install as a package
For using PyClause in your own Python project and as a dependency. When running our examples, you have to set your own correct paths to data and rule files. <br>
install:

```
pip install git+https://github.com/symbolic-kg/PyClause.git
```

as dependency:
```
## in setup.py

setup(
  name              = "MyProject",
  install_requires  = ["PyClause @ git+https://github.com/symbolic-kg/PyClause.git#egg=PyClause"],
  python_requires   = ">=3.7"
)

```

## Quickstart
After installing PyClause with any of the two options run the following code from anywhere.
```python
from c_clause import QAHandler, Loader
from clause.config.options import Options

# ***Example for Query Answering***

# define a knowledge graph
# alternatively, specify file path or use arrays + indices
data = [
    ("anna", "livesIn", "london"),
    ("anna", "learns", "english"),
    ("bernd", "speaks", "french")
]
# define rules, or specify file path
rules = [
     "speaks(X,Y) <= learns(X,Y)",
     "speaks(X,english) <= livesIn(X,london)",
     "speaks(X,english) <= speaks(X,A)"
]
# define rule stats: num_preds, support
stats = [
    [20, 10],
    [40, 35],
    [50, 5],
]
# define options, handlers and load data
opts = Options()
opts.set("qa_handler.aggregation_function", "noisyor")

loader = Loader(options=opts.get("loader"))
loader.load_data(data)
loader.load_rules(rules=rules, stats=stats)

qa = QAHandler(options=opts.get("qa_handler"))
# define query: (anna, speaks, ?); alternatively, use indices
queries = [("anna", "speaks")]
qa.calculate_answers(queries=queries, loader=loader, direction="tail")
# outputs [("english", 0.867 )] 
print(qa.get_answers(as_string=True)[0])

# define query: (?, speaks, english); alternatively, use indices
queries = [("english", "speaks")]
qa.calculate_answers(queries=queries, loader=loader, direction="head")
# outputs [('anna', 0.867), ('bernd', 0.001)] 
print(qa.get_answers(as_string=True)[0])

```


## Entity and Relation Representation

All definitions hold independently for entities and relations. <br>
<strong>Tokens:</strong> The strings that describe entities/relations in the data files such as train.txt. In some repositories these are termed 'Identifier'. <br>
<strong>Idx's:</strong> Integers internally used. Tokens are mapped to Idx's by e.g. an entity_map or relation_map <br>
<strong>Names:</strong> Optional additional strings that describe entities and relations in a human understandable way. <br>

## Supported rule types

Let ```h, b1, b2, b3``` be relation token strings. <br>
Let ```X A B C D E F G H .. Y``` be variables <br>
Let ```c,d``` be entity token strings. <br>

### B Rules
Examples<br>
```
h(X,Y) <= b1(X,A), b2(A,Y)
h(X,Y) <= b1(A,X), b2(A,Y)
h(X,Y) <= b1(X,A), b2(A,B), b3(B,Y)
```

### Uc Rules
Examples<br>
```
h(X,c) <= b1(X,d)
h(c,Y) <= b1(Y,c)
h(c,Y) <= b1(c,Y)

h(X,c) <= b1(A,X), b2(A,d)
h(c,Y) <= b1(Y,A), b2(d,A)
h(c,Y) <= b1(A,Y), b2(A,d)

h(c,Y) <= b1(A,Y), b2(A,B), b3(A,d)
```


### Ud Rules
Examples<br>
```
h(X,c) <= b1(X,A)
h(c,Y) <= b1(Y,A)
h(c,Y) <= b1(A,Y)

h(X,c) <= b1(A,X), b2(A,B)
h(c,Y) <= b1(Y,A), b2(B,A)
h(c,Y) <= b1(A,Y), b2(A,B)

h(c,Y) <= b1(A,Y), b2(A,B), b3(A,C)
```


### Zero (Z) Rules
This rule type is used directed. The first rule only makes prediction in tail direction given a head=X and vice versa for the second rule. <br><br>
Examples<br>
```
h(X,c) <= 
h(c,Y) <= 
```

### Uxxc Rules
This rule type only has one body atom.<br>
Examples<br>
```
h(X,X) <= b1(X,d) 
h(X,X) <= b1(d,X) 
```

### Uxxd Rules
This rule type only has one body atom.<br>
Examples<br>
```
h(X,X) <= b1(X,A) 
h(X,X) <= b1(A,X) 
```




## Run tests (temporary)


Download [this](https://www.dropbox.com/scl/fi/75py3esgpsn6f370ap7q5/test-data.zip?rlkey=7i136d7d7194x2tvoxqsv0e36&dl=0), unpack, and put the folders into data/ <br>
Then, from the base directory run
```pytest -s```.


## Colophon

Have you spotted it? <br>

 <img src="https://raw.githubusercontent.com/symbolic-kg/PyClause/master/docs/logo/PyClause/proc/PyClause-begriffsschrift-transparent.png" width="214" height="75" style="margin-right: 10px;">

The PyClause logo is a reminiscence of Freges Begriffsschrift, which has been published in the year 1879. Even though Frege is not well known in computer science, in his book Begriffsschrift he developed a calculus using an uncommon notation, which is essentially second-order logic with identity (an extension of first-order logic). <br>

The blue lines in the logo are Freges way to express an implication. If we would write A at the end of the first line and B at the end of the second line, this would correspond to a logical rule A <= B, which can again be understood as a clause with a positive and a negative literal. The round dint at the beginning is Freges way to express universal quantification. For more details we refer to the [Wikipedia](https://en.wikipedia.org/wiki/Begriffsschrift) article or the book from 1879 entitled Begriffsschrift.
