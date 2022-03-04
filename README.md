# Determining an optimal modularity resolution parameter for MolTI-DREAM
Discover the optimal modularity resolution parameter for your multilayer network when using [MolTI-DREAM](https://github.com/gilles-didier/MolTi-DREAM).

## Requirements
This script depends on depends on:
* Python (>=2.6, tested on 3.9.9)
* Pandas (tested on 1.2.0)
* Numpy (tested on 1.19.5)

## Input
Add one or more networks with a comma separated file, *.csv*, in the [Networks](input/networks/) folder, as seen in the [Sample Networks Folder](input/sample_networks/)
## Run 

    - Run script: bash optimal_mod.sh 
    - Optional arguments: 
        -u upper_limit    set maximum modularity value tested                 [default is 0]  
        -l lower_limit    set minimum modularity value tested                 [default is 50]
        -s steps          set steps to cover the defined range                [default is 5]
        -m min_nodes      set minimal num. of nodes to define a community     [default is 6]
        -r randomizations set number of Louvain randomizations [default 5]
        
## Output
The result is presented at the terminal and at [Output](output/) directory, determining an optimal modularity resolution parameter as well as the corresponding number of communities and average community size, after filtering for disease modules (communities > 6 nodes). Additional results can be found at the output resulting folder.

## References
Didier G, Valdeolivas A, Baudot A. Identifying communities from multiplex biological networks by randomized optimization of modularity. F1000Res. 2018 Jul 10;7:1042. doi: 10.12688/f1000research.15486.2. PMID: 30210790; PMCID: PMC6107982.
