# Optimal Modularity Resolution Parameter for Community Definition
Discover the optimal modularity resolution parameter for your multilayer network when using [MolTI-DREAM](https://github.com/gilles-didier/MolTi-DREAM).

## Input
* [Networks](src/networks/)

Add one or more networks with a comma separated file, *.csv*, in the [Networks](https://github.com/marbatlle/Optimal-Rands-Communities/blob/bbc1168e96cfa3c1a0b740dbf828c6bd5c87d74c/src/networks) folder, as seen in the [sample networks folder](https://github.com/marbatlle/Optimal-Rands-Communities/blob/bbc1168e96cfa3c1a0b740dbf828c6bd5c87d74c/src/sample_networks)


## Run 

    - Run script: bash optimal_mod.sh 
    - Optional arguments: -u upper_limit -l lower_limit -s steps

## Output
The result will pop up at the terminal indicating the optimal modularity resolution parameter, as well as the respective average community size and total number of communities. Additional results can be found at the output resulting folder.

## References
Didier G, Valdeolivas A, Baudot A. Identifying communities from multiplex biological networks by randomized optimization of modularity. F1000Res. 2018 Jul 10;7:1042. doi: 10.12688/f1000research.15486.2. PMID: 30210790; PMCID: PMC6107982.

