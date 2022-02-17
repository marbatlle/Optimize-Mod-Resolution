# Optimal Randomizations for Community Definition
Discover the optimal number of randomizations for your networks when using [MolTI-DREAM](https://github.com/gilles-didier/MolTi-DREAM), between 5, 10 or 15 randomizations.
## Input
* [Networks](src/networks/README.md)

## Obtain optimal randomiztions value for MolTI-DREAM
### Run 
* bash obtain_optimal_rands.sh
### Results
* **Optimal randomizations and analysis:** output/optimal_randomizations.txt
* **Randomizations behaviour plot:** output/optimal_rands_figure.png

## References
Didier G, Valdeolivas A, Baudot A. Identifying communities from multiplex biological networks by randomized optimization of modularity. F1000Res. 2018 Jul 10;7:1042. doi: 10.12688/f1000research.15486.2. PMID: 30210790; PMCID: PMC6107982.

## Network

Add one or more networks with the filename format *.tsv* in this folder, as seen in the [sample networks folder](https://github.com/marbatlle/Optimal-Rands-Communities/blob/bbc1168e96cfa3c1a0b740dbf828c6bd5c87d74c/src/sample_networks)
