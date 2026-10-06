How to run experiment

### Run first run
```
ruby run.rb
mv history.js history1.js
```

### Run randomized run
```
ruby run.rb 1
mv history.js history2.js
```

### Merge results
```
ruby merge.rb history1.js history2.js > merged.js
```

### Setup python env

```
python3 -m venv py
source py/bin/activate
python3 -m pip install matplotlib
```

### Plot 

```
python3 plot1.py
python3 plot2.py
python3 plot3.py
python3 plot4.py
python3 plot5.py
```

