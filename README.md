# Determining an optimal modularity resolution parameter for MolTI-DREAM
Discover the optimal modularity resolution parameter for your multilayer network when using [MolTI-DREAM](https://github.com/gilles-didier/MolTi-DREAM).

## Requirements
This script depends on depends on:
* Python ( >=2.6, tested on 3.9.9)
* Pandas (tested on 1.2.0)
* Numpy (tested on 1.19.5)

Requirements Running Higashi Python ( > =3.5.0, tested on 3.7.9) h5py (tested on 2.10.0) numpy (tested on 1.19.2) pandas (tested on 1.1.3) pytorch (tested on 1.4.0) fbpca (tested on 1.0.0) scikit

## Input
Add one or more networks with a comma separated file, *.csv*, in the [Networks](src/networks/) folder, as seen in the [sample networks folder](https://github.com/marbatlle/Optimal-Rands-Communities/blob/bbc1168e96cfa3c1a0b740dbf828c6bd5c87d74c/src/sample_networks)
## Run 

    - Run script: bash optimal_mod.sh 
    - Optional arguments: -u upper_limit -l lower_limit -s steps

## Output
The result is presented at the terminal and at [Output](output/) directory, determining an optimal modularity resolution parameter as well as the corresponding number of communities and average community size. Additional results can be found at the output resulting folder.

## References
Didier G, Valdeolivas A, Baudot A. Identifying communities from multiplex biological networks by randomized optimization of modularity. F1000Res. 2018 Jul 10;7:1042. doi: 10.12688/f1000research.15486.2. PMID: 30210790; PMCID: PMC6107982.

