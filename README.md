PyClause
==============

Installation
------------

git clone https://github.com/Nzteb/PyClause \
cd PyClause \
pip install -e .


Entity and Relation Representation
------------
All definitions hold independently for entities and relations. <br>
<strong>Tokens:</strong> The strings that describe entities in the data files such as train.txt <br>
<strong>Idx's:</strong> Integers internally used. Tokens are mapped to Idx's by e.g. an entity_map <br>
<strong>Names:</strong> Optional additional strings that describe entities and relations in a human understandable way. <br>


Run tests (temporary)
------------

Download [this](https://www.dropbox.com/scl/fi/75py3esgpsn6f370ap7q5/test-data.zip?rlkey=7i136d7d7194x2tvoxqsv0e36&dl=0), unpack, and put the folders into data/ <br>
Then, from the base directory run
```pytest -s```.



